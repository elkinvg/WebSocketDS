#include "WsEvCallBackForServerMode.h"

#include "TangoProcessor.h"
#include "StringProc.h"

namespace WebSocketDS_ns
{
    WsEvCallBackForServerMode::WsEvCallBackForServerMode(WSThread *wsThread, std::string &attr_name)
    {
        _wsThread = wsThread;
        auto options = StringProc::parseInputString(attr_name, ";");
        tango_proc = new TangoProcessor();
        tango_proc->initOptionsForAttrOrComm(attr_name, options, TYPE_WS_REQ::ATTRIBUTE);
    }
    
    WsEvCallBackForServerMode::~WsEvCallBackForServerMode()
    {
        delete tango_proc;
    }
    
    void WsEvCallBackForServerMode::push_event(Tango::EventData *dt)
    {
        try {
            if (dt->err) {
                vector<string> errors;
                for (int i = 0; i < dt->errors.length(); i++) {
                    errors.push_back((string)dt->errors[i].desc);
                }
                send_mess_all(StringProc::exceptionStringOut(errors, "from_event"));
                return;
            }
            stringstream json;
            json << "{\"event\": \"read\", ";
            json << "\"type_req\" : \"from_event\", ";
            json << "\"event_type\": \"" << dt->event << "\", ";
            json << "\"timestamp\": " << dt->get_date().tv_sec << ", ";
            json << "\"attr\": \"" << dt->attr_name << "\", ";
            tango_proc->devAttrToStr(dt->attr_value, json);
            json << "}";
            send_mess_all(json.str());
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            send_mess_all(StringProc::exceptionStringOut(errors, "exc_from_event_dev"));
        }

        catch (...) {
            send_mess_all(StringProc::exceptionStringOut("Unknown exception from event", "exc_from_event_dev"));
        }
        return;
    }

    void WsEvCallBackForServerMode::send_mess_all(const std::string& mess)
    {
        try {
            _wsThread->send_all(mess);
        }
        catch (...) {}
    }

}
