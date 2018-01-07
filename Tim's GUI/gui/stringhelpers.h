#pragma once

#include <string>

std::string toString(float x);

std::string toString(double x);

std::string toString(int x);

// returns entire string read as float, or NaN
float stringToFloat(const std::string& str);

// returns entire string read as double, or NaN
double stringToDouble(const std::string& str);

// returns entire string read as int, or default_value
int stringToInt(const std::string& str, int default_value = 0);