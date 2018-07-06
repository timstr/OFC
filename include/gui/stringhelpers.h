#pragma once

#include <string>

std::wstring toString(float x);

std::wstring toString(double x);

std::wstring toString(int x);

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
struct ParseResult {
	ParseResult(T _value, bool _valid) : value(_value), valid(_valid) {

	}

	T getValue() const {
		return value;
	}

	bool isValid() const {
		return valid;
	}

	operator T() const {
		return value;
	}

	operator bool() const {
		return valid;
	}

private:
	const T value;
	const bool valid;
};

// returns entire string read as float, or NaN
ParseResult<float> stringToFloat(const std::wstring& str);

// returns entire string read as double, or NaN
ParseResult<double> stringToDouble(const std::wstring& str);

// returns entire string read as int, or default_value
ParseResult<int> stringToInt(const std::wstring& str);

// convenience template, automatically selects float/double/int
template<typename T>
inline ParseResult<T> stringTo(const std::wstring& str) {
	return ParseResult<T>({}, false);
}

template<>
inline ParseResult<float> stringTo<float>(const std::wstring& str) {
	return stringToFloat(str);
}

template<>
inline ParseResult<double> stringTo<double>(const std::wstring& str) {
	return stringToDouble(str);
}

template<>
inline ParseResult<int> stringTo<int>(const std::wstring& str) {
	return stringToInt(str);
}

bool stringIsInt(const std::wstring& str);