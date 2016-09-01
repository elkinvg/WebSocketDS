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
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            m_server.send(*it, msg, websocketpp::frame::opcode::text);
        }
    }

    void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
        m_server.send(hdl, msg, websocketpp::frame::opcode::text);
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
