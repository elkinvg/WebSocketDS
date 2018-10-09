#include "ParsingInputJson.h"

#include <boost/property_tree/json_parser.hpp>
#include <tuple>

//using std::move;

namespace WebSocketDS_ns
{
    ParsedInputJson ParsingInputJson::parseInputJson(const string &json)
    {
        // Проверка входных парамеров JSON.

        ParsedInputJson parsedJson;
        stringstream ss;
        ss << json;
        boost::property_tree::ptree pt;

        json_arr_map inpVec;
        json_val_map inpStr;
        json_obj_map inpObj;

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
                        else{
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

            if (inpStr.size()) {
                parsedJson.otherInpStr = move(inpStr);
            }
            if (inpVec.size())
                parsedJson.otherInpVec = move(inpVec);
            if (inpObj.size())
                parsedJson.otherInpObj = move(inpObj);

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

    dev_attr_pipe_map ParsingInputJson::getListDevicesAttrPipe(const json_obj_map& objMap, string& errorMessage, bool isAlias)
    {
        //{
        //    "devices": {
        //        "tango/dev/name": {
        //            "attr": "atr_name",
        //            "pipe" : ["pipe_name", "pipe_name2"]
        //        },
        //        "tango/dev/name2" : {
        //            "attr": ["atr_name1", "atr_name2"]
        //            "pipe" : "pipe_name"
        //        }
        //    }
        //}
        // input JSON is value for key "devices"
        // Проверяется наличия хотя бы одного из ключей, attr или pipe
        // Если один из ключей найден значения добавляются в map
        // Проверка наличия ключа devices производится ранее
        dev_attr_pipe_map attrPipeMap; 
        ptree devices;

        if (isAlias)
            devices = objMap.at("devices_al");
        else 
            devices = objMap.at("devices");

        for (auto &dev : devices) {
            auto attr_pipe = make_pair(
                getArrayOfStr("attr", dev.second),
                getArrayOfStr("pipe", dev.second)
                );
            if (attr_pipe.first.size() || attr_pipe.second.size())
                attrPipeMap[dev.first] = move(attr_pipe);

        }
        return attrPipeMap;
    }

    dev_attr_pipe_map ParsingInputJson::getListDevicesAttrPipe(const ptree &devices)
    {
        dev_attr_pipe_map attrPipeMap;

        for (auto &dev : devices) {
            auto attr_pipe = make_pair(
                getArrayOfStr("attr", dev.second),
                getArrayOfStr("pipe", dev.second)
                );
            if (attr_pipe.first.size() || attr_pipe.second.size()) {
                attrPipeMap[dev.first] = move(attr_pipe);
            }
        }
        return attrPipeMap;
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

    void ParsingInputJson::getEventDevInp(const ptree &devices, vec_event_inf& vecEventInf, string event_type)
    {
        try {
            for (const auto& elem : devices) {
                if (!elem.first.size())
                    continue;
                string devName = elem.first;
                
                // if array
                if (elem.second.size()) {
                    vector<string> attrs = getArrayOfStr(devName, devices);
                    for (auto& _attr : attrs) {
                        vecEventInf.push_back(std::make_tuple(devName, _attr, event_type));
                    }
                }
                // if value
                if (elem.second.data().size()) {
                    vecEventInf.push_back(std::make_tuple(devName, elem.second.data(), event_type));
                }
            }
        }
        catch (...) {}
    }

}
