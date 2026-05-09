#include "utils.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

std::string sanitizeField(const std::string& input) {
    std::string out = input;
    std::replace(out.begin(), out.end(), '|', '/');
    return out;
}

std::vector<std::string> split(const std::string& line, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delim)) {
        parts.push_back(item);
    }
    return parts;
}

std::string nowTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return os.str();
}

std::string hashPinBasic(const std::string& pin) {
    unsigned long long h = 1469598103934665603ULL;
    for (const unsigned char c : pin) {
        h ^= static_cast<unsigned long long>(c);
        h *= 1099511628211ULL;
        h ^= (h >> 13);
    }
    std::ostringstream os;
    os << std::hex << h;
    return os.str();
}

std::string readLine(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int readInt(const std::string& prompt) {
    while (true) {
        try {
            const std::string s = readLine(prompt);
            size_t pos = 0;
            int value = std::stoi(s, &pos);
            if (pos == s.size()) {
                return value;
            }
        } catch (...) {
        }
        std::cout << "Invalid number. Try again.\n";
    }
}

double readDouble(const std::string& prompt) {
    while (true) {
        try {
            const std::string s = readLine(prompt);
            size_t pos = 0;
            double value = std::stod(s, &pos);
            if (pos == s.size()) {
                return value;
            }
        } catch (...) {
        }
        std::cout << "Invalid amount. Try again.\n";
    }
}
