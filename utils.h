#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <string>
#include <vector>

constexpr std::size_t MINI_STATEMENT_SIZE = 5;

std::string toLower(std::string text);
std::string sanitizeField(const std::string& input);
std::vector<std::string> split(const std::string& line, char delim = '|');
std::string nowTimestamp();
std::string hashPinBasic(const std::string& pin);
std::string readLine(const std::string& prompt);
int readInt(const std::string& prompt);
double readDouble(const std::string& prompt);

#endif
