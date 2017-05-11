#ifndef STRING_PROC_H
#define STRING_PROC_H

#include <map>
#include <vector>

#include "ParsingInputJson.h"

using std::string;
using std::vector;
using std::map;
using std::pair;


namespace WebSocketDS_ns
{
    class StringProc {
    public:
        static string responseStringOut(string id, string message, string type_req_str);
        static string responseStringOut(string id, vector<string> messages, string type_req_str);

        static string exceptionStringOut(string id, string commandOrPipeName, string errorMessage, string type_req_str);
        static string exceptionStringOut(string id, string commandOrPipeName, vector<string> errorMessages, string type_req_str);
        static string exceptionStringOut(string id, string commandOrPipeName, pair<string, string> errorMessages, string type_req_str);
        // exception for attribute
        static string exceptionStringOut(string errorMessage);
        static string exceptionStringOut(string errorMessage, string type_attr_resp);
        // parse
        static vector<string> parseInputString(string &inp, string delimiter, bool isAllParts = false);
        static bool isNameAlias(const string& deviceName);
        //static map<std::string, std::string> parseJsonFromCommand(const string& jsonInput, bool isGroup);
        //static ParsedInputJson parseInputJson(const string& json);
    private:
        static string generateExceptionMess(string id, string commandOrPipeName, string& inMessage, string type_req_str);
        static string generateRespMess(string id, string& inMessage, string type_req_str);
    };
}
#endif
