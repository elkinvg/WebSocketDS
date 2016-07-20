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

void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    DEBUG_STREAM << " Input message: " << msg->get_payload() << endl;
    string data_from_client = msg->get_payload();
    Tango::DevString input = const_cast<Tango::DevString>(data_from_client.c_str());
    Tango::DevString output = ds->send_command_to_device(input);
    send(hdl, output);

    //send(hdl, "from on_message");
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

WSThread::~WSThread() {}

}

