#include "WSTangoConn.h"

#include "StringProc.h"
#include "UserControl.h"
#include "ConnectionData.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    WSTangoConn::WSTangoConn() {}

    unsigned short WSTangoConn::getMaxNumberOfConnections()
    {
        return _maxNumOfConnection;
    }

    unsigned int WSTangoConn::getMaxBuffSize()
    {
        return _maxBufferSize;
    }

    TYPE_OF_IDENT WSTangoConn::getTypeOfIdent()
    {
        return typeOfIdent;
    }

    void WSTangoConn::checkUser(ConnectionData* connData)
    {
        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE) {

            if (connData->login.size()) {
                uc->check_user(connData->login, connData->password);
                connData->userCheckStatus = true;
            }
            else {
                connData->userCheckStatus = false;
            }
        }

        if (typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
            // Для аутентификации в Егоровом AuthDS в check_permissions_www
            // В данном случае логин и пароль просто сохраняется.
            if (connData->login.size() && connData->password.size()) {
                connData->userCheckStatus = true;
            }
            else {
                connData->userCheckStatus = false;
            }
        }
    }

    void WSTangoConn::initOptions()
    {
        for (auto& opt : getDevOptions()) {
#ifdef SERVER_MODE
            if (opt == "group")
                _isGroup = true;
#endif
            if (opt == "uselog") {
                uc->setLogActive();
            }
            // TODO: ADD TO README
            // TODO: Доделать и проверить useoldjson
            if (opt == "useoldjson") {
                _isOldVersionOfJson = true;
            }
            if (opt.find("tident") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    //if (gettedIdentOpt[1] == "rndid")
                    //    typeOfIdent = TYPE_OF_IDENT::RANDIDENT;
                    if (gettedIdentOpt[1] == "smpl")
                        typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                    // Для аутентификации в Егоровом AuthDS в check_permissions_www
                    else if (gettedIdentOpt[1] == "permission_www") {
                        typeOfIdent = TYPE_OF_IDENT::PERMISSION_WWW;
                        uc->setTypeOfIdent(typeOfIdent);
                    }
                    else
                        typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                }
            }

            // TODO: ADD TO README
            if (opt.find("command_name_for_check_user") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    uc->setCommandNameForCheckUser(gettedIdentOpt[1]);
                }
            }

            // TODO: ADD TO README
            if (opt.find("command_name_for_check_permission") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    uc->setCommandNameForCheckPermission(gettedIdentOpt[1]);
                }
            }

            // TODO: ADD TO README
            if (opt.find("command_name_for_log") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    uc->setCommandNameForLog(gettedIdentOpt[1]);
                }
            }

            // TODO: ADD TO README
            if (opt.find("maxnconn") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    try {
                        int nconn = std::stoi(gettedIdentOpt[1]);
                        if (nconn >= 0 && nconn <= 65535) {
                            _maxNumOfConnection = nconn;
                        }
                    }
                    catch (...) {}
                }
            }

            // TODO: ADD TO README
            if (opt.find("maxbuffsize") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    try {
                        int nconn = std::stoi(gettedIdentOpt[1]);
                        if (nconn >= 1 && nconn <= 10000) {
                            _maxBufferSize = nconn;
                        }
                    }
                    catch (...) {}
                }
            }
        }
    }

    string WSTangoConn::_forCheckResponse(const std::pair<long, TaskInfo>& idInfo, Tango::Group * groupForWs)
    {
        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::COMMAND) {
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::GROUP) {
                return TangoProcessor::processCommandResponse(groupForWs, idInfo, _isOldVersionOfJson);
            }
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE_FROM_GROUP) {
                return TangoProcessor::processCommandResponse(groupForWs, idInfo, _isOldVersionOfJson);
            }
#endif
        }

        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_READ) {
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::GROUP) {
                return TangoProcessor::processAttrReadResponse(groupForWs, idInfo, _isOldVersionOfJson);
            }
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE_FROM_GROUP) {
                return TangoProcessor::processAttrReadResponse(groupForWs, idInfo, _isOldVersionOfJson);
            }
#endif
        }

        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::GROUP) {
                return TangoProcessor::processAttrWriteResponse(groupForWs, idInfo);
            }
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE_FROM_GROUP) {
                return TangoProcessor::processAttrWriteResponse(groupForWs, idInfo);
            }
#endif
        }

        return StringProc::exceptionStringOut(ERROR_TYPE::CHECK_CODE, idInfo.second.idReq, "CHECK WSTangoConn::_forCheckResponse", "unknown");
    }

    string WSTangoConn::_forCheckResponse(const std::pair<long, TaskInfo>& idInfo, Tango::DeviceProxy * deviceForWs)
    {
        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::COMMAND) {
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE) {

                return TangoProcessor::processCommandResponse(deviceForWs, idInfo, _isOldVersionOfJson);
            }
#endif // SERVER_MODE
#ifdef CLIENT_MODE
            return TangoProcessor::processCommandResponse(deviceForWs, idInfo, _isOldVersionOfJson);
#endif // CLIENT_MODE

        }

        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_READ) {
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE) {
                return TangoProcessor::processAttrReadResponse(deviceForWs, idInfo, _isOldVersionOfJson);
            }
#endif // SERVER_MODE
#ifdef CLIENT_MODE
            return TangoProcessor::processAttrReadResponse(deviceForWs, idInfo, _isOldVersionOfJson);
#endif // CLIENT_MODE
        }

        if (idInfo.second.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
#ifdef SERVER_MODE
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::SINGLE) {
                return TangoProcessor::processAttrWriteResponse(deviceForWs, idInfo);
            }
#endif // SERVER_MODE
#ifdef CLIENT_MODE
            return TangoProcessor::processAttrWriteResponse(deviceForWs, idInfo);
#endif // CLIENT_MODE
        }

        return StringProc::exceptionStringOut(ERROR_TYPE::CHECK_CODE, idInfo.second.idReq, "CHECK WSTangoConn::_forCheckResponse", "unknown");
    }

    string WSTangoConn::fromException(Tango::DevFailed &e, string func)
    {
        string outErrMess;
        auto lnh = e.errors.length();
        for (unsigned int i = 0; i < lnh; i++) {
            if (i)
                outErrMess += " ";
            auto logger = get_logger();
            if (logger != nullptr) {
                logger->error_stream() << " From " + func + ": " << e.errors[i].desc;
            }
            
            outErrMess += e.errors[i].desc;
        }
        return outErrMess;
    }

    void WSTangoConn::removeSymbolsForString(string &str) {
        //if (str.find('\0') != string::npos)
        //    str.erase(remove(str.begin(), str.end(), '\0'), str.end());
        if (str.find('\r') != string::npos)
            str.erase(remove(str.begin(), str.end(), '\r'), str.end());
        if (str.find('\n') != string::npos)
            std::replace(str.begin(), str.end(), '\n', ' ');
    }

    void WSTangoConn::checkPermissionForRequest(const ParsedInputJson& parsedInput, ConnectionData* connData, const string& device_name, bool isGroupReqest)
    {
        string mess;

        // Пока логин пароль (и прочие аут-данные) вводятся при запросе
        // В дальнейшем, когда будет режим проверки во время сессии
        // дописать дополнительно проверку режима.
        // TODO: CHECK!!!
        if (!connData->userCheckStatus) {
            mess = "User " + connData->login + " has not been authenticated";
            INFO_STREAM << mess;

            throw std::runtime_error(
                StringProc::exceptionStringOut(ERROR_TYPE::AUTH_PERM, parsedInput.id, mess, parsedInput.type_req_str)
            );
        }

        mess = uc->check_permission(parsedInput, connData, device_name, isGroupReqest);

        if (mess.size()) {
            INFO_STREAM << mess;
        }
    }

    string WSTangoConn::sendRequest_ForAuth(const ParsedInputJson& parsedInput, ConnectionData* connData) {
        // Производится проверка пользователя
        // Если проверка проводилась ранее, и была успешна, пользователь поменяется только если текущая проверка также успешна

        string login = parsedInput.otherInpStr.at("login");
        string pass = parsedInput.otherInpStr.at("password");

        if (!login.size() || !pass.size()) {
            return StringProc::exceptionStringOut(ERROR_TYPE::AUTH_CHECK
, parsedInput.id, "keys login and password should not be empty", parsedInput.type_req_str);
        }

        if (connData->login.size())
            // Информация о последнем клиенте хранится connData
            if (connData->login == login && connData->userCheckStatus)
            {
                string tmpms = "User " + connData->login + " successfully authenticated";
                return StringProc::responseStringOut(parsedInput.id, tmpms, parsedInput.type_req_str);
            }

        bool userCheckStatus;

        if (typeOfIdent != TYPE_OF_IDENT::PERMISSION_WWW) {
            uc->check_user(login, pass);
            userCheckStatus = true;
        }
        else {
            // Для аутентификации в Егоровом AuthDS в check_permissions_www
            // При подключении не проводится check_user. 
            userCheckStatus = true;
        }

        connData->userCheckStatus = userCheckStatus;
        connData->login = login;
        connData->password = pass;
        string tmpms = "User " + connData->login + " successfully authenticated";

        return StringProc::responseStringOut(parsedInput.id, tmpms, parsedInput.type_req_str);
    }

    string WSTangoConn::commonRequsts(const ParsedInputJson & inputReq, ConnectionData * connData)
    {
        TYPE_WS_REQ typeWsReq = inputReq.type_req;

        if (typeWsReq == TYPE_WS_REQ::USER_CHECK_STATUS) {
            return sendRequest_UserStatus(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::CHANGE_USER) {
            string mess;
            try {
                mess = sendRequest_ForAuth(inputReq, connData);
            }
            catch (std::runtime_error &re) {
                mess = re.what();
            }
            return mess;
        }
    }

    string WSTangoConn::sendRequest_UserStatus(const ParsedInputJson& parsedInput, ConnectionData* connData)
    {
        // Проверка статуса клиента. Возвращает результат запроса check_user
        bool user_status = connData->userCheckStatus;

        std::stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"" << parsedInput.type_req_str << "\", \"data\": {";
        json << "\"status\": " << boolalpha << user_status;
        json << ", \"username\": \"" << connData->login << "\"";
        json << "}";
        json << "}";

        return json.str();
    }

    WSTangoConn::~WSTangoConn()
    {
    }
}
