#include "WSThread_plain.h"

#include <tango.h>
#include <omnithread.h>
//#include <log4tango.h>
#include <cmath>

#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "WSTangoConn.h"


namespace WebSocketDS_ns
{
    WSThread_plain::WSThread_plain(WSTangoConn *tc, int portNumber) :
        WSThread(tc, portNumber)
    {
        start_undetached();
    }

    bool WSThread_plain::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM_F << "Check validate WSTHREAD_PLAIN" << endl;
        
        auto conSize = m_connections.size();
        DEBUG_STREAM_F << "Number of connections: " << conSize << endl;

        if (_tc->getMaxNumberOfConnections() == 0)
            return true;
        if (conSize >= _tc->getMaxNumberOfConnections())
            return false;
        
        return true;
        //return  forValidate(conf);
    }

    void *WSThread_plain::run_undetached(void *ptr)
    {
        DEBUG_STREAM_F << "The upload thread (PLAIN) starts..." << endl;
        cache = "";
        m_server.set_open_handler(websocketpp::lib::bind(&WSThread_plain::on_open, this, websocketpp::lib::placeholders::_1));
        m_server.set_close_handler(websocketpp::lib::bind(&WSThread_plain::on_close, this, websocketpp::lib::placeholders::_1));
        m_server.set_message_handler(websocketpp::lib::bind(&WSThread_plain::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));//m_server.set_user_agent();
        m_server.set_validate_handler(bind(&WSThread_plain::on_validate, this, websocketpp::lib::placeholders::_1));
        
         m_server.set_fail_handler(bind(&WSThread_plain::on_fail,this,websocketpp::lib::placeholders::_1));

        // this will turn off console output for frame header and payload
        m_server.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

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

    void WSThread_plain::send_all(std::string msg) {
        cache.clear();
        cache = msg;
        removeSymbolsForString(msg);
        con_list::iterator it;
        int ii=0;
        size_t total = 0;

        unsigned int maxBuffSize = _tc->getMaxBuffSize();
        if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
            maxBuffSize = maximumBufferSizeDef * 1024;
        else
            maxBuffSize = maxBuffSize * 1024;

        for (it = m_connections.begin(); it != m_connections.end();) {
            try {
                websocketpp::lib::error_code erc;
                size_t buffered_amount = get_buffered_amount((it->first));

                total += buffered_amount;
                DEBUG_STREAM_F << "con: " << ii << " bufersize: " << buffered_amount << " bytes | " << std::fixed << std::setprecision(3) << (buffered_amount / (1024. * 1024.)) << " Mb" << endl;

                if (buffered_amount<maxBuffSize)
                    m_server.send((it->first), msg, websocketpp::frame::opcode::text);
                else {
                    ii++;
                    close_from_server((it++->first));
                    continue;
                }

                ++it;
                ii++;
            }
            catch (websocketpp::exception const & e) {
                ERROR_STREAM_F << "exception from send_all: " << e.what() << endl;
                ii++;
                close_from_server((it++->first));
            }
            catch (std::exception& e)
            {
                ERROR_STREAM_F << "exception from send_all: " << e.what() << endl;
                ii++;
                close_from_server((it++->first));
            }
            catch (...) {
                ERROR_STREAM_F << "unknown error from send_all " << endl;
                ii++;
                close_from_server((it++->first));
            }
        }
        DEBUG_STREAM_F << std::fixed << "total bufersize: " << total << " | " << std::setprecision(3) << (total / (1024.*1024.)) << "  Mb: " << endl;
    }



    void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
        removeSymbolsForString(msg);
        try {
            unsigned int maxBuffSize = _tc->getMaxBuffSize();

            if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
                maxBuffSize = maximumBufferSizeDef * 1024;
            else
                maxBuffSize = maxBuffSize * 1024;

            size_t buffered_amount = get_buffered_amount(hdl);

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

    void WSThread_plain::send(websocketpp::connection_hdl hdl, const void *data, size_t len)
    {
        m_server.send(hdl,data,len,websocketpp::frame::opcode::binary);
    }

    void WSThread_plain::stop()
    {
        DEBUG_STREAM_F << "The ws thread stops..." << endl;
        m_server.stop();
        DEBUG_STREAM_F << "WS stops..." << endl;
    }

    unordered_map<string, string> WSThread_plain::getRemoteConf(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        websocketpp::uri_ptr uri = con->get_uri();

        string remoteEndpoint = con->get_remote_endpoint();
        remoteEndpoint = parseOfAddress(remoteEndpoint);

        unordered_map<string, string> parsedGet;

        string query = uri->get_query(); // returns empty string if no query string set.

        if (!query.empty()) {
            parsedGet = parseOfGetQuery(query);
            parsedGet["ip"] = remoteEndpoint;
        }
        return parsedGet;
    }

    void WSThread_plain::startTimer(websocketpp::connection_hdl hdl)
    {
        if (hdl.expired())
            return;
        
        try {
            
            string resp = m_connections[hdl].tangoConnForClient->getJsonForAttribute();
            send(hdl, resp);
            if (hdl.expired())
                return;
            m_connections[hdl].timerInd++;
            m_connections[hdl].timing->m_timer = m_server.set_timer(m_connections[hdl].timing->msec, bind(&WSThread_plain::runTimer
                                                                                                          , this, placeholders::_1, hdl, m_connections[hdl].timerInd));
        }
        catch (...) {
            // This exception is not thrown out in normal operation
            ERROR_STREAM_F << "START EXCEPTION!!!!!!!";
        }
    }
    
    void WSThread_plain::runTimer(const error_code & ec, websocketpp::connection_hdl hdl, int timerInd)
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

        // if ec.value() !=0 and if ec.value()!= SUCCESS
//        DEBUG_STREAM_F << ec.message();
//        DEBUG_STREAM_F << ec.value();
//        DEBUG_STREAM_F << ec.default_error_condition().message();

        try {
            if (forRunTimer(hdl, timerInd))
                return;
            
            m_connections[hdl].timing->m_timer = m_server.set_timer(m_connections[hdl].timing->msec, bind(&WSThread_plain::runTimer
                , this, placeholders::_1, hdl, timerInd));
        }
        catch (...) {
            // This exception is not thrown out in normal operation
            ERROR_STREAM_F << "RUN EXCEPTION!!!!!!!";
        }
    }

    void WSThread_plain::close_from_server(websocketpp::connection_hdl hdl) {
        if (_tc->isServerMode())
            websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        con->close(websocketpp::close::status::going_away, "");
        m_connections.erase(hdl);
    }

    size_t WSThread_plain::get_buffered_amount(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        return con->get_buffered_amount();
    }

    WSThread_plain::~WSThread_plain() {}

}
