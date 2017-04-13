#ifndef WSTANGOCONN_H
#define WSTANGOCONN_H

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include <tango.h>

#include "common.h"
#include <array>

using std::array;

namespace WebSocketDS_ns
{
    class WSThread;
    class GroupOrDeviceForWs;
    class WebSocketDS;
    class UserControl;

    class WSTangoConn: public Tango::LogAdapter
    {
    public:
        WSTangoConn(WebSocketDS *dev,pair<string, string> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber);
        
        WSTangoConn(WebSocketDS *dev,pair<string, string> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, string cert, string key);

        ~WSTangoConn();

        string for_update_data();
        string sendRequest(const ParsedInputJson& inputReq, bool& isBinary);

        void setNumOfConnections(unsigned long num) {_numOfConnections = num; }
        unsigned long getNumOfConnections() {return _numOfConnections;}
        TYPE_OF_IDENT getTypeOfIdent() {return typeOfIdent;}
        bool isLogActive() {return _isLogActive; }
        bool isInitDs(string &errorMessage) {errorMessage = this->errorMessage; return _isInitDs; }
        unsigned int getMaxBuffSize();
        unsigned short getMaxNumberOfConnections();
        string getAuthDS();

    private:
        void init_wstc(WebSocketDS *dev, pair<string, string> &dsAndOptions, array<vector<string>, 3> &attrCommPipe);
        void initOptionsAndDeviceServer(pair<string, string>& dsAndOptions);
        bool initDeviceServer();
        string fromException(Tango::DevFailed &e, string func);
        void  removeSymbolsForString(string &str);

        TYPE_WS_REQ getTypeWsReq(const string& req);



    private:
        WSThread *wsThread = nullptr;
        WebSocketDS *_wsds;
        std::unique_ptr<GroupOrDeviceForWs> groupOrDevice;
        std::unique_ptr<UserControl> uc;
        TYPE_OF_IDENT typeOfIdent{ TYPE_OF_IDENT::SIMPLE };
        string _deviceName;
        bool  _isLogActive{ false };
        bool _isInitDs{ false };
        bool _isGroup{ false };
        bool _isShortAttr{ true };
        unsigned long _numOfConnections{0};
        string errorMessage;
    };
}
#endif
