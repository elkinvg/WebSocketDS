#include "ParsingInputJson.h"

#include <boost/property_tree/json_parser.hpp>
#include <tuple>
#include "StringProc.h"
#include "common.h"
#include "CurrentMode.h"
#include "EnumConverter.h"

namespace WebSocketDS_ns
{
    ParsedInputJson ParsingInputJson::parseInputJson(const string &json)
    {
        // Проверка входных парамеров JSON.

        ParsedInputJson parsedInput;
        stringstream ss;
        ss << json;
        boost::property_tree::ptree pt;

        json_arr_map inpVec;
        json_val_map inpStr;
        json_obj_map inpObj;

        string errMess;

        parsedInput.inputJson = json;
        ERROR_TYPE errType{ ERROR_TYPE::UNKNOWN_REQ_TYPE};

        try {
            boost::property_tree::read_json(ss, pt);

            if (pt.find("type_req") == pt.not_found()) {
                parsedInput.type_req = TYPE_WS_REQ::UNKNOWN;
                errMess = "key type_req not found";
            }
            else {
                for (auto& elem : pt) {
                    string tmpStr;
                    // id должен присутствовать, но не обязательно
                    if (elem.first == "id") {
                        tmpStr = elem.second.data();

                        if (tmpStr.size()) {
                            parsedInput.id = tmpStr;
                        }
                        continue;
                    }

                    // type_req должен присутствовать
                    if (elem.first == "type_req") {
                        tmpStr = elem.second.data();
                        if (!tmpStr.size()) {
                            parsedInput.type_req = TYPE_WS_REQ::UNKNOWN;
                            errMess = "type of type_req must be value";
                            break;
                        }
                        // DONE: type_req (TYPE_WS_REQ) высчитывается здесь
                        // type_req_str остаётся для мест где нужны string
                        parsedInput.type_req_str = tmpStr;
                        parsedInput.type_req = EnumConverter::typeWsReqFromString(tmpStr);
                        continue;
                    }

                    // Если является значением
                    if (elem.second.data() != "") {
                        inpStr[elem.first] = elem.second.data();
                        continue;
                    }

                    // Если является массивом
                    vector<string> vecStr;
                    int it = 0;
                    try {
                        for (boost::property_tree::ptree::value_type &v : elem.second) {
                            if (!v.first.size()) {
                                // Массив должен содержать только значения
                                // Проверка содержит ли массив объекты {}
                                if (!v.second.data().size()) {
                                    it = 0;
                                    break;
                                }
                                vecStr.push_back(v.second.data());
                                it++;
                            }
                            else {
                                inpObj[elem.first] = elem.second;
                                break;
                            }
                        }
                    }
                    catch (boost::property_tree::ptree_bad_data) { it = 0; }
                    if (it) {
                        inpVec[elem.first] = vecStr;
                    }
                }
            }
        }
        catch (boost::property_tree::json_parser::json_parser_error &je)
        {
            parsedInput.type_req = TYPE_WS_REQ::UNKNOWN;
            errMess = "Json parsed error. Message from Boost: " + je.message();
            errType = ERROR_TYPE::IS_NOT_VALID;
        }
        catch (...) {
            parsedInput.type_req = TYPE_WS_REQ::UNKNOWN;
            errMess = "unknown error from parseInputJson";
            errType = ERROR_TYPE::IS_NOT_VALID;
        }

        // TODO: CHECK
        // id должен присутствовать, но не обязательно
        if (!parsedInput.id.size()) {
            parsedInput.id = NONE;
        }

        // Если была ошибка
        // Или если не был найден тип запроса
        if (parsedInput.type_req == TYPE_WS_REQ::UNKNOWN) {
            if (!parsedInput.type_req_str.size()) {
                parsedInput.type_req_str = "unknown_req";
            }

            if (errType != ERROR_TYPE::IS_NOT_VALID) {
                errType = ERROR_TYPE::UNKNOWN_REQ_TYPE;
            }
            parsedInput.errMess = StringProc::exceptionStringOut(errType, parsedInput.id, errMess, parsedInput.type_req_str);
            return parsedInput;
        }


        if (inpStr.size()) {
            parsedInput.otherInpStr = move(inpStr);
        }
        if (inpVec.size())
            parsedInput.otherInpVec = move(inpVec);
        if (inpObj.size())
            parsedInput.otherInpObj = move(inpObj);

        _checkValidity(parsedInput);

        return parsedInput;
    }

    vector<string> ParsingInputJson::getArrayOfStr(string key, const ptree& inPtree)
    {
        // Получение массива значений из ptree
        // Соответственно это должен быть либо value либо array
        vector<string> out;

        try {
            // Поправлено. Добавлен сепаратор |
            // По умолчанию в качестве сепаратора принимает . и / или \
            // 
            boost::property_tree::ptree child = inPtree.get_child(ptree::path_type(ptree::path_type{ key, '|' }));
            if (child.data().size())
                return vector<string>({ child.data() });

            for (auto& val : child) {
                if (!val.second.data().size())
                    continue;
                out.push_back(val.second.data());
            }
        }
        catch (...){}
        return out;
    }

#ifdef CLIENT_MODE
    unordered_map<string, vector<string>> ParsingInputJson::_getEventSubList(const ptree & inPtree)
    {
        unordered_map<string, vector<string>> evsublist;

        try {
            for (const auto& elem : inPtree) {
                if (!elem.first.size())
                    continue;
                string devName = elem.first;
                // if array
                if (elem.second.size()) {
                    vector<string> attrs = getArrayOfStr(devName, inPtree);
                    evsublist[devName] = attrs;
                }

                // if value
                if (elem.second.data().size()) {
                    // TODO: CHECK!
                    evsublist[devName] = vector<string>({ elem.second.data() });
                }
            }
        }
        catch (...) {}

        return evsublist;
    }
#endif

    /**
    Проверка правильности запроса - наличие ключей, соблюдение требуемого формата
    */
    void ParsingInputJson::_checkValidity(ParsedInputJson &parsedInput)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;
        // TODO: Переделывать типы

#ifdef SERVER_MODE
        if (parsedInput.type_req_str.find("eventreq") != string::npos) {
            parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP_IN_CURR, parsedInput.id, "Subscribing to events is not supported in the current mode", parsedInput.type_req_str);
            return;
        }
#endif // SERVER_MODE
#ifdef CLIENT_MODE
        if (parsedInput.type_req_str.find("eventreq_add_dev") != string::npos) {
            if (parsedInput.check_key("periodic") == TYPE_OF_VAL::OBJECT) {
                parsedInput.periodicEvSubList = _getEventSubList(parsedInput.otherInpObj.at("periodic"));

                if (!parsedInput.isValid) {
                    parsedInput.isValid = parsedInput.periodicEvSubList.size();
                }
            }

            if (parsedInput.check_key("change") == TYPE_OF_VAL::OBJECT) {
                parsedInput.changeEvSubList = _getEventSubList(parsedInput.otherInpObj.at("change"));

                if (!parsedInput.isValid) {
                    parsedInput.isValid = parsedInput.changeEvSubList.size();
                }
            }

            if (parsedInput.check_key("user") == TYPE_OF_VAL::OBJECT) {
                parsedInput.userEvSubList = _getEventSubList(parsedInput.otherInpObj.at("user"));

                if (!parsedInput.isValid) {
                    parsedInput.isValid = parsedInput.userEvSubList.size();
                }
            }

            if (parsedInput.check_key("archive") == TYPE_OF_VAL::OBJECT) {
                parsedInput.archiveEvSubList = _getEventSubList(parsedInput.otherInpObj.at("archive"));

                if (!parsedInput.isValid) {
                    parsedInput.isValid = parsedInput.archiveEvSubList.size();
                }
            }

            if (!parsedInput.isValid) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "No attribute found for event subscription", parsedInput.type_req_str);
            }

            return;
    }

        if (parsedInput.type_req_str.find("eventreq_rem_dev") != string::npos) {
            if (parsedInput.check_key("event_sub_id") != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "key event_sub_id not found or must be value", parsedInput.type_req_str);
                return;
            }
            parsedInput.isValid = true;
            return;
        }

        if (parsedInput.type_req_str.find("eventreq_check_dev") != string::npos) {
            if (parsedInput.check_keys({ "event_type", "device", "attribute" }) != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "check keys for request", parsedInput.type_req_str);
                return;
            }
            parsedInput.isValid = true;
            return;
        }

        if (parsedInput.type_req_str.find("eventreq_off") != string::npos) {
            parsedInput.isValid = true;
            return;
        }
#endif // CLIENT_MODE

        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            || typeWsReq == TYPE_WS_REQ::COMMAND_DEV
            || typeWsReq == TYPE_WS_REQ::COMMAND_GR
            || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT
            ) {
            // command_name - должен быть для всех командных запросов
            if (parsedInput.check_key("command_name") != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key command_name or command_name is not value ", parsedInput.type_req_str);
                return;
            }

#ifdef SERVER_MODE
            if (
                typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT
                ) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP_IN_CURR, parsedInput.id, "type_req must be command", parsedInput.type_req_str);
                return;
            }
#endif
#ifdef CLIENT_MODE
            // device_name - должен быть для всех запросов клиентного режима
            if (
                parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE
                ) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key device_name or device_name is not value", parsedInput.type_req_str);
                return;
            }
#endif
            parsedInput.type_req = TYPE_WS_REQ::COMMAND;
            parsedInput.isValid = true;
            return;
        }



        if (
            typeWsReq == TYPE_WS_REQ::PIPE_COMM
            || typeWsReq == TYPE_WS_REQ::PIPE_COMM_DEV
            || typeWsReq == TYPE_WS_REQ::PIPE_COMM_GR
            ) {
            if (parsedInput.check_key("pipe_name") != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key pipe_name or pipe_name is not value", parsedInput.type_req_str);
                return;
            }
#ifdef CLIENT_MODE
            // device_name - должен быть для всех запросов клиентного режима
            if (
                parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE
                ) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key device_name or device_name is not value", parsedInput.type_req_str);
                return;
            }
#endif
            parsedInput.type_req = TYPE_WS_REQ::PIPE_COMM;
            parsedInput.isValid = true;
            return;
        }


        if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE_DEV
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE_GR
            || typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR
            ) {
            if (parsedInput.check_key("attr_name") != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key attr_name or attr_name is not value", parsedInput.type_req_str);
                return;
            }

#ifdef CLIENT_MODE
            // device_name - должен быть для всех запросов клиентного режима
            if (
                parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE
                ) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key device_name or device_name is not value", parsedInput.type_req_str);
                return;
            }
#endif
            parsedInput.type_req = TYPE_WS_REQ::ATTRIBUTE_WRITE;
            parsedInput.isValid = true;
            return;
        }


        if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ_GR
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ_DEV
            || typeWsReq == TYPE_WS_REQ::ATTR_GR_CLIENT
            ) {
            if (parsedInput.check_key("attr_name") != TYPE_OF_VAL::VALUE && parsedInput.check_key("attr_name") != TYPE_OF_VAL::ARRAY) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key attr_name or attr_name is not value or array", parsedInput.type_req_str);
                return;
            }

#ifdef CLIENT_MODE
            // device_name - должен быть для всех запросов клиентного режима
            if (
                parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE
                ) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "Not found key device_name or device_name is not value", parsedInput.type_req_str);
                return;
            }
#endif
            parsedInput.type_req = TYPE_WS_REQ::ATTRIBUTE_READ;
            parsedInput.isValid = true;
            return;
        }


        if (
            typeWsReq == TYPE_WS_REQ::CHANGE_USER
            ) {
            if (parsedInput.check_keys({ "login", "password" }) != TYPE_OF_VAL::VALUE) {
                parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "keys login or password not found", parsedInput.type_req_str);
                return;
            }
            parsedInput.isValid = true;
            return;
        }

        if (
            typeWsReq == TYPE_WS_REQ::USER_CHECK_STATUS
            ) {
            parsedInput.isValid = true;
            return;
        }

        parsedInput.isValid = false;
        parsedInput.type_req = TYPE_WS_REQ::UNKNOWN;
        parsedInput.errMess = StringProc::exceptionStringOut(ERROR_TYPE::UNKNOWN_REQ_TYPE, parsedInput.id, "Unknown Request", "unknown_req");
    }

}
