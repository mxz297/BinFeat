#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include "FeatureAnalyzer.h"
#include "FeatureQueue.h"
#include <thread>
#include <vector>
#include <string>
#include <cstdio>

class FeatureManager {
    std::thread *consumer;
    std::string outPrefix;
    FILE *featFile;
    int featSize;
    FeatureQueue q;
    std::set<std::string> internalFuncs;
    std::vector<ParseAPI::Function*> fvec;
    std::vector<FeatureAnalyzer*> passes;

    Dyninst::SymtabAPI::Symtab *symObj;
    Dyninst::ParseAPI::CodeObject *co;
    bool InTextSection(Dyninst::ParseAPI::Function *f);

    typedef unordered_map<std::string, int> FeatureIndexType;
    FeatureIndexType featureIndex;

    int GetFeatureIndex(const std::string&);
    void Consume();


public:
    void printFeatureList();
    // We will create a CodeObject for the file to analyze,
    // set the wanted feature size, and the prefix of the output files
    int setup(const char *binPath, int featSize, const char *outPrefix);
    void parse();
    void sortFuncs();
    void runPass(int pass);
    void addPass(FeatureAnalyzer*);
    void finish();

};

#endif
