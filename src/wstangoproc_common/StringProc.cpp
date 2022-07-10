#include "StringProc.h"
#include <sstream>
#include "common.h"
#include <algorithm>
#include "ParsingInputJson.h"

#ifdef CLIENT_MODE
#include "ResponseFromEvent.h"
#endif

#include "EnumConverter.h"
#include "ErrorInfo.h"

using std::stringstream;

namespace WebSocketDS_ns
{
    /**
     * Вывод для запроса, в случае успеха
     */
    string StringProc::successRespOut(const ParsedInputJson & parsedInput)
    {
        stringstream ss;
        ss << "{";
        ss << "\"event\":\"read\", ";
        ss << "\"type_req\": \"" << parsedInput.type_req_str << "\", ";
        try {
            auto idTmp = stoi(parsedInput.id);
            ss << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {}

        ss << "\"success\": true";
        ss << "}";
        return ss.str();
    }

    /**
     * Вывод для запроса, с указанием данных
     */
    string StringProc::responseStringOut(const string& id, string& message, const string& type_req_str, bool isString)
    {
        if (isString)
            message = ("\"" + message + "\"");
        return generateRespMess(id,message,type_req_str);
    }

    /**
     * Вывод для запроса, с указанием данных
     */
    std::string StringProc::responseStringOut(const string& id, const vector<std::string>& messages, const string& type_req_str)
    {
        string inpMess;
        inpMess += "[";
        bool first = true;
        for (auto& mess: messages) {
            if (first) {
                inpMess += ("\"" + mess+ "\"");
                first = false;
            }
            else
                inpMess += (", \"" + mess+ "\"");
        }
        inpMess += "]";
        return generateRespMess(id, inpMess, type_req_str);
    }

#ifdef CLIENT_MODE
    /**
     * Вывод для запросов к подписке на события
     */
    string StringProc::responseStringOutForEventSub(const string& id, const string& type_req_str, const vector<ResponseFromEventReq>& successResponses)
    {
        return generateRespMess(
            id
            , generateSuccessStringOutForEvent(successResponses)
            , type_req_str
        );
    }
#endif

    /**
     * JSON - сообщение об ошибке
     */
    string StringProc::exceptionStringOut(const ErrorInfo & errorInfo)
    {
        stringstream ss;
        ss << "{\"event\": \"error\"";
        if (errorInfo.typeofReq.size()) {
            ss << " ,\"type_req\": \"" << errorInfo.typeofReq << "\"";
        }

        ss << " ,\"type_err\": \"" << EnumConverter::errorTypeToString(errorInfo.typeofError) << "\"";

        if (errorInfo.id.size()) {
            try {
                auto idTmp = stoi(errorInfo.id);
                ss << " ,\"id_req\": " << idTmp;
            }
            catch (...) {}
        }

        if (errorInfo.device_name.size()) {
            ss << " ,\"device_name\": \"" << errorInfo.device_name << "\"";
        }

        // Имя атрибута указывается обычно при работе с событиями
        if (errorInfo.attr_name.size()) {
            ss << " ,\"attr_name\": \"" << errorInfo.attr_name << "\"";
        }

        if (errorInfo.event_type.size()) {
            ss << " ,\"event_type\": \"" << errorInfo.event_type << "\"";
        }
#ifdef CLIENT_MODE
        if (errorInfo.errorResponses.size()) {
            ss << " ,\"errors\": " << generateSuccessStringOutForEvent(errorInfo.errorResponses);
        }
#endif

        if (errorInfo.errorMessage.size() || errorInfo.errorMessages.size()) {
            if (errorInfo.errorMessage.size()) {
                generateErrorMess(ss, errorInfo.errorMessage);
            }
            else {
                generateErrorMess(ss, errorInfo.errorMessages);
            }
        }
        ss << "}";
        return ss.str();
    }

    // TODO: NOT_USES
    bool StringProc::isNameAlias(const string& deviceName)
    {
        bool isAlias = false;
        int slashNum = 0;
        for (auto& chr : deviceName) {
            if (chr == '/')
                slashNum++;
        }
        // for tango://host:10000/device/name/1
        // or device/name/1
        if (slashNum != 2 && slashNum != 5)
            isAlias = true;
        return isAlias;
    }

    void StringProc::removeSymbolsForString(string &str) {
        //if (str.find('\0') != string::npos)
        //    str.erase(remove(str.begin(), str.end(), '\0'), str.end());
        if (str.find('\r') != string::npos)
            str.erase(remove(str.begin(), str.end(), '\r'), str.end());
        if (str.find('\n') != string::npos)
            std::replace(str.begin(), str.end(), '\n', ' ');
    }

    std::vector<std::string> StringProc::parseInputString(string &inp, string delimiter, bool isAllParts) {
        // Если isAllParts == true в вектор помещаются все части входящей строки
        // Иначе все кроме первой (для команд и аттрибутов первая часть содержит имя)
        // По умолчанию isAllParts == false
        // Из входящей строки inp вырезаются всё, что расположено после delimiter
        string s = inp;
        //std::string delimiter = ";";
        std::string token;
        string nameAttr;
        std::vector<std::string> parsed;

        size_t pos = 0;
        bool firstiter = true;

        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            if (firstiter) {
                firstiter = false;
                nameAttr = token;
            }
            else
                parsed.push_back(token);
            s.erase(0, pos + delimiter.length());
        }

        if (!firstiter)
            parsed.push_back(s);

        if (parsed.size() == 0) {
            if (isAllParts)
                parsed.insert(parsed.begin(), inp);
                //parsed.push_back(inp);
            inp = s;
            return parsed;
        }
        inp = nameAttr;
        if (isAllParts)
            parsed.insert(parsed.begin(), inp);
            //parsed.push_back(inp);

        return parsed;
    }

    std::string StringProc::generateRespMess(const string& id, const string &inMessage, const string& type_req_str)
    {
        stringstream ss;
        ss << "{";
        ss << "\"event\":\"read\", ";
        ss << "\"type_req\": \"" << type_req_str << "\", ";
        try {
            auto idTmp = stoi(id);
            ss << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {}

        ss << "\"resp\": " << inMessage;
        ss << "}";
        return ss.str();
    }

    std::string StringProc::generateRespMess(const string& id, const string &inMessage, const string& errMessage, const string& type_req_str)
    {
        stringstream ss;
        ss << "{";
        ss << "\"event\":\"read\", ";
        ss << "\"type_req\": \"" << type_req_str << "\", ";
        try {
            auto idTmp = stoi(id);
            ss << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {}

        ss << "\"resp\": " << inMessage;
        ss << ", \"errors\": " << errMessage;
        ss << "}";
        return ss.str();
    }

#ifdef CLIENT_MODE
    string StringProc::generateSuccessStringOutForEvent(const vector<ResponseFromEventReq>& successResponses)
    {
        stringstream ss;
        ss << "[";
        for (int i = 0; i < successResponses.size(); i++) {
            if (i) {
                ss << ", ";
            }
            ss << "{";
            ss << "\"device\": \"" << successResponses[i].deviceName << "\"";
            if (successResponses[i].attrName.size()) {
                ss << ", \"attribute\": \"" << successResponses[i].attrName << "\"";
            }
            ss << ", \"event_type\": \"" << successResponses[i].eventTypeStr << "\"";
            ss << ", \"event_sub_id\": " << successResponses[i].eventSubId;
            ss << "}";
        }
        ss << "]";
        return ss.str();
    }
#endif

    std::pair<string, string> StringProc::splitDeviceName(const string& deviceName)
    {
        std::pair<string, string> hostAndDev;
        auto cpDevName = deviceName;
        vector<string> parsedDeviceName = parseInputString(cpDevName, "/", true);

        if (parsedDeviceName.size() == 3) {
            hostAndDev.second = parsedDeviceName[0] + "/" + parsedDeviceName[1] + "/" + parsedDeviceName[2];
        }
        else if (parsedDeviceName.size() == 6) {
            hostAndDev.first = parsedDeviceName[0] + "//" + parsedDeviceName[2] + "/";
            hostAndDev.second = parsedDeviceName[3] + "/" + parsedDeviceName[4] + "/" + parsedDeviceName[5];
        }
        return hostAndDev;
    }

    unordered_map<string, string> StringProc::parseOfGetQuery(const string& query)
    {
        // For parsing of get Query
        //
        // ws://address?arg1=val&arg2=val&arg3=val
        // For check_permission method Query mustcontain arguments "login" and "password"
        // If defined #USERANDIDENT method Query must contain 
        // arguments "login", "id_ri","rand_ident_hash" and "rand_ident"

        unordered_map<string, string> outMap;
        if (query.size() == 0)
            return outMap;

        vector<string> outSplit;
        outSplit.clear();
        outSplit = split(query, '&');

        for (auto &str : outSplit) {
            vector<string> tmpStr = split(str, '=');
            if (tmpStr.size() != 2)
                continue;
            outMap.insert(std::pair<string, string>(tmpStr[0], tmpStr[1]));
        }

        return outMap;
    }

    string StringProc::parseOfAddress(const string& addrFromConn)
    {
        string out = "";
        string tmpFnd;

        std::size_t found = addrFromConn.find("]");
        if (found == std::string::npos)
            return out;

        tmpFnd = addrFromConn.substr(0, found);

        found = tmpFnd.find_last_of(":");
        if (found == std::string::npos)
            return out;

        out = tmpFnd.substr(found + 1);

        return out;
    }

    vector<string>& StringProc::split(const string & s, char delim, vector<string>& elems)
    {
        stringstream ss(s);
        string item;
        while (getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    vector<string> StringProc::split(const string & s, char delim)
    {
        vector<string> elems;
        split(s, delim, elems);
        return elems;
    }

    string StringProc::checkPrecisionOptions(string & reqName, const ParsedInputJson & parsedInput)
    {
        string optStr = ""; // string from received opt

        if (parsedInput.check_key("precision") == TYPE_OF_VAL::VALUE) {
            optStr = parsedInput.otherInpStr.at("precision");
        }

        vector<string> gotOptions = StringProc::parseInputString(reqName, ";");

        if (!optStr.size()) {
            for (auto &opt : gotOptions) {
                auto iterator_attr = opt.find(OPT_PREC);
                if (iterator_attr != string::npos) {
                    optStr = opt;
                    break;
                }
            }
        }

        return optStr;
    }

    std::unordered_map<std::string, std::string> StringProc::checkPrecisionOptions(vector<string>& reqName, const ParsedInputJson & parsedInput)
    {
        std::unordered_map<std::string, std::string> optStr;

        // DONE: precision для всех атрибутов из списка
        if (parsedInput.check_key("precision") == TYPE_OF_VAL::VALUE) {
            string prec = parsedInput.otherInpStr.at("precision");
            for (int i = 0; i < reqName.size(); i++) {
                // Удаление ненужных постфиксов
                StringProc::parseInputString(reqName[i], ";");
                optStr[reqName[i]] = prec;
            }
            return optStr;
        }

        // DONE: precision для отдельных атрибутов из списка. Передаётся массивом
        if (parsedInput.check_key("precision") == TYPE_OF_VAL::ARRAY) {
            vector<string> prec = parsedInput.otherInpVec.at("precision");
            auto sz = prec.size();

            for (int i = 0; i < reqName.size(); i++) {
                // Удаление ненужных постфиксов
                StringProc::parseInputString(reqName[i], ";");
                if (i < sz) {
                    optStr[reqName[i]] = prec[i];
                }
            }
            return optStr;
        }

        // DONE: precision options in OBJECT
        if (parsedInput.check_key("precision") == TYPE_OF_VAL::OBJECT) {
            return checkPrecisionOptions(parsedInput);
        }

        for (int i = 0; i < reqName.size(); i++) {
            vector<string> gotOptions = StringProc::parseInputString(reqName[i], ";");
            for (auto &opt : gotOptions) {
                auto iterator_attr = opt.find(OPT_PREC);
                if (iterator_attr != string::npos) {
                    optStr[reqName[i]] = opt;
                    break;
                }
            }
        }

        return optStr;
    }

    void StringProc::checkPrecisionOptions(string & attrName, std::unordered_map<std::string, std::string>& _precisionOpts)
    {
        vector<string> gotOptions = StringProc::parseInputString(attrName, ";");
        for (auto &opt : gotOptions) {
            auto iterator_attr = opt.find(OPT_PREC);
            if (iterator_attr != string::npos) {
                _precisionOpts.insert(
                    make_pair(attrName, opt)
                );
                break;
            }
        }
    }

    std::unordered_map<std::string, std::string> StringProc::checkPrecisionOptions(const ParsedInputJson & parsedInput)
    {
        std::unordered_map<std::string, std::string> optStr;

        // DONE: precision options in OBJECT
        if (parsedInput.check_key("precision") == TYPE_OF_VAL::OBJECT) {
            ptree attributes = parsedInput.otherInpObj.at("precision");
            try {
                for (const auto& elem : attributes) {
                    if (!elem.first.size())
                        continue;
                    string devName = elem.first;
                    // if value
                    if (elem.second.data().size()) {
                        optStr[devName] = elem.second.data();
                    }
                }
            }
            catch (...) {}
        }
        return optStr;
    }

    void StringProc::generateErrorMess(stringstream & ss, const string & errorMessage)
    {
        ss << ", \"err_mess\": \"" << errorMessage << "\"";
    }

    void StringProc::generateErrorMess(stringstream & ss, const vector<string>& errorMessages)
    {
        ss << ", \"err_mess\": [";

        int it = 0;
        for (const auto& mess : errorMessages) {
            if (it)
                ss << ", ";
            ss << "\"" << mess << "\"";
            it++;
        }
        ss << "]";
    }
}
