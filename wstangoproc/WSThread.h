#ifndef WSTHREAD
#define WSTHREAD

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include "common.h"
#include <tango.h>
//#include <omnithread.h>
//#include <unordered_map>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

//#include "ConnectionData.h"
//#include "common.h"
//#include <tango.h>
#include "ParsingInputJson.h"
#include <omnithread.h>
#include <unordered_map>

#define INFO_STREAM_F logger->info_stream()
#define DEBUG_STREAM_F logger->debug_stream()
#define FATAL_STREAM_F logger->fatal_stream()
#define ERROR_STREAM_F logger->error_stream()

typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;

namespace WebSocketDS_ns
{
    class WSTangoConn;
    class ConnectionData;
    class DeviceProxy;
    
    class WSThread : public omni_thread
    {
    public:
        WSThread(WSTangoConn *tc, int portNumber);
        virtual ~WSThread();

        virtual void *run_undetached(void *) = 0;

        virtual void stop() = 0;
        virtual void send_all(std::string msg) = 0;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg) = 0;
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) = 0;
        bool isAliasMode();
        log4tango::Logger *logger;

    protected:
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

        unordered_map<string, string> parseOfGetQuery(string query);

        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) = 0;

        //void removeSymbolsForString(string &str);

        int port;

        const unsigned long maximumBufferSizeMin = 1;
        const unsigned long maximumBufferSizeMax = 10000;
        const unsigned long maximumBufferSizeDef = 1000;

        typedef std::map<websocketpp::connection_hdl, ConnectionData*, std::owner_less<websocketpp::connection_hdl> > con_list;
        con_list m_connections;

        std::mutex m_connection_lock;
        
        WSTangoConn* _tc;

        unsigned long m_next_sessionid;
        ParsingInputJson* parsing = nullptr;

    private:

        vector<string> &split(const string &s, char delim, vector<string> &elems);
        vector<string> split(const string &s, char delim);

        ConnectionData* getConnectionData(websocketpp::connection_hdl hdl);
        bool checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet);
        
        std::mutex m_action_lock;
        websocketpp::lib::condition_variable m_action_cond;
        bool local_th_exit;

    };
}

#endif
