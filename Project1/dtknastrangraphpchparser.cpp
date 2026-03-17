#include "dtknastrangraphpchparser.h"
#include <fstream>

PchParser::PchParser(PchDataStore& store)
    : m_store(store),
    m_currentSubcase(0),
    m_currentParentID(0),
    m_currentElementType(0),
    m_currentSortMode(SortMode::SORT_1)
{
}

bool PchParser::parse(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // --- 1. 识别类别 (Category) ---
        if (line.find("$DISPLACEMENTS") != std::string::npos) m_currentCategory = "DISPLACEMENT";
        else if (line.find("$VELOCITY") != std::string::npos) m_currentCategory = "VELOCITY";
        else if (line.find("$ACCELERATION") != std::string::npos) m_currentCategory = "ACCELERATION";
        else if (line.find("$ELEMENT STRAINS") != std::string::npos) m_currentCategory = "STRAIN";
        else if (line.find("$ELEMENT STRESSES") != std::string::npos) m_currentCategory = "STRESS";

        // --- 2. 识别 Subcase 和 输出模式 ---
        else if (line.find("$SUBCASE ID =") != std::string::npos) {
            m_currentSubcase = std::stoi(line.substr(line.find("=") + 1));
        }
        else if (line.find("$REAL OUTPUT") != std::string::npos) {
            m_isComplex = false; m_isMagPhase = false;
        }
        else if (line.find("$COMPLEX OUTPUT") != std::string::npos) {
            m_isComplex = true;
        }
        else if (line.find("$MAGNITUDE-PHASE") != std::string::npos) {
            m_isComplex = true; m_isMagPhase = true;
        }
        else if (line.find("$REAL-IMAGINARY") != std::string::npos) {
            m_isComplex = true; m_isMagPhase = false;
        }

        // --- 3. 识别 Sort 模式与元数据 ---
        else if (line.find("$FREQUENCY =") != std::string::npos || line.find("$TIME STEP =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_1;
            m_currentXVal = m_utils.safeStod(line.substr(line.find("=") + 1));
        }
        else if (line.find("$POINT ID =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_2;
            m_currentParentID = std::stoi(line.substr(line.find("=") + 1));
            m_currentElementType = 0;
        }
        else if (line.find("$ELEMENT ID =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_2;
            m_currentParentID = std::stoi(line.substr(line.find("=") + 1));
            // ELEMENT ID 后面通常会跟着 ELEMENT TYPE 标题，我们在下面处理
        }
        else if (line.find("ELEMENT TYPE =") != std::string::npos) {
            m_currentElementType = std::stoi(line.substr(line.find("=") + 1));
        }
        else if (line.find("$") != std::string::npos) {
            continue;
        }

        // --- 4. 关键：识别并解析数据行 ---
        // 如果这行开头是数字，说明这就是我们要的数据。
        // 我们不再在看到标题时调用 processBlock，而是在看到数据时调用解析函数。
        else
        {
            ElementLayout layout = m_mapping.getLayout(m_currentElementType, m_currentCategory, m_isComplex, m_isMagPhase);
            if (layout.wordsPerPoint > 0) {
                // 直接调用解析函数处理这一块（包括后续的 -CONT-）
                parseElementData(file, line, layout);
            }
        }
    }
    m_store.finalize();
    return true;
}

void PchParser::processBlock(std::ifstream& file)
{
    m_store.registerMetadata(m_currentSubcase, m_currentCategory, m_currentElementType);
    ElementLayout layout = m_mapping.getLayout(m_currentElementType, m_currentCategory,m_isComplex,m_isMagPhase);
    std::string line;
    // repeatCount 代表有多少个点（或单元）
    for (int i = 0; i < layout.repeatCount; ++i)
    {
        if (!std::getline(file, line) || line.empty()) break;
        parseElementData(file, line, layout);
    }
}

void PchParser::parseElementData(std::ifstream& file, const std::string& firstLine, const ElementLayout& layout)
{
    std::string currentLine = firstLine;
    int wordsHandled = 0;

    // 循环直到填满该点所需的所有 wordsPerPoint
    while (wordsHandled < layout.wordsPerPoint)
    {
        // --- 1. 处理 Word 1：获取频率 ---
        std::string w1 = m_utils.getField(currentLine, 0);
        if (w1 != "-CONT-" && !w1.empty())
        {
            // 如果不是续行，更新当前频率（解决频率丢失问题）
            m_currentXVal = m_utils.safeStod(w1);
        }

        // --- 2. 处理物理数据 (Word 3, 4, 5) ---
        // 每一行最多提供 3 个数据字段
        for (int f = 0; f < 3 && wordsHandled < layout.wordsPerPoint; ++f)
        {
            // 数据位在 getField 的索引 2, 3, 4 (即 Word 3, 4, 5)
            std::string valStr = m_utils.getField(currentLine, f + 2);

            if (!valStr.empty())
            {
                // targetWordIdx 对应 Mapping 表中的 Word 3, 4, 5...
                int targetWordIdx = wordsHandled + 3;

                if (layout.wordToInfo.count(targetWordIdx))
                {
                    PchEntry entry;
                    entry.subcase = m_currentSubcase;
                    entry.eType = m_currentElementType;
                    entry.parentID = m_currentParentID;
                    entry.gridID = 0;
                    entry.xVal = (float)m_currentXVal; // 使用刚更新的频率
                    entry.yVal = (float)m_utils.safeStod(valStr);
                    entry.comp = layout.wordToInfo.at(targetWordIdx).comp;
                    entry.loc = layout.wordToInfo.at(targetWordIdx).loc;
                    m_store.addEntry(entry);
                }
            }
            wordsHandled++;
        }

        // --- 3. 读取续行 ---
        if (wordsHandled < layout.wordsPerPoint)
        {
            if (!std::getline(file, currentLine)) break;
        }
    }
}
