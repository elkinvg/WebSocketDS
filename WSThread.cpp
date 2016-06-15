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

void *WSThread::run_undetached(void *ptr)
{
    DEBUG_STREAM << "The upload thread starts..." << endl;
    cache = "";
    print_server.set_open_handler(websocketpp::lib::bind(&WSThread::on_open,this,websocketpp::lib::placeholders::_1));
    print_server.set_close_handler(websocketpp::lib::bind(&WSThread::on_close,this,websocketpp::lib::placeholders::_1));
    print_server.set_message_handler(websocketpp::lib::bind(&WSThread::on_message,this,websocketpp::lib::placeholders::_1,websocketpp::lib::placeholders::_2));
    
    // this will turn off console output for frame header and payload
    print_server.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

    // this will turn off everything in console output
    //print_server.clear_access_channels(websocketpp::log::alevel::all);

    print_server.init_asio();
    /*boost::asio::ip::tcp::resolver resolver(print_server.get_io_service());
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);*/

    //print_server.listen(endpoint);
    print_server.listen(port);
    print_server.start_accept();
    print_server.run();
    DEBUG_STREAM << "WS stopped.." << endl;
    return 0;      
}

void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    DEBUG_STREAM << msg->get_payload() << endl;
    string data_from_client = msg->get_payload();
    send(hdl, "from on_message");
}

void WSThread::on_open(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "New user has been connected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        m_connections.insert(hdl);
        send(hdl, cache);
}

void WSThread::on_close(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "User has been disconnected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        m_connections.erase(hdl);
  
}

void WSThread::send_all(std::string msg) {
    cache.clear();
    cache = msg;
    //msg.clear();
    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(*it,msg ,websocketpp::frame::opcode::text);
    }
}

void WSThread::send(websocketpp::connection_hdl hdl, std::string msg) {
    m_server.send(hdl, msg ,websocketpp::frame::opcode::text);
}


WSThread::~WSThread() {
}

void WSThread::stop()
{
    DEBUG_STREAM << "The ws thread stops..." << endl;
    print_server.stop();
    DEBUG_STREAM << "WS stops..." << endl;
}

}

