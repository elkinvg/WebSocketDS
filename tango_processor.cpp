#include "tango_processor.h"
#include <iomanip>  
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
//#include <typeinfo>

namespace WebSocketDS_ns
{
	std::string tango_processor::process_attribute_t(Tango::DeviceAttribute att) {
		std::stringstream json;
		json << "";

		Tango::AttrQuality quality = att.get_quality();

		json << "{";
		json << "\"attr\": \"" << att.get_name() << "\", ";
		json << "\"qual\": \"" << SwitchAttrQuality(att.get_quality()) << "\", ";
		json << "\"time\": " << att.time.tv_sec << ", ";

		Tango::AttrDataFormat format = att.get_data_format();
        json << devAttrToStr(&att);
		json << "}";
		return json.str();
	}

	template <typename T>
    std::string  tango_processor::attrsToString(T& data, Tango::DeviceAttribute *attr) {
		Tango::AttrDataFormat format = attr->get_data_format();
		int type = attr->get_type();
		std::vector<T> dataVector;
		Tango::DevState stateIn;
		string stateStr;

		std::stringstream ss;

		if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
			ss << "\"dimX\": " << attr->dim_x << ", ";
		if (format == Tango::AttrDataFormat::IMAGE)
			ss << "\"dimY\": " << attr->dim_y << ", ";

		ss << "\"data\": ";
		if (format == Tango::AttrDataFormat::SCALAR) {
			if (type == Tango::DEV_STATE) {
				(*attr) >> stateIn;
				stateStr = SwitchTangoState(stateIn);
                dataFromAttrsToJson(stateStr, ss);
			}				
			else {
				(*attr) >> data;
                dataFromAttrsToJson(data, ss);
			}				
		}
		else			
		if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
			(*attr) >> dataVector;
			ss << "[";
			bool begin = true;
            for (const auto& fromData : dataVector) {
				if (!begin) ss << ", ";
				else begin = false;
                dataFromAttrsToJson(fromData, ss);
			}
			ss << "]";
		}
		return ss.str();
	}

	template <typename T>
    void tango_processor::dataFromAttrsToJson(T& data, std::stringstream& ss) {
		if (is_floating_point<T>::value) ss << std::setprecision(5) << data;
		else if (std::is_same<T, bool>::value) ss << std::boolalpha << data;
		else if (std::is_same<T, std::string>::value) ss << "\"" << data << "\"";
		else ss << data;
	}

    std::string tango_processor::devAttrToStr(Tango::DeviceAttribute *attr) {
		int type = attr->get_type();
		Tango::AttrDataFormat format = attr->get_data_format();
		std::string out;
		switch (type) {
		case Tango::DEV_BOOLEAN:
		{
			Tango::DevBoolean inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_SHORT:
		{
			Tango::DevShort inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_LONG:
		{
			Tango::DevLong inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_LONG64:
		{
			Tango::DevLong64 inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_FLOAT:
		{
			Tango::DevFloat inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_DOUBLE:
		{
			Tango::DevDouble inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_UCHAR:
		{
			Tango::DevUChar inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_USHORT:
		{
			Tango::DevUShort inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_ULONG:
		{
			Tango::DevULong inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_ULONG64:
		{
			Tango::DevULong64 inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_STRING:
		{
			//Tango::DevString inp;
			std::string inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_STATE:
		{
			Tango::DevState inp;
            out = attrsToString(inp, attr);
		}
		break;
		case Tango::DEV_ENCODED:
			// посмотреть
			break;
		default:
			break;
		}
		return out;
	}

    std::string tango_processor::process_device_data_t(Tango::DeviceData& deviceData) {
        std::stringstream json;
        json << "{";
        json << "\"argout\": ";
        json << "}";
        return json.str();
    }

    std::string tango_processor::devDataToString(Tango::DeviceData* deviceData) {
        int type = deviceData->get_type();
        std::string out;

        switch (type) {
        case Tango::DEV_VOID:
            break;
        case Tango::DEV_BOOLEAN:
            break;
         case Tango::DEV_SHORT:
            Tango::DevShort inp;
            out = dataToString(inp,deviceData);
            break;
        default:
            break;
        }
        return out;
    }

    tango_processor::TYPE_OF_DEVICE_DATA tango_processor::getTypeOfData(int tangoType)
    {
        switch (tangoType)
        {
        case Tango::DEV_VOID:
            return TYPE_OF_DEVICE_DATA::VOID_D;
        case Tango::DEV_BOOLEAN:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_SHORT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_LONG:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_FLOAT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_DOUBLE:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_USHORT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_ULONG:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_STRING:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEVVAR_CHARARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_SHORTARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_LONGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_FLOATARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_DOUBLEARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_USHORTARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_ULONGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_STRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
//        case Tango::DEV_STATE:
//            json << devStateToStr(&data);
//            break;
//        case Tango::CONST_DEV_STRING:
//            json << devConstStringToStr(&data);
//            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEV_UCHAR:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_LONG64:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_ULONG64:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEVVAR_LONG64ARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_ULONG64ARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEV_INT:
            return TYPE_OF_DEVICE_DATA::DATA;
//        case Tango::DEV_ENCODED:
//            json << devEncodedToStr(&data);
//            break;
        default:
            break;
        }
    }

    Tango::DeviceData tango_processor::gettingJsonStrToDevData(string jsonData)
    {
        Tango::DeviceData d;
        boost::property_tree::ptree pt;
        std::stringstream ss;
        std::string command;

        ss << jsonData;
        command = pt.get<std::string>("command");

        boost::property_tree::read_json(ss, pt);
        bool parsed = pt.get<bool>("argin");
        d << parsed;
        return d;
    }

    template <typename T>
    std::string  tango_processor::dataToString(T& data, Tango::DeviceData *devData) {
        int type = devData->get_type();
        std::vector<T> dataVector;

        std::stringstream ss;
        ss << "\"argout\": ";
        if (getTypeOfData(type)==TYPE_OF_DEVICE_DATA::DATA) {
            (*devData) >> data;
            dataFromAttrsToJson(data,ss);
        }
        if (getTypeOfData(type)==TYPE_OF_DEVICE_DATA::ARRAY) {
            (*devData) >> dataVector;
            ss << "[";
            bool begin = true;
            for (const auto& fromData : dataVector) {
                if (!begin) ss << ", ";
                else begin = false;
                dataFromAttrsToJson(fromData, ss);
            }
            ss << "]";
        }
        return ss.str();
    }

	std::string tango_processor::process_device_data(Tango::DeviceData data)
	{
		std::stringstream json;
		json << "";

		json << "{";

        int type = data.get_type();

		switch (type)
		{
		case Tango::DEV_VOID:
			json << devVoidToStr(&data);
			break;
		case Tango::DEV_BOOLEAN:
			json << devBooleanToStr(&data);
			break;
		case Tango::DEV_SHORT:
			json << devShortToStr(&data);
			break;
		case Tango::DEV_LONG:
			json << devLongToStr(&data);
			break;
		case Tango::DEV_FLOAT:
			json << devFloatToStr(&data);
			break;
		case Tango::DEV_DOUBLE:
			json << devDoubleToStr(&data);
			break;
		case Tango::DEV_USHORT:
			json << devUShortToStr(&data);
			break;
		case Tango::DEV_ULONG:
			json << devULongToStr(&data);
			break;
		case Tango::DEV_STRING:
			json << devStringToStr(&data);
			break;
		case Tango::DEVVAR_CHARARRAY:
			json << devCharArrayToStr(&data);
			break;
		case Tango::DEVVAR_SHORTARRAY:
			json << devShortArrayToStr(&data);
			break;
		case Tango::DEVVAR_LONGARRAY:
			json << devLongArrayToStr(&data);
			break;
		case Tango::DEVVAR_FLOATARRAY:
			json << devFloatArrayToStr(&data);
			break;
		case Tango::DEVVAR_DOUBLEARRAY:
			json << devDoubleArrayToStr(&data);
			break;
		case Tango::DEVVAR_USHORTARRAY:
			json << devUShortArrayToStr(&data);
			break;
		case Tango::DEVVAR_ULONGARRAY:
			json << devULongArrayToStr(&data);
			break;
		case Tango::DEVVAR_STRINGARRAY:
			json << devStringArrayToStr(&data);
			break;
		case Tango::DEVVAR_LONGSTRINGARRAY:
			json << devLongStringArrayToStr(&data);
			break;
		case Tango::DEVVAR_DOUBLESTRINGARRAY:
			json << devDoubleStringArrayToStr(&data);
			break;
		case Tango::DEV_STATE:
			json << devStateToStr(&data);
			break;
		case Tango::CONST_DEV_STRING:
			json << devConstStringToStr(&data);
			break;
		case Tango::DEVVAR_BOOLEANARRAY:
			json << devBooleanArrayToStr(&data);
			break;
		case Tango::DEV_UCHAR:
			json << devUCharToStr(&data);
			break;
		case Tango::DEV_LONG64:
			json << devLong64ToStr(&data);
			break;
		case Tango::DEV_ULONG64:
			json << devULong64ToStr(&data);
			break;
		case Tango::DEVVAR_LONG64ARRAY:
			json << devLong64ArrayToStr(&data);
			break;
		case Tango::DEVVAR_ULONG64ARRAY:
			json << devULong64ArrayToStr(&data);
			break;
		case Tango::DEV_INT:
			json << devIntToStr(&data);
			break;
		case Tango::DEV_ENCODED:
			json << devEncodedToStr(&data);
			break;
		default:
			break;
		}
		json << "}";
		return json.str();
	}

	std::string tango_processor::process_device_data_json(Tango::DeviceData data)
	{
		std::string argout;
		(data) >> argout;
		return argout;
	}

	std::string tango_processor::process_device_attribute_json(Tango::DeviceAttribute data)
	{
		std::string argout;
		(data) >> argout;
		return argout;
	}

	Tango::DeviceData tango_processor::process_json(std::string source, int type)
	{
		switch (type)
		{
		case Tango::DEV_VOID:
			return strToDevVoidData(source);
		case Tango::DEV_BOOLEAN:
			return strToDevBooleanData(source);
		case Tango::DEV_SHORT:
			return strToDevShortData(source);
		case Tango::DEV_LONG:
			return strToDevLongData(source);
		case Tango::DEV_FLOAT:
			return strToDevFloatData(source);
		case Tango::DEV_DOUBLE:
			return strToDevDoubleData(source);
		case Tango::DEV_USHORT:
			return strToDevUShortData(source);
		case Tango::DEV_ULONG:
			return strToDevULongData(source);
		case Tango::DEV_STRING:
			return strToDevStringData(source);
		case Tango::DEVVAR_CHARARRAY:
			return strToDevCharArrayData(source);
		case Tango::DEVVAR_SHORTARRAY:
			return strToDevShortArrayData(source);
		case Tango::DEVVAR_LONGARRAY:
			return strToDevLongArrayData(source);
		case Tango::DEVVAR_FLOATARRAY:
			return strToDevFloatArrayData(source);
		case Tango::DEVVAR_DOUBLEARRAY:
			return strToDevDoubleArrayData(source);
		case Tango::DEVVAR_USHORTARRAY:
			return strToDevUShortArrayData(source);
		case Tango::DEVVAR_ULONGARRAY:
			return strToDevULongArrayData(source);
		case Tango::DEVVAR_STRINGARRAY:
			return strToDevStringArrayData(source);
		case Tango::DEVVAR_LONGSTRINGARRAY:
			return strToDevLongStringArrayData(source);
		case Tango::DEVVAR_DOUBLESTRINGARRAY:
			return strToDevDoubleStringArrayData(source);
		case Tango::DEV_STATE:
			return strToDevStateData(source);
		case Tango::CONST_DEV_STRING:
			return strToConstStringData(source);
		case Tango::DEVVAR_BOOLEANARRAY:
			return strToDevBooleanArrayData(source);
		case Tango::DEV_UCHAR:
			return strToDevUCharData(source);
		case Tango::DEV_LONG64:
			return strToDevLong64Data(source);
		case Tango::DEV_ULONG64:
			return strToDevULong64Data(source);
		case Tango::DEVVAR_LONG64ARRAY:
			return strToDevLong64ArrayData(source);
		case Tango::DEVVAR_ULONG64ARRAY:
			return strToDevULong64ArrayData(source);
		case Tango::DEV_INT:
			return strToDevIntData(source);
		case Tango::DEV_ENCODED:
			return strToDevEncodedData(source);
		default:
			break;
		}
	}

	std::string tango_processor::SwitchTangoState(Tango::DevState state)
	{

		switch (state)
		{
		case Tango::ON:
			return std::string("ON");
			break;
		case Tango::OFF:
			return std::string("OFF");
			break;
		case Tango::CLOSE:
			return std::string("CLOSE");
			break;
		case Tango::OPEN:
			return std::string("OPEN");
			break;
		case Tango::INSERT:
			return std::string("INSERT");
			break;
		case Tango::EXTRACT:
			return std::string("EXTRACT");
			break;
		case Tango::MOVING:
			return std::string("MOVING");
			break;
		case Tango::STANDBY:
			return std::string("STANDBY");
			break;
		case Tango::FAULT:
			return std::string("FAULT");
			break;
		case Tango::INIT:
			return std::string("INIT");
			break;
		case Tango::RUNNING:
			return std::string("RUNNING");
			break;
		case Tango::ALARM:
			return std::string("ALARM");
			break;
		case Tango::DISABLE:
			return std::string("DISABLE");
			break;
		default:
			return std::string("UNKNOWN");
		}
	}

	std::string tango_processor::SwitchAttrQuality(Tango::AttrQuality quality)
	{
		//ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING, ATTR_WARNING
		switch (quality)
		{
		case Tango::AttrQuality::ATTR_VALID:
			return std::string("VALID");
			break;
		case Tango::AttrQuality::ATTR_INVALID:
			return std::string("INVALID");
			break;
		case Tango::AttrQuality::ATTR_ALARM:
			return std::string("ALARM");
			break;
		case Tango::AttrQuality::ATTR_CHANGING:
			return std::string("CHANGING");
			break;
		case Tango::AttrQuality::ATTR_WARNING:
			return std::string("WARNING");
			break;
		default:
			return std::string("INVALID");
		}
	}


	// Encoded
	std::string tango_processor::devEncodedToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}

	//std::string tango_processor::devEncodedScalarToStr(Tango::DeviceAttribute *attr)
	//{
	//	return "";
	//}

	std::string tango_processor::devEncodedSpectrumToStr(Tango::DeviceAttribute *attr)
	{
		return "";
	}

	std::string tango_processor::devEncodedImageToStr(Tango::DeviceAttribute *attr)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevEncodedData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}


	// Void
	std::string tango_processor::devVoidToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevVoidData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}


	// State
	std::string tango_processor::devStateToStr(Tango::DeviceData *deviceData)
	{
		Tango::DevState data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "\"" << SwitchTangoState(data) << "\"";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevStateData(std::string data)
	{
		Tango::DeviceData d;

		return d;
	}


	// Char
	std::string tango_processor::devCharArrayToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}


	// Char array
	Tango::DeviceData tango_processor::strToDevCharArrayData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}


	// Boolean
	std::string tango_processor::devBooleanToStr(Tango::DeviceData *deviceData)
	{
		bool data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << std::boolalpha << data;
		return ss.str();
	}

	std::string tango_processor::devBooleanArrayToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevBooleanData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		bool parsed = pt.get<bool>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevBooleanArrayData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevBooleanScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevBooleanSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevBooleanImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// String
	std::string tango_processor::devStringToStr(Tango::DeviceData *deviceData)
	{
		std::string data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "\"" << data << "\"";
		return ss.str();
	}

	std::string tango_processor::devStringArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<std::string> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<std::string>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << "\"" << *it << "\"";
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevStringData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		std::string parsed = pt.get<std::string>("argin");
		std::cout << parsed << std::endl;
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevStringArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<std::string> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.data());
			//std::cout << v.second.data() << std::endl;
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevStringScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevStringSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevStringImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// Short
	std::string tango_processor::devShortToStr(Tango::DeviceData *deviceData)
	{
		short data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devShortArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<short> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<short>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevShortData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		short parsed = pt.get<short>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevShortArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<short> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<short>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevShortScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevShortSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevShortImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// UShort
	std::string tango_processor::devUShortToStr(Tango::DeviceData *deviceData)
	{
		unsigned short data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devUShortArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<unsigned short> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<unsigned short>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevUShortData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		unsigned short parsed = pt.get<unsigned short>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevUShortArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<unsigned short> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<unsigned short>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevUShortScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevUShortSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevUShortImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// Long
	std::string tango_processor::devLongToStr(Tango::DeviceData *deviceData)
	{
		long data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devLongArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<long> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<long>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevLongData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		long parsed = pt.get<long>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevLongArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<long> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<long>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevLongScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevLongSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevLongImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// ULong
	std::string tango_processor::devULongToStr(Tango::DeviceData *deviceData)
	{
		unsigned long data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devULongArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<unsigned long> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<unsigned long>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevULongData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		unsigned long parsed = pt.get<unsigned long>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevULongArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<unsigned long> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<unsigned long>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevULongScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevULongSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevULongImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// Long64
	std::string tango_processor::devLong64ToStr(Tango::DeviceData *deviceData)
	{
		Tango::DevLong64 data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devLong64ArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<Tango::DevLong64> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<Tango::DevLong64>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevLong64Data(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		Tango::DevLong64 parsed = pt.get<Tango::DevLong64>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevLong64ArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<Tango::DevLong64> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<Tango::DevLong64>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevLong64ScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevLong64SpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevLong64ImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// ULong64
	std::string tango_processor::devULong64ToStr(Tango::DeviceData *deviceData)
	{
		Tango::DevULong64 data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << data;
		return ss.str();
	}

	std::string tango_processor::devULong64ArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<Tango::DevULong64> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<Tango::DevULong64>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevULong64Data(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		Tango::DevULong64 parsed = pt.get<Tango::DevULong64>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevULong64ArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<Tango::DevULong64> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<Tango::DevLong64>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevULong64ScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevULong64SpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevULong64ImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// Float
	std::string tango_processor::devFloatToStr(Tango::DeviceData *deviceData)
	{
		float data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << std::setprecision(5) << data;
		return ss.str();
	}

	std::string tango_processor::devFloatArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<float> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<float>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << std::setprecision(5) << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevFloatData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		float parsed = pt.get<float>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevFloatArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<float> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<float>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevFloatScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevFloatSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevFloatImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}


	// Double
	std::string tango_processor::devDoubleToStr(Tango::DeviceData *deviceData)
	{
		double data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << std::setprecision(5) << data;
		return ss.str();
	}

	std::string tango_processor::devDoubleArrayToStr(Tango::DeviceData *deviceData)
	{
		std::vector<double> data;
		(*deviceData) >> data;
		std::stringstream ss;
		ss << "\"argout\": ";
		ss << "[";
		for (std::vector<double>::iterator it = data.begin(); it != data.end(); ++it)
		{
			if (it != data.begin())
				ss << ", ";
			ss << std::setprecision(5) << *it;
		}
		ss << "]";
		return ss.str();
	}

	Tango::DeviceData tango_processor::strToDevDoubleData(std::string data)
	{
		Tango::DeviceData d;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);
		double parsed = pt.get<double>("argin");
		d << parsed;
		return d;
	}

	Tango::DeviceData tango_processor::strToDevDoubleArrayData(std::string data)
	{
		Tango::DeviceData d;
		std::vector<double> parsed;
		boost::property_tree::ptree pt;
		std::stringstream ss;
		ss << data;
		boost::property_tree::read_json(ss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("argin"))
		{
			assert(v.first.empty()); // array elements have no names
			parsed.push_back(v.second.get_value<double>());
		}
		d << parsed;
		return d;
	}

	Tango::DeviceAttribute tango_processor::strToDevDoubleScalarAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevDoubleSpectrumAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	Tango::DeviceAttribute tango_processor::strToDevDoubleImageAttr(std::string data)
	{
		Tango::DeviceAttribute a;
		return a;
	}

	// Long string
	std::string tango_processor::devLongStringArrayToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevLongStringArrayData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}

	// Double string
	std::string tango_processor::devDoubleStringArrayToStr(Tango::DeviceData *deviceData)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevDoubleStringArrayData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}

	// Const string
	std::string tango_processor::devConstStringToStr(Tango::DeviceData *data)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToConstStringData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}

	// UChar
	std::string tango_processor::devUCharToStr(Tango::DeviceData *data)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevUCharData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}

	// Int
	std::string tango_processor::devIntToStr(Tango::DeviceData *data)
	{
		return "";
	}

	Tango::DeviceData tango_processor::strToDevIntData(std::string data)
	{
		Tango::DeviceData d;
		return d;
	}
}
