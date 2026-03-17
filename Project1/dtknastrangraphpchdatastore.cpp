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
    if (!m_isSorted) finalize();

    outX.clear();
    outY.clear();

    //1.定义一个用于比较的“伪项”只填充我们需要查找的关键字段
    PchEntry searchKey;
    searchKey.parentID = pID;
    searchKey.gridID = gID;
    searchKey.loc = loc;
    searchKey.comp = comp;

    //2.定义比较规则
    auto compareForSearch = [](const PchEntry& a, const PchEntry& b)
    {
        if (a.parentID != b.parentID) return a.parentID < b.parentID;
        if (a.gridID != b.gridID) return a.gridID < b.gridID;
        if (a.loc != b.loc) return a.loc < b.loc;
        return a.comp < b.comp;
    };

    //二分查找锁定符合条件的第一个和最后一个元素的迭代器
    auto range = std::equal_range(m_allEntries.begin(), m_allEntries.end(), searchKey, compareForSearch);

    for (auto it = range.first; it != range.second; ++it)
    {
        outX.push_back(static_cast<double>(it->xVal));
        outY.push_back(static_cast<double>(it->yVal));
    }
}
