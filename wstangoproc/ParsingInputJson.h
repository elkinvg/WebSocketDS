#ifndef PARSINGINPUTJSON_H
#define PARSINGINPUTJSON_H

#include "common.h"
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

using std::string;
using std::stringstream;
using std::vector;
using std::unordered_map;
using std::pair;

typedef boost::property_tree::ptree ptree;
typedef unordered_map<string, vector<string>> json_arr_map;
typedef unordered_map<string, string> json_val_map;
typedef unordered_map<string, ptree> json_obj_map;
typedef unordered_map< string, pair<vector<string>, vector<string>>> dev_attr_pipe_map;


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

        bool isOk{ false };
        string errMess;
        string id;
        string type_req;
        string inputJson;
        json_arr_map otherInpVec;
        json_val_map otherInpStr;
        json_obj_map otherInpObj;
    };

    class ParsingInputJson {
    public:
        ParsingInputJson(){}
        ~ParsingInputJson(){}

        ParsedInputJson parseInputJson(const string& json);
        dev_attr_pipe_map getListDevicesAttrPipe(const json_obj_map& objMap, string& errorMessage, bool isAlias);
        dev_attr_pipe_map getListDevicesAttrPipe(const ptree &devices);
        vector<string> getArrayOfStr(string key, const ptree& inPtree);
    };
}

#endif // PARSINGINPUTJSON_H
