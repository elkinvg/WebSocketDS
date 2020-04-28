#include "GroupOrDeviceForWs.h"

#include "StringProc.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    GroupOrDeviceForWs::GroupOrDeviceForWs()
    {
    }

    GroupOrDeviceForWs::~GroupOrDeviceForWs() {}

    bool GroupOrDeviceForWs::hasPipe()
    {
        return _pipeName.size();
    }

    std::unordered_map<std::string, std::string> GroupOrDeviceForWs::getPrecisionOptionsForAttrs()
    {
        return _precisionOptsOfAttrs;
    }

    std::unordered_map<std::string, std::string> GroupOrDeviceForWs::getPrecisionOptionsForAttrsFromPipes()
    {
        return _precisionOptsOfPipe;
    }

    vector<string> GroupOrDeviceForWs::getListOfAllAttributes()
    {
        return vector<string>();
    }

    void GroupOrDeviceForWs::initPipe(vector<string>&pipeName)
    {
        if (pipeName.size())
            _pipeName = pipeName[0];
        if (pipeName.size() > 1) {
            for (unsigned int i = 1; i < pipeName.size(); i++) {
                string attrName = pipeName[i];
                StringProc::checkPrecisionOptions(attrName, _precisionOptsOfPipe);
            }
        }
    }

    void GroupOrDeviceForWs::initAttr(vector<string> &attributes)
    {
        // Method gettingAttrUserConf added for Searhing of additional options for attributes
        // Now it is options "prec", "precf", "precs" for precision
        // And "niter"

        auto iterator = std::find(attributes.begin(), attributes.end(), "__all_attrs__");

        if (iterator != attributes.end()) {
            attributes.erase(iterator);

            _ifUsingAllAttrsOpt(attributes);
        }

        for (auto& attr : attributes) {
            if (!attr.size())
                continue;

            StringProc::checkPrecisionOptions(attr, _precisionOptsOfAttrs);

            // TODO: Опции
            //forNiterOpt(attr);

            string tmpAttrName = attr;
            std::transform(tmpAttrName.begin(), tmpAttrName.end(), tmpAttrName.begin(), ::tolower);
            if (tmpAttrName.find("json") != std::string::npos)
                _jsonAttributes.insert(attr);

            if (std::find(_attributes.begin(), _attributes.end(), attr) == _attributes.end()) {
                _attributes.push_back(attr);
            }
        }
    }

    void GroupOrDeviceForWs::_ifUsingAllAttrsOpt(vector<string>& attributes)
    {
        vector<string> all_attributes = getListOfAllAttributes();

        // Если только __all_attrs__ список будет пуст
        // Остальной список, например атрибуты с опциями
        // Далее список читается. Это нужно, если указаны опции для атрибутов
        if (attributes.size()) {
            for (auto& attr : attributes) {
                auto cp_attr = attr;
                auto opts = StringProc::parseInputString(attr, ";", true);
                if (opts.size() == 1) {
                    continue;
                }
                auto iterator_attr = std::find(all_attributes.begin(), all_attributes.end(), opts[0]);
                (*iterator_attr) = cp_attr;
            }
        }

        attributes = all_attributes;
    }
}
