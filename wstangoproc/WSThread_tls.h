#ifndef WSTHREAD_TLS
#define WSTHREAD_TLS
#include "WSThread.h"

namespace WebSocketDS_ns
{
    class WSThread_tls : public WSThread
    {
    public:
        WSThread_tls(/*WebSocketDS *dev,*/ /*pair<string, string> deviceServerAndOptions,*/WSTangoConn *tc, int portNumber, string cert, string key);

        ~WSThread_tls();
        virtual void *run_undetached(void *) override;

        virtual  void stop() override;
        virtual void send_all(std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) override;
        virtual bool on_validate(websocketpp::connection_hdl hdl) override;
    private:
        context_ptr on_tls_init(websocketpp::connection_hdl hdl);
        std::string get_password();
        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;
        virtual void close_from_server(websocketpp::connection_hdl hdl) override;
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) override;
        server_tls m_server;

        std::string certificate_;
        std::string key_;
    };
}

#endif
