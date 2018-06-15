#pragma once

#include <string>

std::wstring toString(float x);

std::wstring toString(double x);

std::wstring toString(int x);

// returns entire string read as float, or NaN
float stringToFloat(const std::wstring& str);

// returns entire string read as double, or NaN
double stringToDouble(const std::wstring& str);

// returns entire string read as int, or default_value
int stringToInt(const std::wstring& str, int default_value = 0);

bool stringIsInt(const std::wstring& str);