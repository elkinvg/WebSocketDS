#ifndef MY_ATTRIBUTE_DATA_H
#define MY_ATTRIBUTE_DATA_H

#include <string>
#include <vector>
#include <tango.h>

using std::string;
using std::vector;

namespace WebSocketDS_ns
{
    class MyAttributeData {
    public:
        MyAttributeData(Tango::DeviceAttribute* devAttr);
        string attrName;
        int dim_x;
        int dim_y;
        Tango::AttrDataFormat format;
        std::string quality;
        int type;

        void getData(Tango::DevBoolean& data, vector<Tango::DevBoolean>& dataSet);
        void getData(Tango::DevShort& data, vector<Tango::DevShort>& dataSet);
        void getData(Tango::DevLong& data, vector<Tango::DevLong>& dataSet);
        void getData(Tango::DevLong64& data, vector<Tango::DevLong64>& dataSet);
        void getData(Tango::DevFloat& data, vector<Tango::DevFloat>& dataSet);
        void getData(Tango::DevDouble& data, vector<Tango::DevDouble>& dataSet);
        void getData(Tango::DevUChar& data, vector<Tango::DevUChar>& dataSet);
        void getData(Tango::DevUChar& data);
        void getData(Tango::DevUShort& data, vector<Tango::DevUShort>& dataSet);
        void getData(Tango::DevULong& data, vector<Tango::DevULong>& dataSet);
        void getData(Tango::DevULong64& data, vector<Tango::DevULong64>& dataSet);
        void getData(string& data, vector<string>& dataSet);
        void getData(string& data);
        void getData(Tango::DevState& data, vector<Tango::DevState>& dataSet);
        void getData(Tango::DevState& data);


        void getData(vector<Tango::DevBoolean>& data, vector<Tango::DevBoolean>& dataSet);
        void getData(vector<Tango::DevShort>& data, vector<Tango::DevShort>& dataSet);
        void getData(vector<Tango::DevLong>& data, vector<Tango::DevLong>& dataSet);
        void getData(vector<Tango::DevLong64>& data, vector<Tango::DevLong64>& dataSet);
        void getData(vector<Tango::DevFloat>& data, vector<Tango::DevFloat>& dataSet);
        void getData(vector<Tango::DevDouble>& data, vector<Tango::DevDouble>& dataSet);
        void getData(vector<Tango::DevUChar>& data, vector<Tango::DevUChar>& dataSet);
        void getData(vector<Tango::DevUChar>& data);
        void getData(vector<Tango::DevUShort>& data, vector<Tango::DevUShort>& dataSet);
        void getData(vector<Tango::DevULong>& data, vector<Tango::DevULong>& dataSet);
        void getData(vector<Tango::DevULong64>& data, vector<Tango::DevULong64>& dataSet);
        void getData(vector<string>& data, vector<string>& dataSet);
        void getData(vector<Tango::DevState>& data, vector<Tango::DevState>& dataSet);

    private:
        void extract(Tango::DeviceAttribute* devAttr);

        template <typename T>
        void extractTempl(Tango::DeviceAttribute* devAttr, T& data, vector<T>& dataSet) {
            (*devAttr) >> data;

            try {
                devAttr->extract_set(dataSet);
            }
            catch (...) {}
        }

        template <typename T>
        void extractTempl(Tango::DeviceAttribute* devAttr, vector<T>& data, vector<T>& dataSet) {
            (*devAttr) >> data;

            try {
                devAttr->extract_set(dataSet);
            }
            catch (...) {}
        }

    private:
        // SCALAR
        Tango::DevBoolean dataDevBoolean;
        Tango::DevShort dataDevShort;
        Tango::DevLong dataDevLong;
        Tango::DevLong64 dataDevLong64;
        Tango::DevFloat dataDevFloat;
        Tango::DevDouble dataDevDouble;
        Tango::DevUChar dataDevUChar;
        Tango::DevUShort dataDevUShort;
        Tango::DevULong dataDevULong;
        Tango::DevULong64 dataDevULong64;
        std::string dataString;
        Tango::DevState dataDevState;

        // SET
        vector<Tango::DevBoolean> dataDevBooleanSet;
        vector<Tango::DevShort> dataDevShortSet;
        vector<Tango::DevLong> dataDevLongSet;
        vector<Tango::DevLong64> dataDevLong64Set;
        vector<Tango::DevFloat> dataDevFloatSet;
        vector<Tango::DevDouble> dataDevDoubleSet;
        vector<Tango::DevUChar> dataDevUCharSet;
        vector<Tango::DevUShort> dataDevUShortSet;
        vector<Tango::DevULong> dataDevULongSet;
        vector<Tango::DevULong64> dataDevULong64Set;
        vector<std::string> dataStringSet;
        vector<Tango::DevState> dataDevStateSet;

        // SPECTRUM OR IMAGE
        vector<Tango::DevBoolean> dataDevBooleanArr;
        vector<Tango::DevShort> dataDevShortArr;
        vector<Tango::DevLong> dataDevLongArr;
        vector<Tango::DevLong64> dataDevLong64Arr;
        vector<Tango::DevFloat> dataDevFloatArr;
        vector<Tango::DevDouble> dataDevDoubleArr;
        vector<Tango::DevUChar> dataDevUCharArr;
        vector<Tango::DevUShort> dataDevUShortArr;
        vector<Tango::DevULong> dataDevULongArr;
        vector<Tango::DevULong64> dataDevULong64Arr;
        vector<std::string> dataStringArr;
        vector<Tango::DevState> dataDevStateArr;

        static const string attrQuality[];
    };
}
#endif
