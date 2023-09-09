#include "GUI/Helpers/StringHelpers.hpp"
#include <sstream>
#include <limits>

namespace ui {

	namespace {
		std::wstringstream stream;
	}

	std::wstring toString(float x) {
		stream.str(L"");
		stream.clear();
		stream << x;
		return stream.str();
	}

	std::wstring toString(double x) {
		stream.str(L"");
		stream.clear();
		stream << x;
		return stream.str();
	}

	std::wstring toString(int x) {
		stream.str(L"");
		stream.clear();
		stream << x;
		return stream.str();
	}

	ParseResult<float> stringToFloat(const std::wstring& str) {
		stream.str(str);
		stream.clear();
		float x;
		stream >> x;
		if (!stream.fail() && stream.eof()) {
			return ParseResult<float>(x, true);
		} else {
			return ParseResult<float>(std::numeric_limits<float>::quiet_NaN(), false);
		}
	}

	ParseResult<double> stringToDouble(const std::wstring& str) {
		stream.str(str);
		stream.clear();
		double x;
		stream >> x;
		if (!stream.fail() && stream.eof()) {
			return ParseResult<double>(x, true);;
		} else {
			return ParseResult<double>(std::numeric_limits<double>::quiet_NaN(), false);
		}
	}

	ParseResult<int> stringToInt(const std::wstring& str) {
		stream.str(str);
		stream.clear();
		int x;
		stream >> x;
		if (!stream.fail() && stream.eof()) {
			return ParseResult<int>(x, true);
		} else {
			return ParseResult<int>(x, false);
		}
	}

	bool stringIsInt(const std::wstring& str) {
		stream.str(str);
		stream.clear();
		int x;
		stream >> x;
		return !stream.fail() && stream.eof();
	}

} // namespace ui