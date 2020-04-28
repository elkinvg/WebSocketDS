#include "WSThread_tls.h"
#ifdef SERVER_MODE
#include "WSTangoConnSer.h"
#endif
#ifdef CLIENT_MODE
#include "WSTangoConnCli.h"
#endif

#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
// TODO: DELETE
//#include <cmath>
//
//#include <locale.h>
//#include <boost/lexical_cast.hpp>
//#include <boost/asio.hpp>

#include "StringProc.h"


namespace WebSocketDS_ns
{
#ifdef SERVER_MODE
    WSThread_tls::WSThread_tls(WSTangoConnSer *tc, int portNumber, string cert, string key)
#endif
#ifdef CLIENT_MODE
    WSThread_tls::WSThread_tls(WSTangoConnCli *tc, int portNumber, string cert, string key)
#endif
        :WSThread(tc, portNumber),
        certificate_(cert),
        key_(key)
    {
        start_undetached();
    }

    bool WSThread_tls::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM << "Check validate WSTHREAD_TLS" << endl;

        auto conSize = m_connections.size();
        DEBUG_STREAM << "Number of connections: " << conSize << endl;

        if (_tc->getMaxNumberOfConnections() == 0)
            return true;
        if (conSize >= _tc->getMaxNumberOfConnections())
            return false;

        return true;
    }

    void *WSThread_tls::run_undetached(void *ptr)
    {
        DEBUG_STREAM << "The upload thread (TLS) starts..." << endl;
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
        DEBUG_STREAM << "WS stopped.." << endl;
        return 0;
    }

    WSThread_tls::~WSThread_tls() {
    }

    std::string WSThread_tls::get_password() {
        return "test";
    }

    void WSThread_tls::send_all(std::string msg) {
        StringProc::removeSymbolsForString(msg);
        int ii = 0;
        size_t total = 0;

        unsigned int maxBuffSize = _tc->getMaxBuffSize();
        if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
            maxBuffSize = maximumBufferSizeDef * 1024;
        else
            maxBuffSize = maxBuffSize * 1024;

        vector<std::pair<websocketpp::connection_hdl, string>> for_close;

        std::unique_lock<std::mutex> con_lock(m_connection_lock);

        for (const auto& conn : m_connections) {
            try {
                websocketpp::lib::error_code erc;
                size_t buffered_amount = get_buffered_amount((conn.first));

                total += buffered_amount;
                DEBUG_STREAM << "con: " << ii << " bufersize: " << buffered_amount << " bytes | " << std::fixed << std::setprecision(3) << (buffered_amount / (1024. * 1024.)) << " Mb" << endl;

                if (buffered_amount<maxBuffSize)
                    m_server.send((conn.first), msg, websocketpp::frame::opcode::text);
                else {
                    ii++;
                    for_close.push_back(
                        std::make_pair(conn.first, "")
                    );
                    continue;
                }
                ii++;
            }
            catch (websocketpp::exception const & e) {
                string exc = "exception from send_all: " + string(e.what());
                DEBUG_STREAM << exc;
                ii++;
                for_close.push_back(
                    std::make_pair(conn.first, exc)
                );
            }
            catch (std::exception& e)
            {
                string exc = "exception from send_all: " + string(e.what());
                ii++;
                for_close.push_back(
                    std::make_pair(conn.first, exc)
                );
            }
            catch (...) {
                string exc = "unknown error from send_all ";
                DEBUG_STREAM << exc;
                ii++;
                for_close.push_back(
                    std::make_pair(conn.first, exc)
                );
            }
        }

        for (auto& cls : for_close) {
            if (!cls.second.size()) {
                close_from_server(cls.first, websocketpp::close::status::going_away, "buffer overloaded", false);
            }
            else {
                close_from_server(cls.first, websocketpp::close::status::internal_endpoint_error, cls.second, false);
            }
        }

        DEBUG_STREAM << std::fixed << "total bufersize: " << total << " | " << std::setprecision(3) << (total / (1024.*1024.)) << "  Mb: " << endl;
    }

    void WSThread_tls::send(websocketpp::connection_hdl hdl, std::string msg, bool isLocked) {
        if (hdl.expired()) {
            return;
        }
        StringProc::removeSymbolsForString(msg);
        try {
            size_t buffered_amount = get_buffered_amount(hdl);

            unsigned int maxBuffSize = _tc->getMaxBuffSize();

            if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
                maxBuffSize = maximumBufferSizeDef * 1024;
            else
                maxBuffSize = maxBuffSize * 1024;

            if (buffered_amount < maxBuffSize)
                m_server.send(hdl, msg, websocketpp::frame::opcode::text);
            else
                close_from_server(hdl, websocketpp::close::status::going_away, "buffer overloaded", isLocked);

        }
        catch (websocketpp::exception const & e) {
            // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
            string exc = "exception from send: " + string(e.what());
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, isLocked);
        }
        catch (std::exception& e) {
            // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
            string exc = "exception from send: " + string(e.what());
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, isLocked);
        }
        catch (...) {
            // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
            string exc = "unknown error from send";
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, isLocked);
        }
    }

    void WSThread_tls::send(websocketpp::connection_hdl hdl, const void *data, size_t len)
    {
        m_server.send(hdl, data, len, websocketpp::frame::opcode::binary);
    }

    void WSThread_tls::stop()
    {
        local_th_exit = true;
        m_action_cond.notify_one();

        actTh->join();
        delete actTh;

        DEBUG_STREAM << "The ws thread stops..." << endl;
        m_server.stop();
        DEBUG_STREAM << "WS stops..." << endl;
    }

    std::unordered_map<string, string> WSThread_tls::getRemoteConf(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        websocketpp::uri_ptr uri = con->get_uri();

        string remoteEndpoint = con->get_remote_endpoint();
        std::unordered_map<string, string> parsedGet;
        remoteEndpoint = StringProc::parseOfAddress(remoteEndpoint);

        // Получение X-Forwarded-For проксированных запросов
        string xforwarded = con->get_request_header("X-Forwarded-For");
        xforwarded.erase(remove(xforwarded.begin(), xforwarded.end(), ' '), xforwarded.end());
        vector<string> proxyes = StringProc::parseInputString(xforwarded
            , ",", true);

        string query = uri->get_query(); // returns empty string if no query string set.

        if (!query.empty()) {
            parsedGet = StringProc::parseOfGetQuery(query);
        }

        if (proxyes.size() && proxyes[0].size() && proxyes[0] != "::1")
            parsedGet["ip"] = proxyes[0];
        else
            parsedGet["ip"] = remoteEndpoint;

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
            ERROR_STREAM << e.what() << std::endl;
        }
        return ctx;
    }

    void WSThread_tls::close_from_server(websocketpp::connection_hdl hdl, websocketpp::close::status::value const code, std::string const & reason, bool isLocked) {
        try {
            websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
            con->close(code, reason);
        }
        catch (...) {}

        if (isLocked) {
            std::unique_lock<std::mutex> con_lock(m_connection_lock);
            _close_from_server(hdl);
        }
        else {
            _close_from_server(hdl);
        }
    }

    size_t WSThread_tls::get_buffered_amount(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        return con->get_buffered_amount();
    }
}

