#ifndef PARSINGINPUTJSON_H
#define PARSINGINPUTJSON_H

#include "common.h"
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include "CurrentMode.h"

using std::string;
using std::stringstream;
using std::vector;
using std::unordered_map;
using std::pair;



namespace WebSocketDS_ns
{
    struct ParsedInputJson {
        TYPE_OF_VAL check_key(string key) const {
            if (otherInpStr.find(key) != otherInpStr.end())
                return TYPE_OF_VAL::VALUE;
            if (otherInpVec.find(key) != otherInpVec.end())
                return TYPE_OF_VAL::ARRAY;
            if (otherInpObj.find(key) != otherInpObj.end())
                return TYPE_OF_VAL::OBJECT;
            else
                return TYPE_OF_VAL::NONE;
        }

        TYPE_OF_VAL check_keys(vector<string> keys) const {
            bool fst = true;
            TYPE_OF_VAL typeOfVal;
            for (auto& key : keys) {
                if (fst) {
                    typeOfVal = check_key(key);
                    if (typeOfVal == TYPE_OF_VAL::NONE)
                        break;
                    fst = false;
                }
                if (check_key(key) != typeOfVal) {
                    typeOfVal = TYPE_OF_VAL::NONE;
                    break;
                }
            }
            return typeOfVal;
        }

        bool isValid{ false };
        string errMess;
        string id;

        string type_req_str;
        TYPE_WS_REQ type_req;
        string inputJson;
        json_arr_map otherInpVec;
        json_val_map otherInpStr;
        json_obj_map otherInpObj;
#ifdef CLIENT_MODE
        unordered_map<string, vector<string>> periodicEvSubList;
        unordered_map<string, vector<string>> changeEvSubList;
        unordered_map<string, vector<string>> userEvSubList;
        unordered_map<string, vector<string>> archiveEvSubList;
#endif
    };

    class ParsingInputJson {
    public:
        ParsingInputJson() = delete;
        ParsingInputJson(ParsingInputJson&) = delete;

        // TODO: Replace in StringProc
        // TODO: add argument isServerMode
        static ParsedInputJson parseInputJson(const string& json);
       
        //void getEventDevInp(const ptree &devices, vec_event_inf& vecEventInf, string event_type);

    private:
        static void _checkValidity(ParsedInputJson &parsedInput);
        static vector<string> getArrayOfStr(string key, const ptree& inPtree);
#ifdef CLIENT_MODE
        static unordered_map<string, vector<string>> _getEventSubList(const ptree& inPtree);
#endif
//    private:
//#ifdef SERVER_MODE
//        bool _isServerMode{ true };
//#else
//        bool _isServerMode{ false };
//#endif // SERVER_MODE
    };
}

#endif // PARSINGINPUTJSON_H
