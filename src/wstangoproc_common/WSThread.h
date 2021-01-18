#ifndef WSTHREAD
#define WSTHREAD

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include "common.h"
#include <tango.h>

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

#include "ParsingInputJson.h"
#include <omnithread.h>
#include <unordered_map>
#include "TaskInfo.h"

#include "CurrentMode.h"
#include <stdexcept>

typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;

namespace WebSocketDS_ns
{
    class ConnectionClosedException : public std::runtime_error {
    public:
        ConnectionClosedException() : std::runtime_error("Connection was closed") { }
    };

#ifdef SERVER_MODE
    class WSTangoConnSer;
#endif
#ifdef CLIENT_MODE
    class WSTangoConnCli;
#endif

    class ConnectionData;
    
    class WSThread : public omni_thread
    {
    public:
#ifdef SERVER_MODE
        WSThread(WSTangoConnSer *tc, int portNumber);
#endif
#ifdef CLIENT_MODE
        WSThread(WSTangoConnCli *tc, int portNumber);
#endif
        virtual ~WSThread();

        virtual void *run_undetached(void *) = 0;
        virtual log4tango::Logger *get_logger(void);
        virtual void stop() = 0;
        virtual void send_all(std::string msg) = 0;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg) = 0;
        // Для бинарных данных. Пока не используется.
        // TODO: NOT USED virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) = 0;
        void checkActions();
        void closeConnections(vector<websocketpp::connection_hdl>& _del_conn);

    protected:
        // TODO: MB WAS WITHOUT NULL
        virtual bool on_validate(websocketpp::connection_hdl hdl) = 0;
        void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
        void on_open(websocketpp::connection_hdl hdl);
        void on_close(websocketpp::connection_hdl hdl);
        //test
        void  on_fail(websocketpp::connection_hdl);

        void addActive(websocketpp::connection_hdl hdl, const vector<pair<long, TaskInfo>>& listforIdInfo);

        virtual void close_from_server(websocketpp::connection_hdl hdl, websocketpp::close::status::value const code, std::string const & reason) = 0;
        void _deleteFromActiveConnections(websocketpp::connection_hdl hdl);
        // TODO: MB WAS WITHOUT NULL
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) = 0;

        // TODO: MB WAS WITHOUT NULL
        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) = 0;


        int port;

        const unsigned long maximumBufferSizeMin = 1;
        const unsigned long maximumBufferSizeMax = 10000;
        const unsigned long maximumBufferSizeDef = 1000;

        typedef std::map<websocketpp::connection_hdl, ConnectionData*, std::owner_less<websocketpp::connection_hdl> > con_list;
        con_list m_connections;

        typedef std::set < websocketpp::connection_hdl, std::owner_less < websocketpp::connection_hdl>> con_active_list;
        con_active_list m_active_connections;

        std::mutex m_connection_lock;
        
        // DONE: NOT VIRTUAL
#ifdef SERVER_MODE
        WSTangoConnSer* _tc;
#endif
#ifdef CLIENT_MODE
        WSTangoConnCli* _tc;
#endif
        unsigned long m_next_sessionid;
        log4tango::Logger *logger;
        std::thread *actTh = nullptr;
        bool local_th_exit = false;
        websocketpp::lib::condition_variable m_action_cond;

    private:
        void _forCheckActions();
        bool _checkRequests(websocketpp::connection_hdl hdl, std::unordered_map<long, TaskInfo> &task);
        bool _isAsyncRequest(TYPE_WS_REQ typeWsReq);

        ConnectionData* getConnectionData(websocketpp::connection_hdl hdl);
        bool checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet);
        
        // TODO: CHECK ACTION LOCK
        std::mutex m_action_lock;

    };
}

#endif
