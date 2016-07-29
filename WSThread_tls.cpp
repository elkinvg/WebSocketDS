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

bool WSThread_tls::on_validate(websocketpp::connection_hdl hdl) {

    map<string, string> conf = getRemoteConf(hdl);

    return forValidate(conf);
}

void *WSThread_tls::run_undetached(void *ptr)
{
    DEBUG_STREAM << "The upload thread (TLS) starts..." << endl;
    cache = ""; 
    m_server.set_open_handler(websocketpp::lib::bind(&WSThread_tls::on_open,this,websocketpp::lib::placeholders::_1));
    m_server.set_close_handler(websocketpp::lib::bind(&WSThread_tls::on_close,this,websocketpp::lib::placeholders::_1));
    m_server.set_message_handler(websocketpp::lib::bind(&WSThread_tls::on_message,this,websocketpp::lib::placeholders::_1,websocketpp::lib::placeholders::_2));
    m_server.set_validate_handler(bind(&WSThread_tls::on_validate, this, websocketpp::lib::placeholders::_1));
    
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

WSThread_tls::~WSThread_tls() {}

std::string WSThread_tls::get_password() {
    return "test";
}

void WSThread_tls::send_all(std::string msg) {
    cache.clear();
    cache = msg;
    //msg.clear();
    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(*it,msg ,websocketpp::frame::opcode::text);
    }
}

void WSThread_tls::send(websocketpp::connection_hdl hdl, std::string msg) {
    m_server.send(hdl, msg ,websocketpp::frame::opcode::text);
}

void WSThread_tls::stop()
{
    DEBUG_STREAM << "The ws thread stops..." << endl;
    m_server.stop();
    DEBUG_STREAM << "WS stops..." << endl;
}

map<string, string> WSThread_tls::getRemoteConf(websocketpp::connection_hdl hdl) {
    websocketpp::server<websocketpp::config::asio_tls>::connection_ptr con = m_server.get_con_from_hdl(hdl);
    websocketpp::uri_ptr uri = con->get_uri();

    string remoteEndpoint = con->get_remote_endpoint();
    map<string, string> parsedGet;
    remoteEndpoint = parseOfAddress(remoteEndpoint);

    string query = uri->get_query(); // returns empty string if no query string set.

    if (!query.empty()) {
        parsedGet = parseOfGetQuery(query);
        parsedGet["ip"] = remoteEndpoint;
    }
    return parsedGet;
}

context_ptr WSThread_tls::on_tls_init(websocketpp::connection_hdl hdl) {
    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
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
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

}

