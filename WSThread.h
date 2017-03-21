#ifndef WSTHREAD
#define WSTHREAD

//#ifdef _WIN32
//#define WINVER 0x0A00
//#define WINVER 0x0601
//#define _MSC_VER 1600
//#endif
#ifdef _WIN32
#define WINVER 0x0A00
#endif

//#include <websocketpp/config/asio.hpp>
//#include <websocketpp/server.hpp>
//#include <websocketpp/common/thread.hpp>
#include <tango.h>
//#include "UserControl.h"

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class UserControl;
    class WSThread : public omni_thread, public Tango::LogAdapter
    {
    public:
        WSThread(WebSocketDS *dev/*, std::string hostName*/, int portNumber);
        virtual ~WSThread();

        virtual void *run_undetached(void *) = 0;

        virtual  void stop() = 0;
        virtual void send_all(std::string msg) = 0;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg) = 0;
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) = 0;
    protected:
        WebSocketDS *ds;
        std::string cache;
        virtual bool on_validate(websocketpp::connection_hdl hdl) = 0;
        void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
        void on_open(websocketpp::connection_hdl hdl);
        void on_close(websocketpp::connection_hdl hdl);
        //test
        void  on_fail(websocketpp::connection_hdl);
        virtual void close_from_server(websocketpp::connection_hdl hdl) = 0;
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) = 0;

        string parseOfAddress(string addrFromConn); // parsing of get_remote_endpoint-return
        // remoteEndpoint in websocket output formate[::ffff:127.0.0.1 : 11111]

        map<string, string> parseOfGetQuery(string query);

        virtual map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) = 0;

        bool forValidate(map<string, string> remoteConf);
        int port;

        const Tango::DevULong maximumBufferSizeMin = 1;
        const Tango::DevULong maximumBufferSizeMax = 10000;
        const Tango::DevULong maximumBufferSizeDef = 1000;

        typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl> > con_list;
        con_list m_connections;
        unique_ptr<UserControl> uc;

        websocketpp::lib::mutex m_connection_lock;

    private:
        vector<string> &split(const string &s, char delim, vector<string> &elems);
        vector<string> split(const string &s, char delim);

        websocketpp::lib::mutex m_action_lock;
        websocketpp::lib::condition_variable m_action_cond;
        bool local_th_exit;

        //    WebSocketDS *ds;

        //    std::string host;

    };
}


#endif