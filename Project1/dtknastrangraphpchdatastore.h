#ifndef DTKNASTRANGRAPHPCHDATASTORE_H
#define DTKNASTRANGRAPHPCHDATASTORE_H
#include "dtknastranpchdefinitions.h"
#include <vector>
#include <map>
#include <set>

class PchDataStore {
public:
    void addEntry(const PchEntry& entry);
    void registerMetadata(int subcase, const std::string& cat, int eType);
    void finalize();
    void getCurveData(int subcase, int eType, int pID, int gID, LocationType loc, Component comp,
        std::vector<double>& outX, std::vector<double>& outY);

    std::map<int, std::set<ResultModule>> m_uiNavigationTree;
    
private:
    std::vector<PchEntry> m_allEntries;
    bool m_isSorted = false;
};
#endif
