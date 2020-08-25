#ifndef WSTHREAD_PLAIN
#define WSTHREAD_PLAIN
#include "WSThread.h"
#include "CurrentMode.h"

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class WSThread_plain : public WSThread
    {
    public:
#ifdef SERVER_MODE
        WSThread_plain(WSTangoConnSer *tc, int portNumber);
#endif
#ifdef CLIENT_MODE
        WSThread_plain(WSTangoConnCli *tc, int portNumber);
#endif
        // TODO: DELETE WSThread_plain(WSTangoConn *tc, int portNumber);

        ~WSThread_plain();
        virtual void *run_undetached(void *) override;
        virtual void stop() override;
        virtual void send_all(std::string msg) override;
        virtual void send(websocketpp::connection_hdl hdl, std::string msg, bool isLocked) override;
        // Для бинарных данных. Пока не используется.
        virtual void send(websocketpp::connection_hdl hdl, const void *data, size_t len) override;
        virtual bool on_validate(websocketpp::connection_hdl hdl) override;
    private:
        virtual void close_from_server(websocketpp::connection_hdl hdl, websocketpp::close::status::value const code,
            std::string const & reason, bool isLocked) override;
        virtual size_t get_buffered_amount(websocketpp::connection_hdl hdl) override;
        virtual std::unordered_map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;

        server m_server;
    };
}

#endif
