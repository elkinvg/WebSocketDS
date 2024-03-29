#include "EventProcSer.h"

#include "WSThread.h"
#include "StringProc.h"
#include "WsEvCallBackSer.h"
#include "ResponseFromEvent.h"

#include "ErrorInfo.h"

namespace WebSocketDS_ns
{
    EventProcSer::EventProcSer(WSThread * wsThread, DeviceForWs* device, const std::array<std::vector<std::string>, 4>& event_subcr):_wsThread(wsThread), _device(device)
    {
        eventSubscrs = initSubscr(device, event_subcr);
    }

    EventProcSer::EventProcSer(WSThread * wsThread, GroupForWs * group, const std::array<std::vector<std::string>, 4>& event_subcr):_wsThread(wsThread), _group(group)
    {
        for (auto& dev : group->getDeviceList()) {
            try {
                auto dp = group->get_device(dev);
                auto ev = initSubscr(dp, event_subcr);
                if (ev.size()) {
                    eventSubscrsGr[dev] = ev;
                }
            } catch (...) {}
        }
    }

    log4tango::Logger * EventProcSer::get_logger(void)
    {
        return _wsThread->get_logger();
    }

    std::vector<pair<int, WsEvCallBackSer*>> EventProcSer::initSubscr(Tango::DeviceProxy * device, const std::array<std::vector<std::string>, 4>& event_subcr)
    {
        std::vector<pair<int, WsEvCallBackSer*>> _eventSubscrs;
        bool isExcFromSubscr = false;
        vector<ResponseFromEventReq> exceptions;

        for (int i = 0; i < event_subcr.size(); i++) {
            auto evs_vec = event_subcr[i];
            if (!evs_vec.size()) continue;
            Tango::EventType et = eventTypes[i];

            for (auto& attr : evs_vec) {
                if (!attr.size()) continue;
                try {
                    vector<string> gotOptions = StringProc::parseInputString(attr, ";");
                    string optStr;
                    for (auto &opt : gotOptions) {
                        auto iterator_attr = opt.find(OPT_PREC);
                        if (iterator_attr != string::npos) {
                            optStr = opt;
                            break;
                        }
                    }

                    UserOptions uo;
                    uo.isJsonString = false;
                    uo.precision = optStr;

                    WsEvCallBackSer* wsCallBack = new WsEvCallBackSer(_wsThread, uo);

                    int ev = device->subscribe_event(attr, et, wsCallBack);
                    _eventSubscrs.push_back(make_pair(
                        ev,
                        wsCallBack
                    ));
                    DEBUG_STREAM << "subscribe_event with id :" << ev << " device: " << device->name();
                    //
                }
                catch (Tango::DevFailed &e) {
                    vector<string> errs;
                    for (int i = 0; i < e.errors.length(); i++) {
                        errs.push_back((string)e.errors[i].desc);
                    }

                    try {
                        ErrorInfo err;
                        err.typeofError = ERROR_TYPE::FROM_EVENT_SUBSCR;
                        err.device_name = device->dev_name();
                        err.attr_name = attr;
                        err.errorMessages = errs;
                        // DONE:
                        // Сообщение отправляется отдельно для каждой ошибочной подписки
                        // Раньше отправлялось вместе с ответами об успешной подписке
                        string errMess = StringProc::exceptionStringOut(err);
                        _wsThread->send_all(errMess);
                    } catch(...){}
                }
            }
        }

        return _eventSubscrs;
    }

    const Tango::EventType EventProcSer::eventTypes[] = {
            Tango::EventType::CHANGE_EVENT,
            Tango::EventType::PERIODIC_EVENT,
            Tango::EventType::USER_EVENT,
            Tango::EventType::ARCHIVE_EVENT
    };

    EventProcSer::~EventProcSer() {
        if (_device) {
            for (auto& sub : eventSubscrs) {
                try {
                    _device->unsubscribe_event(sub.first);
                    DEBUG_STREAM << "unsubscribe_event with id :" << sub.first;
                }
                catch (...) {}
                delete sub.second;
            }
        }

        if (_group) {
            for (auto& _eventSubscrs : eventSubscrsGr) {
                for (auto& sub : _eventSubscrs.second) {
                    try {
                        auto tmpDev = _group->get_device(_eventSubscrs.first);
                        tmpDev->unsubscribe_event(sub.first);
                        DEBUG_STREAM << "unsubscribe_event with id :" << sub.first << " device: " << _eventSubscrs.first;
                    }
                    catch (...) {}
                    delete sub.second;
                }
            }
            eventSubscrsGr.clear();
        }
    }
}
