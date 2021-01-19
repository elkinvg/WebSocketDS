#include "WSThread_plain.h"

#ifdef SERVER_MODE
#include "WSTangoConnSer.h"
#endif
#ifdef CLIENT_MODE
#include "WSTangoConnCli.h"
#endif

#include <tango.h>
#include <omnithread.h>
#include "StringProc.h"


namespace WebSocketDS_ns
{
#ifdef SERVER_MODE
    WSThread_plain::WSThread_plain(WSTangoConnSer *tc, int portNumber)
#endif
#ifdef CLIENT_MODE
    WSThread_plain::WSThread_plain(WSTangoConnCli *tc, int portNumber)
#endif
        :WSThread(tc, portNumber)
    {
        start_undetached();
    }

    bool WSThread_plain::on_validate(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM << "Check validate WSTHREAD_PLAIN" << endl;
        
        auto conSize = m_connections.size();
        DEBUG_STREAM << "Number of connections: " << conSize << endl;

        if (_tc->getMaxNumberOfConnections() == 0)
            return true;
        if (conSize >= _tc->getMaxNumberOfConnections())
            return false;
        
        return true;
    }

    void *WSThread_plain::run_undetached(void *ptr)
    {
        try {
            DEBUG_STREAM << "The upload thread (PLAIN) starts..." << endl;
            m_server.set_open_handler(websocketpp::lib::bind(&WSThread_plain::on_open, this, websocketpp::lib::placeholders::_1));
            m_server.set_close_handler(websocketpp::lib::bind(&WSThread_plain::on_close, this, websocketpp::lib::placeholders::_1));
            m_server.set_message_handler(websocketpp::lib::bind(&WSThread_plain::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));//m_server.set_user_agent();
            m_server.set_validate_handler(bind(&WSThread_plain::on_validate, this, websocketpp::lib::placeholders::_1));

            m_server.set_fail_handler(bind(&WSThread_plain::on_fail, this, websocketpp::lib::placeholders::_1));

            // this will turn off console output for frame header and payload
            m_server.clear_access_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

            // this will turn off everything in console output
            //m_server.clear_access_channels(websocketpp::log::alevel::all);

            m_server.init_asio();
            m_server.set_reuse_addr(true); // for LINUX
            m_server.listen(port);
            m_server.start_accept();
            m_server.run();
        }
        catch (...) {
            _tc->setFalsedConnectionStatus();
        }
        DEBUG_STREAM << "WS stopped.." << endl;
        return 0;
    }

    void WSThread_plain::send_all(std::string msg) {

        StringProc::removeSymbolsForString(msg);
        int ii=0;
        size_t total = 0;

        unsigned int maxBuffSize = _tc->getMaxBuffSize();
        if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
            maxBuffSize = maximumBufferSizeDef * 1024;
        else
            maxBuffSize = maxBuffSize * 1024;

        vector<std::pair<websocketpp::connection_hdl, string>> for_close;

        // При инициализации WsEvCallBackSer отправляется сообщение, и происходит deadlock
        std::unique_lock<std::mutex> con_lock(m_connection_lock, std::defer_lock);
        if (con_lock.try_lock()) {
            for (const auto& conn : m_connections) {
                try {
                    websocketpp::lib::error_code erc;
                    size_t buffered_amount = get_buffered_amount((conn.first));

                    total += buffered_amount;
                    DEBUG_STREAM << "con: " << ii << " bufersize: " << buffered_amount << " bytes | " << std::fixed << std::setprecision(3) << (buffered_amount / (1024. * 1024.)) << " Mb" << endl;

                    if (buffered_amount < maxBuffSize)
                        m_server.send((conn.first), msg, websocketpp::frame::opcode::text);
                    else {
                        ii++;
                        for_close.push_back(
                            std::make_pair(conn.first, "")
                        );
                        continue;
                    }
                    ii++;
                }
                catch (websocketpp::exception const & e) {
                    string exc = "exception from send_all: " + string(e.what());
                    DEBUG_STREAM << exc;
                    ii++;
                    for_close.push_back(
                        std::make_pair(conn.first, exc)
                    );
                }
                catch (std::exception& e)
                {
                    string exc = "exception from send_all: " + string(e.what());
                    ii++;
                    for_close.push_back(
                        std::make_pair(conn.first, exc)
                    );
                }
                catch (...) {
                    string exc = "unknown error from send_all ";
                    DEBUG_STREAM << exc;
                    ii++;
                    for_close.push_back(
                        std::make_pair(conn.first, exc)
                    );
                }
            }

            // Закрытие соединения со стороны сервера при переполнении буфера, или при ошибках возникших на сервере
            for (auto& cls : for_close) {
                try {
                    if (!cls.second.size()) {
                        close_from_server(cls.first, websocketpp::close::status::going_away, "buffer overloaded");
                    }
                    else {
                        close_from_server(cls.first, websocketpp::close::status::internal_endpoint_error, cls.second);
                    }
                }
                catch(...) {}
                _deleteFromActiveConnections(cls.first);
            }


            DEBUG_STREAM << std::fixed << "total bufersize: " << total << " | " << std::setprecision(3) << (total / (1024.*1024.)) << "  Mb: " << endl;
        }
    }

    void WSThread_plain::send(websocketpp::connection_hdl hdl, std::string msg) {
        if (hdl.expired()) {
            return;
        }
        StringProc::removeSymbolsForString(msg);
        bool closed = false;

        try {
            unsigned int maxBuffSize = _tc->getMaxBuffSize();

            if (maxBuffSize < maximumBufferSizeMin || maxBuffSize > maximumBufferSizeMax)
                maxBuffSize = maximumBufferSizeDef * 1024;
            else
                maxBuffSize = maxBuffSize * 1024;

            size_t buffered_amount = get_buffered_amount(hdl);

            if (buffered_amount < maxBuffSize)
                m_server.send(hdl, msg, websocketpp::frame::opcode::text);
            else
                closed = true;
        }
        catch (websocketpp::exception const & e) {
            string exc = "exception from send: " + string(e.what());
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc);
        }
        catch (std::exception& e) {
            string exc = "exception from send: " + string(e.what());
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc);
        }
        catch (...) {
            string exc = "unknown error from send";
            DEBUG_STREAM << exc;
            close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc);
        }
        if (closed) {
            close_from_server(hdl, websocketpp::close::status::going_away, "buffer overloaded");
        }
    }

    // TODO: NOT USED 
    //void WSThread_plain::send(websocketpp::connection_hdl hdl, const void *data, size_t len)
    //{
    //    // DONE: Помещён в try catch блок
    //    try {
    //        m_server.send(hdl, data, len, websocketpp::frame::opcode::binary);
    //    }
    //    catch (websocketpp::exception const & e) {
    //        // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
    //        string exc = "exception from send: " + string(e.what());
    //        DEBUG_STREAM << exc;
    //        close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, true);
    //    }
    //    catch (std::exception& e) {
    //        // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
    //        string exc = "exception from send: " + string(e.what());
    //        DEBUG_STREAM << exc;
    //        close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, true);
    //    }
    //    catch (...) {
    //        // DONE: Закрывается c std::unique_lock. Выводится сообщение и выставляется статус
    //        string exc = "unknown error from send";
    //        DEBUG_STREAM << exc;
    //        close_from_server(hdl, websocketpp::close::status::internal_endpoint_error, exc, true);
    //    }
    //}

    void WSThread_plain::stop()
    {
        local_th_exit = true;
        m_action_cond.notify_one();

        actTh->join();
        delete actTh;

        DEBUG_STREAM << "The ws thread stops..." << endl;
        m_server.stop();
        DEBUG_STREAM << "WS stops..." << endl;
    }

    unordered_map<string, string> WSThread_plain::getRemoteConf(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        websocketpp::uri_ptr uri = con->get_uri();

        string remoteEndpoint = con->get_remote_endpoint();
        remoteEndpoint = StringProc::parseOfAddress(remoteEndpoint);

        // Получение X-Forwarded-For проксированных запросов
        string xforwarded =  con->get_request_header("X-Forwarded-For");
        xforwarded.erase(remove(xforwarded.begin(), xforwarded.end(), ' '), xforwarded.end());
        vector<string> proxyes = StringProc::parseInputString(xforwarded
, ",", true);

        unordered_map<string, string> parsedGet;

        string query = uri->get_query(); // returns empty string if no query string set.

        if (!query.empty()) {
            parsedGet = StringProc::parseOfGetQuery(query);
        }

        if (proxyes.size() && proxyes[0].size() && proxyes[0] != "::1")
            parsedGet["ip"] = proxyes[0];
        else
            parsedGet["ip"] = remoteEndpoint;

        return parsedGet;
    }

    void WSThread_plain::close_from_server(websocketpp::connection_hdl hdl, websocketpp::close::status::value const code, std::string const & reason) {
        try {
            websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
            con->close(code, reason);
        }
        catch (...){}

        throw ConnectionClosedException();
    }

    size_t WSThread_plain::get_buffered_amount(websocketpp::connection_hdl hdl) {
        websocketpp::server<websocketpp::config::asio>::connection_ptr con = m_server.get_con_from_hdl(hdl);
        return con->get_buffered_amount();
    }

    WSThread_plain::~WSThread_plain() {}

}
