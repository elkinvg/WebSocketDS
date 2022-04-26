#ifndef STRING_PROC_H
#define STRING_PROC_H

#include "CurrentMode.h"
#include <unordered_map>
#include <vector>
#include <iostream>
#include "ErrorType.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;
using std::stringstream;


namespace WebSocketDS_ns
{
    struct ParsedInputJson;
    struct ResponseFromEventReq;
    struct ErrorInfo;

    class StringProc {
    public:
        static string successRespOut(const ParsedInputJson & parsedInput);

        static string responseStringOut(const string& id, string& message, const string& type_req_str, bool isString = true);
        static string responseStringOut(const string& id, const vector<std::string>& messages, const string& type_req_str);

#ifdef CLIENT_MODE
        static string responseStringOutForEventSub(const string& id, const string& type_req_str, const vector<ResponseFromEventReq>& successResponses);
#endif

        static string exceptionStringOut(const ErrorInfo& errorInfo);


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
        // TODO: RENAME
        static void generateErrorMess(stringstream &ss, const string& errorMessage);
        static void generateErrorMess(stringstream &ss, const vector<string>& errorMessages);

        static string generateRespMess(const string& id, const string& inMessage, const string& type_req_str);
        static string generateRespMess(const string& id, const string& inMessage, const string& errMessage, const string& type_req_str);
#ifdef CLIENT_MODE
        static string generateSuccessStringOutForEvent(const vector<ResponseFromEventReq>& successResponses);
#endif
    };
}
#endif
