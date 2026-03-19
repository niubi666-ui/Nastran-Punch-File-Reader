#include "dtknastrangraphpchutils.h"
#include <algorithm>

double PchUtils::safeStod(const std::string& str) {
    if (str.empty()) return 0.0;
    bool isAllSpace = true;
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) { isAllSpace = false; break; }
    }
    if (isAllSpace) return 0.0;
    try { return std::stod(str); }
    catch (...) { return 0.0; }
}

std::string PchUtils::getField(const std::string& line, int wordIndex)
{
    int start = 0;
    int width = 0;

    // 期望列（0-based 索引）:
    // field0: cols 0..15  (width 16)
    // field1: cols 16..21 (width 6)
    // field2: cols 22..35 (width 14)
    // field3: cols 40..53 (width 14)
    // field4: cols 59..72 (width 14)
    if (wordIndex == 0) { start = 0;  width = 16; }
    else if (wordIndex == 1) { start = 16; width = 6; }
    else if (wordIndex == 2) { start = 22; width = 14; }
    else if (wordIndex == 3) { start = 40; width = 14; }
    else if (wordIndex == 4) { start = 59; width = 14; }
    else { return std::string(); }

    if (start >= static_cast<int>(line.length())) return "";
    int avail = static_cast<int>(line.length()) - start;
    int useWidth = std::min(width, avail);
    std::string s = line.substr(start, useWidth);

    // 清理空格
    size_t first = s.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(' ');
    return s.substr(first, (last - first + 1));
}
