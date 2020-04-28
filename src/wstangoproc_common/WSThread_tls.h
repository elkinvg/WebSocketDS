#ifndef WSTHREAD_TLS
#define WSTHREAD_TLS
#include "WSThread.h"

#include "CurrentMode.h"
namespace WebSocketDS_ns
{
    class WSThread_tls : public WSThread
    {
    public:
#ifdef SERVER_MODE
        WSThread_tls(WSTangoConnSer *tc, int portNumber, string cert, string key);
#endif
#ifdef CLIENT_MODE
        WSThread_tls(WSTangoConnCli *tc, int portNumber, string cert, string key);
#endif
        // TODO: DELETE WSThread_tls(WSTangoConn *tc, int portNumber, string cert, string key);

        ~WSThread_tls();
        virtual void *run_undetached(void *) override;

        virtual  void stop() override;
        virtual void send_all(std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg, bool isLocked) override;
        // Для бинарных данных. Пока не используется.
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) override;
        virtual bool on_validate(websocketpp::connection_hdl hdl) override;
    private:
        context_ptr on_tls_init(websocketpp::connection_hdl hdl);
        std::string get_password();
        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;
        virtual void close_from_server(websocketpp::connection_hdl hdl, websocketpp::close::status::value const code, std::string const & reason, bool isLocked) override;
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) override;

        server_tls m_server;

        std::string certificate_;
        std::string key_;
    };
}

#endif
