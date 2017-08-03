#include "WSThread_tls.h"
#include "WSTangoConn.h"

#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
#include <cmath>

#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "StringProc.h"



namespace WebSocketDS_ns
{
    WSThread_tls::WSThread_tls(WSTangoConn *tc, int portNumber, string cert, string key) :
        WSThread(tc, portNumber)
    {
        certificate_ = cert;
        key_ = key;
        start_undetached();
    }

    bool WSThread_tls::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM_F << "Check validate WSTHREAD_TLS" << endl;

        auto conSize = m_connections.size();
        DEBUG_STREAM_F << "Number of connections: " << conSize << endl;

        if (_tc->getMaxNumberOfConnections() == 0)
            return true;
        if (conSize >= _tc->getMaxNumberOfConnections())
            return false;

        return true;
        //return  forValidate(conf);;
    }

    void *WSThread_tls::run_undetached(void *ptr)
    {
        DEBUG_STREAM_F << "The upload thread (TLS) starts..." << endl;
        cache = "";
        m_server.set_open_handler(websocketpp::lib::bind(&WSThread_tls::on_open,this,websocketpp::lib::placeholders::_1));
        m_server.set_close_handler(websocketpp::lib::bind(&WSThread_tls::on_close,this,websocketpp::lib::placeholders::_1));
        m_server.set_message_handler(websocketpp::lib::bind(&WSThread_tls::on_message,this,websocketpp::lib::placeholders::_1,websocketpp::lib::placeholders::_2));
        m_server.set_validate_handler(bind(&WSThread_tls::on_validate, this, websocketpp::lib::placeholders::_1));
        m_server.set_fail_handler(bind(&WSThread_tls::on_fail, this, websocketpp::lib::placeholders::_1));

        // this will turn off console output for frame header and payload
        m_server.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);
        m_server.set_tls_init_handler(websocketpp::lib::bind(&WSThread_tls::on_tls_init,this,websocketpp::lib::placeholders::_1));

        // this will turn off everything in console output
        //m_server.clear_access_channels(websocketpp::log::alevel::all);

        m_server.init_asio();

        m_server.set_reuse_addr(true); // for LINUX
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
        DEBUG_STREAM_F << "WS stopped.." << endl;
        return 0;
    }

    WSThread_tls::~WSThread_tls() {
        for (auto &cn : m_connections) {
            // Выскакивал access violation reading location, если таймер запущен
            if (cn.second.timing != nullptr) {
                cn.second.timing.reset(nullptr);
            }
        }
    }

    std::string WSThread_tls::get_password() {
        return "test";
    }

    void WSThread_tls::send_all(std::string msg) {
        cache.clear();
        cache = msg;
        StringProc::removeSymbolsForString(msg);
        //msg.clear();
        con_list::iterator it;

        unsigned int maxBuffSize = _tc->getMaxBuffSize();
        if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
            maxBuffSize = maximumBufferSizeDef * 1024;
        else
            maxBuffSize = maxBuffSize * 1024;

        for (it = m_connections.begin(); it != m_connections.end();) {
            try {
                size_t buffered_amount = get_buffered_amount((it->first));

                if (buffered_amount<maxBuffSize)
                    m_server.send((it->first), msg, websocketpp::frame::opcode::text);
                else {
                    close_from_server((it++->first));
                    continue;
                }

                ++it;
            }
            catch (websocketpp::exception const & e) {
                ERROR_STREAM_F << "exception from send_all: " << e.what() << endl;
                close_from_server((it++->first));
            }
            catch (std::exception& e) {
                ERROR_STREAM_F << "exception from send_all: " << e.what() << endl;
                close_from_server((it++->first));
            }
            catch (...) {
                ERROR_STREAM_F << "unknown error from send_all " << endl;
                close_from_server((it++->first));
            }
        }
    }

    void WSThread_tls::send(websocketpp::connection_hdl hdl, std::string msg) {
        StringProc::removeSymbolsForString(msg);
        try {
            size_t buffered_amount = get_buffered_amount(hdl);

            unsigned int maxBuffSize = _tc->getMaxBuffSize();

            if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
                maxBuffSize = maximumBufferSizeDef * 1024;
            else
                maxBuffSize = maxBuffSize * 1024;

            if (buffered_amount<maxBuffSize)
                m_server.send(hdl, msg, websocketpp::frame::opcode::text);
            else
                close_from_server(hdl);

        }
        catch (websocketpp::exception const & e) {
            ERROR_STREAM_F << "exception from send: " << e.what() << endl;
            close_from_server(hdl);
        }
        catch (std::exception& e) {
            ERROR_STREAM_F << "exception from send: " << e.what() << endl;
            close_from_server(hdl);
        }
        catch (...) {
            ERROR_STREAM_F << "unknown error from send " << endl;
            close_from_server(hdl);
        }
    }

    void WSThread_tls::send(websocketpp::connection_hdl hdl, const void *data, size_t len)
    {
        m_server.send(hdl, data, len, websocketpp::frame::opcode::binary);
    }

    void WSThread_tls::stop()
    {
        DEBUG_STREAM_F << "The ws thread stops..." << endl;
        m_server.stop();
        DEBUG_STREAM_F << "WS stops..." << endl;
    }

    std::unordered_map<string, string> WSThread_tls::getRemoteConf(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        websocketpp::uri_ptr uri = con->get_uri();

        string remoteEndpoint = con->get_remote_endpoint();
        std::unordered_map<string, string> parsedGet;
        remoteEndpoint = parseOfAddress(remoteEndpoint);

        string query = uri->get_query(); // returns empty string if no query string set.

        if (!query.empty()) {
            parsedGet = parseOfGetQuery(query);
            parsedGet["ip"] = remoteEndpoint;
        }
        return parsedGet;
    }

    context_ptr WSThread_tls::on_tls_init(websocketpp::connection_hdl hdl) {
        //std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
        //context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));
        context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::sslv23));

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds
                        | boost::asio::ssl::context::no_sslv2
                        | boost::asio::ssl::context::single_dh_use);
    //        ctx->set_options(boost::asio::ssl::context::default_workarounds |
    //                         boost::asio::ssl::context::no_sslv2 |
    //                         boost::asio::ssl::context::no_sslv3 |
    //                         boost::asio::ssl::context::single_dh_use);
            ctx->set_password_callback(websocketpp::lib::bind(&WSThread_tls::get_password, this));
            ctx->use_certificate_chain_file(certificate_);
            ctx->use_private_key_file(key_, boost::asio::ssl::context::pem);
        } catch (std::exception& e) {
            ERROR_STREAM_F << e.what() << std::endl;
        }
        return ctx;
    }

    void WSThread_tls::close_from_server(websocketpp::connection_hdl hdl) {
        if (_tc->isServerMode())
            websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        con->close(websocketpp::close::status::normal, "");
        m_connections.erase(hdl);
    }

    size_t WSThread_tls::get_buffered_amount(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        return con->get_buffered_amount();
    }

    void WSThread_tls::startTimer(websocketpp::connection_hdl hdl)
    {
        if (hdl.expired())
            return;
        
        try {
            bool hasDevice;
            string resp = m_connections[hdl].tangoConnForClient->getJsonForAttribute(hasDevice);
            
            if (hdl.expired())
                return;
            
            send(hdl, resp);

            if (!hasDevice) {
                m_connections[hdl].timing.reset(nullptr);
                return;
            }
            m_connections[hdl].timerInd++;
            m_connections[hdl].timing->m_timer = m_server.set_timer(m_connections[hdl].timing->msec, bind(&WSThread_tls::runTimer
                                                                                                          , this, placeholders::_1, hdl, m_connections[hdl].timerInd));
        }
        catch (...) {
            // This exception is not thrown out in normal operation
            ERROR_STREAM_F << "START EXCEPTION!!!!!!!";
        }
    }
    
    void WSThread_tls::runTimer(const error_code & ec, websocketpp::connection_hdl hdl, int timerInd)
    {
        // Все данные в m_connections[hdl]
        if (hdl.expired())
            return;

        if (m_connections[hdl].timing == nullptr)
            return;
        if (!m_connections[hdl].timing->isTimerOn)
            return;

        if (ec.value() !=0 ) {
            ERROR_STREAM_F << " Error code: " << ec.value() << " Mess: " << ec.message();
        }

        try {
            if (forRunTimer(hdl, timerInd))
                return;
            m_connections[hdl].timing->m_timer = m_server.set_timer(m_connections[hdl].timing->msec, bind(&WSThread_tls::runTimer
                , this, placeholders::_1, hdl, timerInd));
        }
        catch (...) {
            // This exception is not thrown out in normal operation
            ERROR_STREAM_F << "RUN EXCEPTION!!!!!!!";
        }
    }

}

