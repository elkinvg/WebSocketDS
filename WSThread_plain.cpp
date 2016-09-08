#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
#include <cmath>
#include "WebSocketDS.h"
#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
namespace WebSocketDS_ns
{
    bool WSThread_plain::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM << "Check validate WSTHREAD_PLAIN!!!" << endl;
        map<string, string> conf = getRemoteConf(hdl);
        
        return  forValidate(conf);
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
        //msg.clear();
        con_list::iterator it;
        int ii=0;
        //for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        for (it = m_connections.begin(); it != m_connections.end();) {
            try {
                m_server.send(*it, msg, websocketpp::frame::opcode::text);
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
                on_close(*(it++));
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
                on_close(*(it++));
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
                on_close(*(it++));
            }
        }
    }



    void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
        try {
            m_server.send(hdl, msg, websocketpp::frame::opcode::text);
        }
        catch (websocketpp::exception const & e) {
            ERROR_STREAM << "exception from send: " << e.what() << endl;
            on_close(hdl);
        }
        catch (std::exception& e) {
            ERROR_STREAM << "exception from send: " << e.what() << endl;
            on_close(hdl);
        }
        catch (...) {
            ERROR_STREAM << "unknown error from send " << endl;
            on_close(hdl);
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

    WSThread_plain::~WSThread_plain() {}

}
