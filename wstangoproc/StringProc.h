#ifndef STRING_PROC_H
#define STRING_PROC_H

#include <map>
#include <vector>

//#include "ParsingInputJson.h"

using std::string;
using std::vector;
using std::map;
using std::pair;


namespace WebSocketDS_ns
{
    class StringProc {
    public:
        static string responseStringOut(string id, string message, string type_req_str, bool isString = true);
        static string responseStringOut(string id, vector<string> messages, string type_req_str);

        static string exceptionStringOut(string id, string commandOrPipeName, string errorMessage, string type_req_str);
        static string exceptionStringOut(string id, string commandOrPipeName, vector<string> errorMessages, string type_req_str);
        static string exceptionStringOut(string id, string commandOrPipeName, pair<string, string> errorMessages, string type_req_str);

        static string exceptionStringOutForEvent(string id, vector<pair<string, vector<string>>>& devNamesAndExc, string type_req_str, string errorType);
        static string exceptionStringOutForEvent(string id, vector<string> devices, string type_req_str, string errorType);
        // exception for attribute
        static string exceptionStringOut(string errorMessage);
        static string exceptionStringOut(string errorMessage, string type_attr_resp);
        static string exceptionStringOut(vector<string> errorMessages, string type_attr_resp);
        // parse
        static vector<string> parseInputString(string &inp, string delimiter, bool isAllParts = false);
        static bool isNameAlias(const string& deviceName);
        static void removeSymbolsForString(string &str);
        static std::pair<string, string> splitDeviceName(const string& deviceName);
        //static map<std::string, std::string> parseJsonFromCommand(const string& jsonInput, bool isGroup);
        //static ParsedInputJson parseInputJson(const string& json);
    private:
        static string generateExceptionMess(const std::string &id, const std::string &commandOrPipeName, const string& inMessage, const string& type_req_str);
        static string generateRespMess(string id, string& inMessage, string type_req_str);
    };
}
#endif
