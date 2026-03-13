#ifndef DTKNASTRANGRAPHPCHMAPPING_H
#define DTKNASTRANGRAPHPCHMAPPING_H
#include "dtknastranpchdefinitions.h"
#include <map>

struct ElementLayout {
    int repeatCount = 0;
    int wordsPerPoint = 0;
    struct Info { Component comp; LocationType loc; };
    std::map<int, Info> wordToInfo;
};

class PchMapping {
public:
    ElementLayout getLayout(int elementType, const std::string& category,bool isComplex,bool isMagPhase);
};
#endif
