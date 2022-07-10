#include "MyAttributeData.h"

namespace WebSocketDS_ns
{
    MyAttributeData::MyAttributeData(Tango::DeviceAttribute* devAttr)
    {
        extract(devAttr);
    }


    void MyAttributeData::extract(Tango::DeviceAttribute * devAttr)
    {
        type = devAttr->data_type;
        format = devAttr->data_format;
        attrName = devAttr->name;
        quality = attrQuality[devAttr->quality];

        if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
            dim_x = devAttr->dim_x;
        if (format == Tango::AttrDataFormat::IMAGE)
            dim_y = devAttr->dim_y;

        switch (type) {
        case Tango::DEV_BOOLEAN:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevBoolean, dataDevBooleanSet);
            }
            else {
                extractTempl(devAttr, dataDevBooleanArr, dataDevBooleanSet);
            }
        }
        break;
        case Tango::DEV_SHORT:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevShort, dataDevShortSet);
            }
            else {
                extractTempl(devAttr, dataDevShortArr, dataDevShortSet);
            }
        }
        break;
        case Tango::DEV_LONG:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevLong, dataDevLongSet);
            }
            else {
                extractTempl(devAttr, dataDevLongArr, dataDevLongSet);
            }
        }
        break;
        case Tango::DEV_LONG64:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevLong64, dataDevLong64Set);
            }
            else {
                extractTempl(devAttr, dataDevLong64Arr, dataDevLong64Set);
            }
        }
        break;
        case Tango::DEV_FLOAT:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevFloat, dataDevFloatSet);
            }
            else {
                extractTempl(devAttr, dataDevFloatArr, dataDevFloatSet);
            }
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevDouble, dataDevDoubleSet);
            }
            else {
                extractTempl(devAttr, dataDevDoubleArr, dataDevDoubleSet);
            }
        }
        break;
        case Tango::DEV_UCHAR:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevUChar, dataDevUCharSet);
            }
            else {
                extractTempl(devAttr, dataDevUCharArr, dataDevUCharSet);
            }
        }
        break;
        case Tango::DEV_USHORT:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevUShort, dataDevUShortSet);
            }
            else {
                extractTempl(devAttr, dataDevUShortArr, dataDevUShortSet);
            }
        }
        break;
        case Tango::DEV_ULONG:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevULong, dataDevULongSet);
            }
            else {
                extractTempl(devAttr, dataDevULongArr, dataDevULongSet);
            }
        }
        break;
        case Tango::DEV_ULONG64:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevULong64, dataDevULong64Set);
            }
            else {
                extractTempl(devAttr, dataDevULong64Arr, dataDevULong64Set);
            }
        }
        break;
        case Tango::DEV_STRING:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataString, dataStringSet);
            }
            else {
                extractTempl(devAttr, dataStringArr, dataStringSet);
            }
        }
        break;
        case Tango::DEV_STATE:
        {
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevState, dataDevStateSet);
            }
            else {
                extractTempl(devAttr, dataDevStateArr, dataDevStateSet);
            }
        }
        break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            // DevEnum is typedef DevShort	DevEnum;
            if (format == Tango::AttrDataFormat::SCALAR) {
                extractTempl(devAttr, dataDevShort, dataDevShortSet);
            }
            else {
                extractTempl(devAttr, dataDevShortArr, dataDevShortSet);
            }
        }
        break;
#endif
        case Tango::DEV_ENCODED:
        {
        }
        break;
        default:
        {
        }
        break;
        }
    }

    const string MyAttributeData::attrQuality[] = {
       "VALID",     // Tango::AttrQuality::ATTR_VALID
       "INVALID",   // Tango::AttrQuality::ATTR_INVALID
       "ALARM",     // Tango::AttrQuality::ATTR_ALARM
       "CHANGING",  // Tango::AttrQuality::ATTR_CHANGING
       "WARNING"    // Tango::AttrQuality::ATTR_WARNING
    };

    void MyAttributeData::getData(Tango::DevBoolean & data, vector<Tango::DevBoolean>& dataSet)
    {
        data = dataDevBoolean;
        dataSet = dataDevBooleanSet;
    }

    void MyAttributeData::getData(Tango::DevShort & data, vector<Tango::DevShort>& dataSet)
    {
        data = dataDevShort;
        dataSet = dataDevShortSet;
    }

    void MyAttributeData::getData(Tango::DevLong & data, vector<Tango::DevLong>& dataSet)
    {
        data = dataDevLong;
        dataSet = dataDevLongSet;
    }

    void MyAttributeData::getData(Tango::DevLong64 & data, vector<Tango::DevLong64>& dataSet)
    {
        data = dataDevLong64;
        dataSet = dataDevLong64Set;
    }

    void MyAttributeData::getData(Tango::DevFloat & data, vector<Tango::DevFloat>& dataSet)
    {
        data = dataDevFloat;
        dataSet = dataDevFloatSet;
    }

    void MyAttributeData::getData(Tango::DevDouble & data, vector<Tango::DevDouble>& dataSet)
    {
        data = dataDevDouble;
        dataSet = dataDevDoubleSet;
    }

    void MyAttributeData::getData(Tango::DevUChar & data, vector<Tango::DevUChar>& dataSet)
    {
        data = dataDevUChar;
        dataSet = dataDevUCharSet;
    }

    void MyAttributeData::getData(Tango::DevUChar & data)
    {
        data = dataDevUChar;
    }

    void MyAttributeData::getData(Tango::DevUShort & data, vector<Tango::DevUShort>& dataSet)
    {
        data = dataDevUShort;
        dataSet = dataDevUShortSet;
    }

    void MyAttributeData::getData(Tango::DevULong & data, vector<Tango::DevULong>& dataSet)
    {
        data = dataDevULong;
        dataSet = dataDevULongSet;
    }

    void MyAttributeData::getData(Tango::DevULong64 & data, vector<Tango::DevULong64>& dataSet)
    {
        data = dataDevULong64;
        dataSet = dataDevULong64Set;
    }

    void MyAttributeData::getData(string & data, vector<string>& dataSet)
    {
        data = dataString;
        dataSet = dataStringSet;
    }

    void MyAttributeData::getData(string & data)
    {
        data = dataString;
    }

    void MyAttributeData::getData(Tango::DevState & data, vector<Tango::DevState>& dataSet)
    {
        data = dataDevState;
        dataSet = dataDevStateSet;
    }

    void MyAttributeData::getData(Tango::DevState & data)
    {
        data = dataDevState;
    }

    void MyAttributeData::getData(vector<Tango::DevBoolean>& data, vector<Tango::DevBoolean>& dataSet)
    {
        data = dataDevBooleanArr;
        dataSet = dataDevBooleanSet;
    }

    void MyAttributeData::getData(vector<Tango::DevShort>& data, vector<Tango::DevShort>& dataSet) {
        data = dataDevShortArr;
        dataSet = dataDevShortSet;
    }

    void MyAttributeData::getData(vector<Tango::DevLong>& data, vector<Tango::DevLong>& dataSet)
    {
        data = dataDevLongArr;
        dataSet = dataDevLongSet;
    }

    void MyAttributeData::getData(vector<Tango::DevLong64>& data, vector<Tango::DevLong64>& dataSet)
    {
        data = dataDevLong64Arr;
        dataSet = dataDevLong64Set;
    }

    void MyAttributeData::getData(vector<Tango::DevFloat>& data, vector<Tango::DevFloat>& dataSet)
    {
        data = dataDevFloatArr;
        dataSet = dataDevFloatSet;
    }

    void MyAttributeData::getData(vector<Tango::DevDouble>& data, vector<Tango::DevDouble>& dataSet)
    {
        data = dataDevDoubleArr;
        dataSet = dataDevDoubleSet;
    }

    void MyAttributeData::getData(vector<Tango::DevUChar>& data, vector<Tango::DevUChar>& dataSet)
    {
        data = dataDevUCharArr;
        dataSet = dataDevUCharSet;
    }

    void MyAttributeData::getData(vector<Tango::DevUChar>& data)
    {
        data = dataDevUCharArr;
    }

    void MyAttributeData::getData(vector<Tango::DevUShort>& data, vector<Tango::DevUShort>& dataSet)
    {
        data = dataDevUShortArr;
        dataSet = dataDevUShortSet;
    }

    void MyAttributeData::getData(vector<Tango::DevULong>& data, vector<Tango::DevULong>& dataSet)
    {
        data = dataDevULongArr;
        dataSet = dataDevULongSet;
    }

    void MyAttributeData::getData(vector<Tango::DevULong64>& data, vector<Tango::DevULong64>& dataSet)
    {
        data = dataDevULong64Arr;
        dataSet = dataDevULong64Set;
    }

    void MyAttributeData::getData(vector<string>& data, vector<string>& dataSet)
    {
        data = dataStringArr;
        dataSet = dataStringSet;
    }

    void MyAttributeData::getData(vector<Tango::DevState>& data, vector<Tango::DevState>& dataSet)
    {
        data = dataDevStateArr;
        dataSet = dataDevStateSet;
    }

}
