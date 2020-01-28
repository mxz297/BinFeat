#ifndef FEATURE_ANALYZER_H
#define FEATURE_ANALYZER_H


#include "CFG.h"
#include "feature.h"
#include <unordered_map>

#define dprintf if (config->debug) printf

struct InstanceDataType {
    ParseAPI::Function* f;
    std::unordered_map<std::string, double> featPair;
};


class FeatureAnalyzer {
public:
    virtual void ProduceAFunction(InstanceDataType*) = 0;
};

class IdiomAnalyzer : public FeatureAnalyzer {
public:
    virtual void ProduceAFunction(InstanceDataType*);

};

class GraphletAnalyzer : public FeatureAnalyzer {
    static bool ANON;
    static int MERGE;
    bool color;
public:
    GraphletAnalyzer(bool c): color(c) {}
    virtual void ProduceAFunction(InstanceDataType*);

};

class RegisterLivenessAnalyzer : public FeatureAnalyzer {
public:
    virtual void ProduceAFunction(InstanceDataType*);

};

class StackHeightAnalyzer : public FeatureAnalyzer {
public:
    virtual void ProduceAFunction(InstanceDataType*);

};

class SlicingAnalyzer : public FeatureAnalyzer {
public:
    virtual void ProduceAFunction(InstanceDataType*);

};

#endif
