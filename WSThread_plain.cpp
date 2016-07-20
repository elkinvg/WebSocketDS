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
void *WSThread_plain::run_undetached(void *ptr)
{
    DEBUG_STREAM << "The upload thread (PLAIN) starts..." << endl;
    cache = "";
    m_server.set_open_handler(websocketpp::lib::bind(&WSThread_plain::on_open,this,websocketpp::lib::placeholders::_1));
    m_server.set_close_handler(websocketpp::lib::bind(&WSThread_plain::on_close,this,websocketpp::lib::placeholders::_1));
    m_server.set_message_handler(websocketpp::lib::bind(&WSThread_plain::on_message,this,websocketpp::lib::placeholders::_1,websocketpp::lib::placeholders::_2));

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
        m_server.send(*it,msg ,websocketpp::frame::opcode::text);
    }
}

void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
    m_server.send(hdl, msg ,websocketpp::frame::opcode::text);
}

void WSThread_plain::stop()
{
    DEBUG_STREAM << "The ws thread stops..." << endl;
    m_server.stop();
    DEBUG_STREAM << "WS stops..." << endl;
}

WSThread_plain::~WSThread_plain() {}

}
