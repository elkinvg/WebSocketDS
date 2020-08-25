#ifndef STRING_PROC_H
#define STRING_PROC_H

#include <unordered_map>
#include <vector>

#include "ErrorType.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;


namespace WebSocketDS_ns
{
    struct ParsedInputJson;
    struct ResponseFromEventReq;

    class StringProc {
    public:
        // TODO: const & in arguments
        static string successRespOut(const ParsedInputJson & parsedInput);
        static string responseStringOut(const string& id, string& message, const string& type_req_str, bool isString = true);
        static string responseStringOut(const string& id, const vector<std::string>& messages, const string& type_req_str);
        static string responseStringOutForEventSub(const string& id, const string& type_req_str, const vector<ResponseFromEventReq>& successResponses, const vector<ResponseFromEventReq>& errorResponses);

        // TODO: Привести сообщения об ошибке к единому формату для всех типов
        static string exceptionStringOut(ERROR_TYPE errType, const string& id, const string& errorMessage, const string& type_req_str);
        static string exceptionStringOut(ERROR_TYPE errType, const string& id, const vector<std::string>& errorMessages, const string& type_req_str);

        static string exceptionStringOutForEvent(ERROR_TYPE errType, const vector<ResponseFromEventReq>& errorResponses);

        static string exceptionStringOutForEvent(ERROR_TYPE errType, const vector<ResponseFromEventReq>& errorResponses, const string& id, const string& type_req_str);

        // exception for attribute
        static string exceptionStringOut(ERROR_TYPE errType, const string& errorMessage);
        static string exceptionStringOut(ERROR_TYPE errType, const string& errorMessage, const string& type_attr_resp);
        static string exceptionStringOut(ERROR_TYPE errType, const vector<string>& errorMessages, const string& type_attr_resp);
        // parse
        static vector<string> parseInputString(string &inp, string delimiter, bool isAllParts = false);
        static bool isNameAlias(const string& deviceName);
        static void removeSymbolsForString(string &str);
        static std::pair<string, string> splitDeviceName(const string& deviceName);
        static unordered_map<string, string> parseOfGetQuery(const string& query);
        static string parseOfAddress(const string& addrFromConn);
        static vector<string> &split(const string &s, char delim, vector<string> &elems);
        static vector<string> split(const string &s, char delim);

        static string checkPrecisionOptions(string& reqName, const ParsedInputJson & parsedInput);
        static std::unordered_map<std::string, std::string> checkPrecisionOptions(vector<string>& reqName, const ParsedInputJson & parsedInput);
        static void checkPrecisionOptions(string & attrName, std::unordered_map<std::string, std::string>& _precisionOpts);
        static std::unordered_map<std::string, std::string> checkPrecisionOptions(const ParsedInputJson & parsedInput);

    private:
        static string generateExceptionMess(ERROR_TYPE errType, const std::string &id, const string& inMessage, const string& type_req_str);
        static string generateExceptionMess(ERROR_TYPE errType, const string& inMessage, const string& type_req_str);

        static string generateRespMess(const string& id, const string& inMessage, const string& type_req_str);
        static string generateRespMess(const string& id, const string& inMessage, const string& errMessage, const string& type_req_str);

        static string generateExceptionStringOutForEvent(const vector<ResponseFromEventReq>& errorResponses);
        static string generateSuccessStringOutForEvent(const vector<ResponseFromEventReq>& successResponses);
    };
}
#endif
