#ifndef EVENT_REQ_EXCEPTION
#define EVENT_REQ_EXCEPTION

#include <string>

using std::string;

namespace WebSocketDS_ns
{
    class EventReqException {
    public:
        EventReqException(string& messFromErrorResponses, string&  messFromSuccResponses) : _messFromErrorResponses(messFromErrorResponses), _messFromSuccResponses(messFromSuccResponses) {};

        string getMessFromErrorResponses() {
            return _messFromErrorResponses;
        }
        string getMessFromSuccResponses() {
            return _messFromSuccResponses;
        }
    private:
        string _messFromErrorResponses;
        string _messFromSuccResponses;
    };
}
#endif
