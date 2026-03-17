#include "dtknastranpchdefinitions.h"
#include "dtknastrangraphpchdatastore.h"
#include <algorithm>

void PchDataStore::addEntry(const PchEntry& entry) {
    m_allEntries.push_back(entry);
    m_isSorted = false;
}

void PchDataStore::registerMetadata(int subcase, const std::string& cat, int eType)
{
    m_uiNavigationTree[subcase].insert({ cat, eType });
}

void PchDataStore::finalize()
{
    if (m_isSorted) return;

    std::sort(m_allEntries.begin(), m_allEntries.end(), [](const PchEntry& a, const PchEntry& b)
    {
        if (a.subcase != b.subcase) return a.subcase < b.subcase;
        if (a.eType != b.eType) return a.eType < b.eType;
        if (a.parentID != b.parentID) return a.parentID < b.parentID;
        if (a.gridID != b.gridID) return a.gridID < b.gridID;
        if (a.loc != b.loc) return a.loc < b.loc;
        if (a.comp != b.comp) return a.comp < b.comp;
        return a.xVal < b.xVal;
    });

    m_isSorted = true;
}

void PchDataStore::getCurveData(int subcase, int eType, int pID, int gID, LocationType loc, Component comp,
    std::vector<double>& outX, std::vector<double>& outY)
{
    if (!m_isSorted)
    {
        finalize();
    }

    outX.clear();
    outY.clear();

    // 1. 定义一个用于比较的“伪项”，包含所有关键维度
    PchEntry searchKey;
    searchKey.subcase = subcase;
    searchKey.eType = eType;
    searchKey.parentID = pID;
    searchKey.gridID = gID;
    searchKey.loc = loc;
    searchKey.comp = comp;

    // 2. 定义严格的比较规则，确保二分查找的顺序与 finalize() 中的排序一致
    auto compareForSearch = [](const PchEntry& a, const PchEntry& b)
    {
        if (a.subcase != b.subcase) return a.subcase < b.subcase;
        if (a.eType != b.eType) return a.eType < b.eType;
        if (a.parentID != b.parentID) return a.parentID < b.parentID;
        if (a.gridID != b.gridID) return a.gridID < b.gridID;
        if (a.loc != b.loc) return a.loc < b.loc;
        return a.comp < b.comp;
    };

    // 3. 使用 std::equal_range 进行高效查找
    auto range = std::equal_range(m_allEntries.begin(), m_allEntries.end(), searchKey, compareForSearch);

    // 4. 提取结果
    for (auto it = range.first; it != range.second; ++it)
    {
        outX.push_back(static_cast<double>(it->xVal));
        outY.push_back(static_cast<double>(it->yVal));
    }
}
