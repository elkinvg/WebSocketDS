#include "WSThread_plain.h"
#include "WebSocketDS.h"

#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
#include <cmath>

#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

//#include "UserControl.h"


namespace WebSocketDS_ns
{
    WSThread_plain::WSThread_plain(WebSocketDS *dev/*, std::string hostName*/, int portNumber) :
        WSThread(dev/*,hostName*/, portNumber)
    {
        start_undetached();
    }

    bool WSThread_plain::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM << "Check validate WSTHREAD_PLAIN" << endl;
        
        auto conSize = m_connections.size();
        DEBUG_STREAM << "Number of connections: " << conSize << endl;
        if (ds->maxNumberOfConnections == 0)
            return true;
        
        if (conSize >= ds->maxNumberOfConnections)
            return false;
        
        return true;
        //return  forValidate(conf);
    }

    void *WSThread_plain::run_undetached(void *ptr)
    {
        DEBUG_STREAM << "The upload thread (PLAIN) starts..." << endl;
        cache = "";
        m_server.set_open_handler(websocketpp::lib::bind(&WSThread_plain::on_open, this, websocketpp::lib::placeholders::_1));
        m_server.set_close_handler(websocketpp::lib::bind(&WSThread_plain::on_close, this, websocketpp::lib::placeholders::_1));
        m_server.set_message_handler(websocketpp::lib::bind(&WSThread_plain::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));//m_server.set_user_agent();
        m_server.set_validate_handler(bind(&WSThread_plain::on_validate, this, websocketpp::lib::placeholders::_1));
        //test
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
        DEBUG_STREAM << "WS stopped.." << endl;
        return 0;
    }

    void WSThread_plain::send_all(std::string msg) {
        cache.clear();
        cache = msg;
        removeSymbolsForString(msg);
        con_list::iterator it;
        int ii=0;
        size_t total = 0;

        unsigned int maxBuffSize = ds->maximumBufferSize;
        if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
            maxBuffSize = maximumBufferSizeDef * 1024;
        else
            maxBuffSize = maxBuffSize * 1024;

        for (it = m_connections.begin(); it != m_connections.end();) {
            try {
                websocketpp::lib::error_code erc;

                size_t buffered_amount = get_buffered_amount(*(it));

                total += buffered_amount;
                DEBUG_STREAM << "con: " << ii << " bufersize: " << buffered_amount << " bytes | ";
                DEBUG_STREAM << std::fixed << std::setprecision(3) << (buffered_amount / (1024. * 1024.)) << " Mb" << endl;

                if (buffered_amount<maxBuffSize)
                    m_server.send(*it, msg, websocketpp::frame::opcode::text);
                else {
                    ii++;
                    close_from_server(*(it++));
                    continue;
                }
                //else

                ++it;
                ii++;
            }
            catch (websocketpp::exception const & e) {
                ERROR_STREAM << "exception from send_all: " << e.what() << endl;
#ifdef TESTFAIL
                std::fstream fs;
                fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
                Tango::DevULong cTime;
                std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
                std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                cTime = timeFromUpdateData.count();
                //fs << cTime << " FROM it: " << ii <<  " websocketpp::exception: " << e.what() << '\n';ii++;
                fs << std::ctime(&end_time) << " FROM it: " << ii <<  " websocketpp::exception: " << e.what() << " | wasNconn = " << m_connections.size() << endl;
                fs << " m_code: " << e.m_code << " | m_msg" << e.m_msg << endl;
                fs << endl;
#endif
                ii++;
                close_from_server(*(it++));
                //on_close(*(it++));
#ifdef TESTFAIL
                fs << " now: " << m_connections.size() << "\n";

                fs.close();
#endif
                //on_close(*(it++));
            }
            catch (std::exception& e)
            {
                ERROR_STREAM << "exception from send_all: " << e.what() << endl;
#ifdef TESTFAIL
                std::fstream fs;
                fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
                Tango::DevULong cTime;
                std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
                std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);
                cTime = timeFromUpdateData.count();
                //fs << cTime << " FROM it: " << ii <<  " std exception caught: " << e.what() << '\n';ii++;
                fs << std::ctime(&end_time) << " FROM it: " << ii <<  " std exception caught: " << e.what() << " wasNconn =" << m_connections.size();
#endif
                ii++;
                close_from_server(*(it++));
                //on_close(*(it++));
#ifdef TESTFAIL
                fs << " now: " << m_connections.size() << "\n";

                fs.close();
#endif
                //on_close(*(it++));

            }
            catch (...) {
                ERROR_STREAM << "unknown error from send_all " << endl;
#ifdef TESTFAIL
                std::fstream fs;
                fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
                Tango::DevULong cTime;
                std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
                cTime = timeFromUpdateData.count();
                fs << cTime << " : Exception from send_all " << endl;
                fs.close();
#endif
                ii++;
                close_from_server(*(it++));
                //on_close(*(it++));
            }
        }
        DEBUG_STREAM << std::fixed << "total bufersize: " << total << " | " << std::setprecision(3) << (total / (1024.*1024.)) << "  Mb: " << endl;
    }



    void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
        removeSymbolsForString(msg);
        try {
            unsigned int maxBuffSize = ds->maximumBufferSize;
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
            ERROR_STREAM << "exception from send: " << e.what() << endl;
            close_from_server(hdl);
            //on_close(hdl);
        }
        catch (std::exception& e) {
            ERROR_STREAM << "exception from send: " << e.what() << endl;
            close_from_server(hdl);
            //on_close(hdl);
        }
        catch (...) {
            ERROR_STREAM << "unknown error from send " << endl;
            close_from_server(hdl);
            //on_close(hdl);
#ifdef TESTFAIL
            std::fstream fs;
            fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
            Tango::DevULong cTime;
            std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
            cTime = timeFromUpdateData.count();
            fs << cTime << " :Unknown Exception from send " << endl;
            fs.close();
#endif
        }
    }

    void WSThread_plain::send(websocketpp::connection_hdl hdl, const void *data, size_t len)
    {
        m_server.send(hdl,data,len,websocketpp::frame::opcode::binary);
    }

    void WSThread_plain::stop()
    {
        DEBUG_STREAM << "The ws thread stops..." << endl;
        m_server.stop();
        DEBUG_STREAM << "WS stops..." << endl;
    }

    map<string, string> WSThread_plain::getRemoteConf(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        websocketpp::uri_ptr uri = con->get_uri();

        string remoteEndpoint = con->get_remote_endpoint();
        remoteEndpoint = parseOfAddress(remoteEndpoint);

        map<string, string> parsedGet;

        string query = uri->get_query(); // returns empty string if no query string set.

        if (!query.empty()) {
            parsedGet = parseOfGetQuery(query);
            parsedGet["ip"] = remoteEndpoint;
        }
        return parsedGet;
    }

    void WSThread_plain::close_from_server(websocketpp::connection_hdl hdl) {
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
