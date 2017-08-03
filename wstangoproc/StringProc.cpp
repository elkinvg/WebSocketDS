#include "StringProc.h"
#include <sstream>
//#include <boost/property_tree/json_parser.hpp>
#include "common.h"
#include <algorithm>

using std::stringstream;

namespace WebSocketDS_ns
{
    string StringProc::responseStringOut(string id, string message, string type_req_str, bool isString)
    {
        if (isString)
            message = ("\"" + message + "\"");
        return generateRespMess(id,message,type_req_str);
    }

    std::string StringProc::responseStringOut(std::string id, vector<std::string> messages, std::string type_req_str)
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

    string StringProc::exceptionStringOut(string id, string commandOrPipeName, string errorMessage, string type_req_str)
    {
        string tmpMess = "\"err_mess\": \"" + errorMessage + "\"";
        return generateExceptionMess(id, commandOrPipeName, tmpMess, type_req_str);
    }

    string StringProc::exceptionStringOut(string id, string commandOrPipeName, vector<string> errorMessages, string type_req_str) {
        string errMess = "\"err_mess\": [";
        int it = 0;
        for (auto& mess : errorMessages) {
            if (it)
                errMess += ", ";
            errMess = errMess + "\"" + mess + "\"";
            it++;
        }
        errMess += "]";
        return generateExceptionMess(id, commandOrPipeName, errMess, type_req_str);
    }

    string StringProc::exceptionStringOut(string id, string commandOrPipeName, pair<string, string> errorMessages, string type_req_str)
    {
        string errMess = "\"err_mess\": [";
        errMess += ("\"" + errorMessages.first + "\", ");
        errMess += ("\"" + errorMessages.second + "\"");
        errMess += "]";
        return generateExceptionMess(id, commandOrPipeName, errMess, type_req_str);
    }

    string StringProc::exceptionStringOutForEvent(string id, vector<pair<string, vector<string>>>& devNamesAndExc, string type_req_str, string errorType)
    {
        stringstream ss;
        ss << "\"type_err\": \"" << errorType << "\", ";
        ss << "\"err_mess\": {";
        bool nfstdev = false;
        for (auto& devNExc : devNamesAndExc) {
            if (nfstdev)
                ss << ", ";
            else
                nfstdev = true;
            ss << "\"" << devNExc.first << "\": [";
            bool nfstExc = false;
            for (auto& exc : devNExc.second) {
                if (nfstExc)
                    ss << ", ";
                else
                    nfstExc = true;
                ss << "\"" << exc << "\"";
            }
            ss << "]";
        }
        ss << "}";
        return generateExceptionMess(id, NONE, ss.str(), type_req_str);
    }

    string StringProc::exceptionStringOutForEvent(string id, vector<string> devices, string type_req_str, string errorType)
    {
        stringstream ss;
        ss << "\"type_err\": \"" << errorType << "\", ";
        if (errorType == "not_aliases")
            ss << "\"err_mess\": \"In the current mode only aliases are used.\", ";
        if (errorType == "already_pushed")
            ss << "\"err_mess\": \"The following attributes are already included in the list.\", ";
        ss << "\"devices\": [";
        bool nfst = false;
        for (auto& dev : devices) {
            if (nfst)
                ss << ", ";
            else
                nfst = true;
            ss << "\"" << dev << "\"";
        }
        ss << "]";

        return generateExceptionMess(id, NONE, ss.str(), type_req_str);
    }

    string StringProc::exceptionStringOut(string errorMessage) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << "attribute" << "\", ";
        ss << "\"err_mess\": \"" << errorMessage << "\"";
        ss << "}";

        return ss.str();
    }

    string StringProc::exceptionStringOut(string errorMessage, string type_attr_resp) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << type_attr_resp << "\", ";
        ss << "\"err_mess\": \"" << errorMessage << "\"";
        ss << "}";

        return ss.str();
    }

    string StringProc::exceptionStringOut(vector<string> errorMessages, string type_attr_resp) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << type_attr_resp << "\", ";
        ss << "\"err_mess\": [";
        bool nfst = false;
        for (auto& err : errorMessages) {
            if (nfst)
                ss << ", ";
            else
                nfst = true;
            ss << "\"" << err << "\"";
        }
        ss << "]";
        ss << "}";

        return ss.str();
    }

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

    string StringProc::generateExceptionMess(string id, string commandOrPipeName, string& inMessage, string type_req_str) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << type_req_str << "\", ";
        if (commandOrPipeName != NONE)
            ss << "\"" << "name_req" << "\": \"" << commandOrPipeName << "\", ";

        try {
            auto idTmp = stoi(id);
            ss << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            if (id == NONE)
                ss << "\"id_req\": " << id << ", ";
            else
                ss << "\"id_req\": \"" << id << "\", ";
        }
        ss << inMessage;
        //ss << "\"err_mess\": \"" << inMessage << "\"";
        ss << "}";

        return ss.str();
    }

    std::string StringProc::generateRespMess(std::string id, std::string &inMessage, std::string type_req_str)
    {
        stringstream ss;
        ss << "{";
        ss << "\"event\":\"read\", ";
        ss << "\"type_req\": \"" << type_req_str << "\", ";
        try {
            auto idTmp = stoi(id);
            ss << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            if (id == NONE)
                ss << "\"id_req\": " << id << ", ";
            else
                ss << "\"id_req\": \"" << id << "\", ";
        }
        ss << "\"resp\": " << inMessage;
        ss << "}";
        return ss.str();
    }
}
