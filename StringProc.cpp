#include "StringProc.h"
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

using std::stringstream;

namespace WebSocketDS_ns
{

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

    string StringProc::exceptionStringOut(string errorMessage) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << "attribute" << "\", ";
        ss << "\"err_mess\": \"" << errorMessage << "\"";
        ss << "}";

        return ss.str();
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

    //std::map<std::string, std::string> StringProc::parseJsonFromCommand(const string& jsonInput, bool isGroup) {
    //    // Проверка входных парамеров JSON.
    //    // Если параметры найдены в строке будет значение параметра, иначе пустая строка
    //    // Для argin, если входной параметр массив передаёт "Array", иначе просто значение
    //    boost::property_tree::ptree pt;
    //    std::stringstream ss;
    //    std::map<std::string, std::string> output;

    //    std::string command, id;

    //    ss << jsonInput;
    //    try {
    //        boost::property_tree::read_json(ss, pt);

    //        //pt.get_value_optional
    //        // Взят boost::optional. В случае, если параметр JSON не найден, присваивает этому параметру пустую строку

    //        // Если входная строка не JSON, возвращяет строку с ошибкой.
    //        vector<pair<std::string, boost::optional<std::string>>> boostOpt;


    //        if (isGroup) {
    //            boostOpt.push_back(std::make_pair("command_group", pt.get_optional<std::string>("command_group")));
    //            boostOpt.push_back(std::make_pair("command_device", pt.get_optional<std::string>("command_device")));
    //            boostOpt.push_back(std::make_pair("device_name", pt.get_optional<std::string>("device_name")));
    //            // for pipe
    //            boostOpt.push_back(std::make_pair("read_pipe_gr", pt.get_optional<std::string>("read_pipe_gr")));
    //            boostOpt.push_back(std::make_pair("read_pipe_dev", pt.get_optional<std::string>("read_pipe_dev")));
    //        }
    //        else {
    //            boostOpt.push_back(std::make_pair("command", pt.get_optional<std::string>("command")));
    //            // for pipe
    //            boostOpt.push_back(std::make_pair("read_pipe", pt.get_optional<std::string>("read_pipe")));
    //        }

    //        boostOpt.push_back(std::make_pair("id", pt.get_optional<std::string>("id")));
    //        boostOpt.push_back(std::make_pair("argin", pt.get_optional<std::string>("argin")));

    //        bool isJsonExact = true;
    //        for (auto& v : boostOpt) {
    //            if (v.second) {
    //                output.insert(std::pair<std::string, std::string>(v.first, v.second.get()));
    //            }
    //            else {
    //                output.insert(std::pair<std::string, std::string>(v.first, NONE));
    //            }
    //        }

    //        // Здесь, если output["argin"].size() == 0, значит параметр argin найден, но формат данных не простое значение.
    //        // Прводится проверка, является ли он массивом

    //        if (output["argin"].size() == 0) {
    //            int it = 0;
    //            try {
    //                for (boost::property_tree::ptree::value_type &v : pt.get_child("argin")) {
    //                    it++;
    //                }
    //            }
    //            catch (boost::property_tree::ptree_bad_data) { it = 0; }
    //            if (it) output["argin"] = "Array";
    //        }

    //    }
    //    catch (boost::property_tree::json_parser::json_parser_error &je)
    //    {
    //        std::string err = "Json parsed error. Message from Boost: " + je.message();
    //        output.insert(std::make_pair("error", err));
    //    }

    //    return output;
    //}

    ParsedInputJson StringProc::parseInputJson(const string& json){
        // Проверка входных парамеров JSON.

        ParsedInputJson parsedJson;
        stringstream ss;
        ss << json;
        boost::property_tree::ptree pt;

        std::unordered_map<string, vector<string>> inpVec;
        std::unordered_map<string, string> inpStr;

        try {
            boost::property_tree::read_json(ss, pt);

            // id должен присутствовать, но не обязательно
            if (pt.find("id") == pt.not_found()) {
                parsedJson.id = NONE;
            }


            for (auto& elem : pt) {
                string tmpStr;
                if (elem.first == "id") {
                    tmpStr = elem.second.data();
                    if (!tmpStr.size()) {
                        parsedJson.errMess = "type of id must be value";
                        return parsedJson;
                    }
                    parsedJson.id = tmpStr;
                    continue;
                }

                // type_req должен присутствовать 
                // (для command можно не указывать ... оставлено для совместимости )
                if (elem.first == "type_req") {
                    tmpStr = elem.second.data();
                    if (!tmpStr.size()) {
                        parsedJson.errMess = "type of type_req must be value";
                        return parsedJson;
                    }
                    parsedJson.type_req = tmpStr;
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
                    }
                }
                catch (boost::property_tree::ptree_bad_data) { it = 0; }
                if (it) {
                    inpVec[elem.first] = vecStr;
                }
            }

            if (inpStr.size()) {
                parsedJson.otherInpStr = inpStr;
            }
            if (inpVec.size())
                parsedJson.otherInpVec = inpVec;
            parsedJson.isOk = true;
        }
        catch (boost::property_tree::json_parser::json_parser_error &je)
        {
            parsedJson.errMess = "Json parsed error. Message from Boost: " + je.message() ;
        }
        catch (...) {
            parsedJson.errMess = "unknown error from parseInputJson ";
        }
        return parsedJson;
    }

    string StringProc::generateExceptionMess(string id, string commandOrPipeName, string& inMessage, string type_req_str) {
        stringstream ss;
        ss << "{\"event\": \"error\", ";
        ss << "\"type_req\": \"" << type_req_str << "\", ";
        if (commandOrPipeName == NONE)
            ss << "\"" << "name_req" << "\": " << commandOrPipeName << ", ";
        else
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
}
