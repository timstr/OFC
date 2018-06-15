#include "gui/stringhelpers.h"
#include <sstream>
#include <limits>

namespace {
	std::wstringstream stream;
}

std::wstring toString(float x){
	stream.str(L"");
	stream.clear();
	stream << x;
	return stream.str();
}

std::wstring toString(double x){
	stream.str(L"");
	stream.clear();
	stream << x;
	return stream.str();
}

std::wstring toString(int x){
	stream.str(L"");
	stream.clear();
	stream << x;
	return stream.str();
}

float stringToFloat(const std::wstring& str){
	stream.str(str);
	stream.clear();
	float x;
	stream >> x;
	if (!stream.fail() && stream.eof()){
		return x;
	} else {
		return std::numeric_limits<float>::quiet_NaN();
	}
}

double stringToDouble(const std::wstring& str){
	stream.str(str);
	stream.clear();
	double x;
	stream >> x;
	if (!stream.fail() && stream.eof()){
		return x;
	} else {
		return std::numeric_limits<double>::quiet_NaN();
	}
}

int stringToInt(const std::wstring& str, int default_value){
	stream.str(str);
	stream.clear();
	int x;
	stream >> x;
	if (!stream.fail() && stream.eof()){
		return x;
	} else {
		return default_value;
	}
}

bool stringIsInt(const std::wstring& str){
	stream.str(str);
	stream.clear();
	int x;
	stream >> x;
	return !stream.fail() && stream.eof();
}