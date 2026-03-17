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
    int width = 16;

    // 针对 PCH 特殊列宽的硬核对齐
    if (wordIndex == 0)
    {
        start = 0; width = 16;  // Word 1: 频率或 ID
    }
    else if (wordIndex == 1)
    {
        start = 16; width = 7;  // Word 2: G 字符所在区域
    }
    else
    {
        // 从 Word 3 开始，起始点固定在 23，每字段 16 位
        start = 23 + (wordIndex - 2) * 16;
        width = 16;
    }

    if (start >= (int)line.length()) return "";

    int actualWidth = std::min(width, (int)(line.length() - start));
    std::string s = line.substr(start, actualWidth);

    // 清理空格
    size_t first = s.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(' ');
    return s.substr(first, (last - first + 1));
}
