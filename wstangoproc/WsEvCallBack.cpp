#include "WsEvCallBack.h"
#include "TangoProcessor.h"
#include "StringProc.h"

namespace WebSocketDS_ns
{
    WsEvCallBack::WsEvCallBack(websocketpp::connection_hdl hdl, WSThread* wsThread, string &attr_name)
    {
        _hdl = hdl;
        _wsThread = wsThread;
        auto options = StringProc::parseInputString(attr_name, ";");
        tango_proc = new TangoProcessor();
        tango_proc->initOptionsForAttrOrComm(attr_name, options, TYPE_WS_REQ::ATTRIBUTE);
    }
    
    WsEvCallBack::~WsEvCallBack()
    {
        delete tango_proc;
    }

    void WsEvCallBack::push_event(Tango::EventData *dt)
    {
        try {
            if (dt->err) {
                vector<string> errors;
                for (int i = 0; i < dt->errors.length(); i++) {
                    errors.push_back((string)dt->errors[i].desc);
                }
                _wsThread->send(_hdl, StringProc::exceptionStringOut(errors,"from_event"));
                return;
            }
            stringstream json;
            json << "{\"event\": \"read\", ";
            json << "\"type_req\" : \"from_event\", ";
            json << "\"type_event\": \"" << dt->event << "\", ";
            json << "\"timestamp\": " << dt->get_date().tv_sec << ", ";
            json << "\"attr\": \"" << dt->attr_name << "\", ";
            tango_proc->devAttrToStr(dt->attr_value, json);
            json << "}";
            _wsThread->send(_hdl, json.str());
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            _wsThread->send(_hdl, StringProc::exceptionStringOut(errors, "exc_from_event"));
        }

        catch (...) {
            _wsThread->send(_hdl, StringProc::exceptionStringOut("Unknown exception from event", "exc_from_event"));
        }
    }
}
