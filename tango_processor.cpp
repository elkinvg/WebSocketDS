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

namespace WebSocketDS_ns
{

std::string tango_processor::process_attribute(Tango::DeviceAttribute att)
{
	std::stringstream json;
	json << "";

	Tango::AttrQuality quality = att.get_quality();
			
	json << "{";
	json << "\"attr\": \"" << att.get_name() << "\", ";
	json << "\"qual\": \"" << SwitchAttrQuality(att.get_quality()) << "\", ";
	json << "\"time\": " << att.time.tv_sec << ", ";
			
	int type = att.get_type();
	Tango::AttrDataFormat format = att.get_data_format();
			
	switch (type)
	{
	case Tango::DEV_BOOLEAN:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devBooleanScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devBooleanSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devBooleanImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_SHORT:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devShortScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devShortSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devShortImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_LONG:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devLongScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devLongSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devLongImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_LONG64:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devLong64ScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devLong64SpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devLong64ImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_FLOAT:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devFloatScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devFloatSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devFloatImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_DOUBLE:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devDoubleScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devDoubleSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devDoubleImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_UCHAR:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devUCharScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devUCharSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devUCharImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_USHORT:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devUShortScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devUShortSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devUShortImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_ULONG:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devULongScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devULongSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devULongImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_ULONG64:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devULong64ScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devULong64SpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devULong64ImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_STRING:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devStringScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devStringSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devStringImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_STATE:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devStateScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devStateSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devStateImageToStr(&att);
				break;
			}
			break;
	case Tango::DEV_ENCODED:
			switch (format)
			{
			case Tango::AttrDataFormat::SCALAR:
				json << devEncodedScalarToStr(&att);
				break;
			case Tango::AttrDataFormat::SPECTRUM:
				json << devEncodedSpectrumToStr(&att);
				break;
			case Tango::AttrDataFormat::IMAGE:
				json << devEncodedImageToStr(&att);
				break;
			}
			break;
		default: 
			break;					
	}
	json << "}";
	return json.str();
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

std::string tango_processor::devEncodedScalarToStr(Tango::DeviceAttribute *attr)
{
	return "";
}

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
	ss << "\"" <<  SwitchTangoState(data) << "\"";
	return ss.str();
}

std::string tango_processor::devStateScalarToStr(Tango::DeviceAttribute *attr)
{
	Tango::DevState data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << "\"" <<  SwitchTangoState(data) << "\"";
	return ss.str();
}

std::string tango_processor::devStateSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevState> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevState>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << "\"" <<  SwitchTangoState(*it) << "\"";
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devStateImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevState> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevState>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << "\"" <<  SwitchTangoState(*it) << "\"";
	}
	ss << "]";
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


// UChar
std::string tango_processor::devUCharScalarToStr(Tango::DeviceAttribute *attr)
{
	return "";
}

std::string tango_processor::devUCharSpectrumToStr(Tango::DeviceAttribute *attr)
{
	return "";
}

std::string tango_processor::devUCharImageToStr(Tango::DeviceAttribute *attr)
{
	return "";
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

std::string tango_processor::devBooleanScalarToStr(Tango::DeviceAttribute *attr)
{
	bool data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << std::boolalpha << data;
	return ss.str();
}

std::string tango_processor::devBooleanSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<bool> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<bool>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::boolalpha << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devBooleanImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<bool> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<bool>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::boolalpha << *it;
	}
	ss << "]";
	return ss.str();
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
	for (std::vector<std::string>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << "\""<< *it << "\"";
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devStringScalarToStr(Tango::DeviceAttribute *attr)
{
	std::string data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << "\"" << data << "\"";
	return ss.str();
}

std::string tango_processor::devStringSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<std::string> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<std::string>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << "\""<< *it << "\"";
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devStringImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<std::string> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<std::string>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << "\""<< *it << "\"";
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
	for (std::vector<short>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devShortScalarToStr(Tango::DeviceAttribute *attr)
{
	short data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devShortSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<short> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<short>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devShortImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<short> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<short>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<unsigned short>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devUShortScalarToStr(Tango::DeviceAttribute *attr)
{
	unsigned short data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devUShortSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<unsigned short> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<unsigned short>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devUShortImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<unsigned short> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<unsigned short>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<long>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devLongScalarToStr(Tango::DeviceAttribute *attr)
{
	long data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devLongSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<long> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<long>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devLongImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<long> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<long>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<unsigned long>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devULongScalarToStr(Tango::DeviceAttribute *attr)
{
	unsigned long data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devULongSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<unsigned long> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<unsigned long>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devULongImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<unsigned long> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<unsigned long>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<Tango::DevLong64>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devLong64ScalarToStr(Tango::DeviceAttribute *attr)
{
	Tango::DevLong64 data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devLong64SpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevLong64> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevLong64>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devLong64ImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevLong64> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevLong64>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<Tango::DevULong64>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devULong64ScalarToStr(Tango::DeviceAttribute *attr)
{
	Tango::DevULong64 data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << data;
	return ss.str();
}

std::string tango_processor::devULong64SpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevULong64> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevULong64>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devULong64ImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<Tango::DevULong64> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<Tango::DevULong64>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<float>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::setprecision(5) << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devFloatScalarToStr(Tango::DeviceAttribute *attr)
{
	float data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << std::setprecision(5) << data;
	return ss.str();
}

std::string tango_processor::devFloatSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<float> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<float>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::setprecision(5) << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devFloatImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<float> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<float>::iterator it = data.begin() ; it != data.end(); ++it)
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
	for (std::vector<double>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::setprecision(5) << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devDoubleScalarToStr(Tango::DeviceAttribute *attr)
{
	double data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"data\": ";
	ss << std::setprecision(5) << data;
	return ss.str();
}

std::string tango_processor::devDoubleSpectrumToStr(Tango::DeviceAttribute *attr)
{
	std::vector<double> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<double>::iterator it = data.begin() ; it != data.end(); ++it)
	{
		if (it != data.begin())
			ss << ", ";
		ss << std::setprecision(5) << *it;
	}
	ss << "]";
	return ss.str();
}

std::string tango_processor::devDoubleImageToStr(Tango::DeviceAttribute *attr)
{
	std::vector<double> data;
	(*attr) >> data;
	std::stringstream ss;
	ss << "\"dimX\": " << attr->dim_x << ", ";
	ss << "\"dimY\": " << attr->dim_y << ", ";
	ss << "\"data\": ";
	ss << "[";
	for (std::vector<double>::iterator it = data.begin() ; it != data.end(); ++it)
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
