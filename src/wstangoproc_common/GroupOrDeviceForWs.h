#ifndef GroupOrDeviceForWs_H
#define GroupOrDeviceForWs_H

#include <tango.h>
#include <unordered_set>
#include <unordered_map>

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::string;

namespace WebSocketDS_ns
{

    class GroupOrDeviceForWs
    {
    public:
        GroupOrDeviceForWs();
        ~GroupOrDeviceForWs();
        bool hasPipe();
        unordered_map<string, string> getPrecisionOptionsForAttrs();
        unordered_map<string, string> getPrecisionOptionsForAttrsFromPipes();

    protected:
        vector<string> getListOfAllAttributes();
        void initPipe(vector<string> &pipeName);
        void initAttr(vector<string> &attributes);

    private:
        void _ifUsingAllAttrsOpt(vector<string>& attributes);

    protected:
        vector<string> _attributes;
        string _pipeName;
        unordered_set<string> _jsonAttributes;
        unordered_map<string, string> _precisionOptsOfAttrs;
        unordered_map<string, string> _precisionOptsOfPipe;
    };
}

#endif // GroupOrDeviceForWs_bck_H
