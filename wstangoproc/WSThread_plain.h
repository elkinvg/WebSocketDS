#ifndef WSTHREAD_PLAIN
#define WSTHREAD_PLAIN
#include "WSThread.h"

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class WSThread_plain : public WSThread
    {
    public:
        WSThread_plain(WSTangoConn *tc, int portNumber);

        ~WSThread_plain();
        virtual void *run_undetached(void *) override;
        virtual void stop() override;
        virtual void send_all(std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) override;
        virtual bool on_validate(websocketpp::connection_hdl hdl) override;
    private:
        virtual void close_from_server(websocketpp::connection_hdl hdl) override;
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) override;
        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;

        virtual void startTimer(websocketpp::connection_hdl hdl) override;
        virtual void runTimer(const error_code & ec, websocketpp::connection_hdl hdl, int timerInd) override;

        server m_server;
    };
}

#endif
