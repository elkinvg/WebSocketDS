#include "CurrentMode.h"

#ifdef SERVER_MODE
#include "WSTangoConnSer.h"
#endif
#ifdef CLIENT_MODE
#include "WSTangoConnCli.h"
#endif
#include "WSThread.h"

#include <tango.h>
#include <omnithread.h>

#include "ConnectionData.h"

#include "StringProc.h"
#include "ParsingInputJson.h"


namespace WebSocketDS_ns
{
#ifdef SERVER_MODE
    WSThread::WSThread(WSTangoConnSer *tc, int portNumber)
#endif
#ifdef CLIENT_MODE
    WSThread::WSThread(WSTangoConnCli *tc, int portNumber)
#endif
        : omni_thread(), m_next_sessionid(1), _tc(tc), port(portNumber)
    {
        logger = _tc->get_logger();
        actTh = new thread(&WSThread::checkActions, this);
    }

    void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
        string data_from_client = msg->get_payload();
        INFO_STREAM << " Input message: " << data_from_client << endl;

        ParsedInputJson parsedInput = ParsingInputJson::parseInputJson(data_from_client);

        // TODO: CHECK
        // Проверка типа запроса происходит в WSTangoConn::getTypeWsReq
        // Если вводимый type_req не соответствует ни одному из перечисленных,
        // высылается сообщение об ошибке
        if (!parsedInput.isValid) {
            string errorMess;
            if (parsedInput.errMess.size())
                errorMess = parsedInput.errMess;
            else
                // if (!parsedInput.isValid) Должен быть текст ошибки 
                errorMess = StringProc::exceptionStringOut(ERROR_TYPE::IS_NOT_VALID, NONE, "Unknown message from parsing", parsedInput.type_req_str);
            // Закрытие соединения со стороны сервера при возникновении ошибок
            try {
                send(hdl, errorMess);
            }
            catch (ConnectionClosedException& e) {
                std::unique_lock<std::mutex> con_lock(m_connection_lock);
                _deleteFromActiveConnections(hdl);
            }
            return;
        }

        string resp;
        // TODO: BUG Если девайс не активен. Поместить в try
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;

        if (
            typeWsReq == TYPE_WS_REQ::USER_CHECK_STATUS
            || typeWsReq == TYPE_WS_REQ::CHANGE_USER
            ) {
            resp = _tc->commonRequsts(parsedInput, m_connections[hdl]);

            // Закрытие соединения со стороны сервера при возникновении ошибок
            try {
                send(hdl, resp);
            }
            catch (ConnectionClosedException& e) {
                std::unique_lock<std::mutex> con_lock(m_connection_lock);
                _deleteFromActiveConnections(hdl);
            }
            return;
        }

        if (
            _isAsyncRequest(typeWsReq)
            ) {
            try {
                // TODO: check authentification
                vector<string> errorsFromGroupReq;
                auto forIdInfo = _tc->sendRequestAsync(parsedInput, m_connections[hdl], errorsFromGroupReq);
                if (forIdInfo.size()) {
                    addActive(hdl, forIdInfo);
                }
                m_action_cond.notify_one();
                if (errorsFromGroupReq.size()) {
                    for (auto& err : errorsFromGroupReq) {
                        send(hdl, err);
                    }
                }
                return;
            }
            // Закрытие соединения со стороны сервера при возникновении ошибок
            catch (ConnectionClosedException& e) {
                std::unique_lock<std::mutex> con_lock(m_connection_lock);
                _deleteFromActiveConnections(hdl);
                return;
            }
            // TODO: Проверить, что везде std::runtime_error
            // Переписать Tango::Exception в std::runtime_error
            catch (std::runtime_error &re) {
                resp = re.what();
            }
        }
        else {
            try {
#ifdef CLIENT_MODE
                if (parsedInput.type_req_str.find("eventreq") != string::npos) {
                    resp = _tc->sendRequest_Event(hdl, parsedInput);
                }
                else {
#endif // CLIENT_MODE
                    resp = _tc->sendRequest(parsedInput, m_connections[hdl]);
#ifdef CLIENT_MODE
                }
#endif
            }
            // Для Pipe. Выбрасывается исключения, если например устройство не найдено
            catch (std::runtime_error &re) {
                resp = re.what();
            }
        }
        if (resp.size()) {
            try {
                send(hdl, resp);
            }
            // Закрытие соединения со стороны сервера при возникновении ошибок
            catch (ConnectionClosedException& e) {
                std::unique_lock<std::mutex> con_lock(m_connection_lock);
                _deleteFromActiveConnections(hdl);
                return;
            }
        }
    }

    void WSThread::on_open(websocketpp::connection_hdl hdl) {
        std::unique_lock<std::mutex> con_lock(m_connection_lock);
        m_connections[hdl] = getConnectionData(hdl);
        m_connections[hdl]->sessionId = m_next_sessionid++;

        string runtimeErrorWhat = "";

        try {
            _tc->checkUser(m_connections[hdl]);
        }
        catch (std::runtime_error &re) {
            runtimeErrorWhat = string(re.what());
        }

        try {
            if (runtimeErrorWhat.size()) {
                send(hdl, string(runtimeErrorWhat));
            }
        }
        // Закрытие соединения со стороны сервера при возникновении ошибок
        catch (...) {
            _deleteFromActiveConnections(hdl);
            return;
        }

        _tc->setNumOfConnections(m_connections.size());

        DEBUG_STREAM << "New user has been connected!! sessionId = " << m_connections[hdl]->sessionId << endl;
        DEBUG_STREAM << m_connections.size() << " client connected!!" << endl;
    }

    void WSThread::on_close(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM << "User has been disconnected!!";
#ifdef CLIENT_MODE
        _tc->clientDisconnected(hdl);
#endif // CLIENT_MODE

        std::unique_lock<std::mutex> con_lock(m_connection_lock);

        _deleteFromActiveConnections(hdl);
        delete m_connections[hdl];
        m_connections.erase(hdl);
        _tc->setNumOfConnections(m_connections.size());
        DEBUG_STREAM << m_connections.size() << " client connected!!" << endl;

    }

    void  WSThread::on_fail(websocketpp::connection_hdl hdl) {
        ERROR_STREAM << " Fail from WSThread on_fail " << endl;
    }

    void WSThread::addActive(websocketpp::connection_hdl hdl, const vector<pair<long, TaskInfo>>& listforIdInfo)
    {
        std::unique_lock<std::mutex> con_lock(m_connection_lock);
        if (m_active_connections.find(hdl) == m_active_connections.end()) {
            // DONE: В параллельном потоке, при возникновении исключений, в момент пока mutex занят, закрывается соединение
            // здесь добавлялось hdl в m_active_connections
            if (m_connections.find(hdl) == m_connections.end()) {
                return;
            }
            m_active_connections.insert(hdl);
        }
        for (auto &forIdInfo : listforIdInfo) {
            m_connections[hdl]->idCommandInfo.insert(forIdInfo);
        }
    }

    void WSThread::_deleteFromActiveConnections(websocketpp::connection_hdl hdl)
    {
        if (m_active_connections.find(hdl) != m_active_connections.end()) {
            m_active_connections.erase(hdl);
        }
    }

    void WSThread::_forCheckActions()
    {
        vector<websocketpp::connection_hdl> _del_active_conn;
        vector<websocketpp::connection_hdl> _del_conn;

        for (auto& _hdl : m_active_connections) {
            try {
                bool _delete = _checkRequests(_hdl, m_connections[_hdl]->idCommandInfo);

                if (_delete) {
                    _del_active_conn.push_back(_hdl);
                }
            }
            // Пока соединения закрываются при любых исключениях от WebSocket.
            // TODO: Рассмотреть исключения, при которых соединения можно не закрывать.
            catch (ConnectionClosedException& e) {
                _del_conn.push_back(_hdl);
            }
        }

        for (auto & _hdl : _del_active_conn) {
            _deleteFromActiveConnections(_hdl);
        }

        closeConnections(_del_conn);
    }

    bool WSThread::_checkRequests(websocketpp::connection_hdl hdl, std::unordered_map<long, TaskInfo>& task)
    {
        if (!task.size()) {
            return true;
        }

        vector<long> _del;
        for (auto& _idInfo : task) {
            long _id = _idInfo.first;

            try {
                string resp = _tc->checkResponse(_idInfo);
                send(hdl, resp);
                _del.push_back(_id);
            }
            catch (Tango::AsynReplyNotArrived) {}
            catch (Tango::DevFailed &e) {
                vector<string> errs;
                for (int i = 0; i < e.errors.length(); i++) {
                    errs.push_back((string)e.errors[i].desc);
                }
                string resp = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, _idInfo.second.idReq, errs, _idInfo.second.typeReqStr);
                send(hdl, resp);
                _del.push_back(_id);
            }
        }
        
        for (auto& _dd : _del) {
            task.erase(_dd);
        }
        return task.size() == 0;
    }

    bool WSThread::_isAsyncRequest(TYPE_WS_REQ typeWsReq)
    {
        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            ) {
            return true;
        }
        return false;
    }

    WSThread::~WSThread() {
    }

    log4tango::Logger * WSThread::get_logger(void)
    {
        return _tc->get_logger();
    }

    ConnectionData* WSThread::getConnectionData(websocketpp::connection_hdl hdl) {
        ConnectionData* conn_data = new ConnectionData();
        auto parsedGet = getRemoteConf(hdl);

        auto typeOfIdent = _tc->getTypeOfIdent();

        // If login and password not found in GET
        if (!checkKeysFromParsedGet(parsedGet)) {
            conn_data->userCheckStatus = false;
        }

        conn_data->login = parsedGet["login"];
        conn_data->ip_client = parsedGet["ip"];

        conn_data->password = parsedGet["password"];

        if (typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
            // Для аутентификации в Егоровом AuthDS в check_permissions_www
            // При подключении не проводится check_user. 
            conn_data->userCheckStatus = true;
        }

        return conn_data;
    }

    bool WSThread::checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet)
    {
        if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end() || parsedGet.find("ip") == parsedGet.end()) {
            DEBUG_STREAM << "login or password or ip not found" << endl;
            return false;
        }
        return true;
    }

    void WSThread::checkActions()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(m_connection_lock);
            m_action_cond.wait(lock, [&]() {return !m_active_connections.empty() || local_th_exit; });
            if (local_th_exit) {
                lock.unlock();
                break;
            }
            lock.unlock();
            while (!m_active_connections.empty()) {
                lock.lock();
                _forCheckActions();
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void WSThread::closeConnections(vector<websocketpp::connection_hdl>& _del_conn)
    {
        for (auto & _hdl : _del_conn) {
            _deleteFromActiveConnections(_hdl);
        }
    }
}
