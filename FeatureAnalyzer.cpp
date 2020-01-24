#include <algorithm>
#include <stdio.h>
#include "FeatureAnalyzer.h"

#include "CodeSource.h"
#include <thread>

#include <mutex>
std::mutex consumeMutex;

using namespace std;

struct FuncSort {
    bool operator() (ParseAPI::Function *a, ParseAPI::Function *b) {
        return a->num_blocks() > b->num_blocks();
    }
};
void FeatureAnalyzer::PrintFeatureList() {
    string listFile = outPrefix + ".featlist";
    FILE *f = fopen(listFile.c_str(), "w");
    vector< pair<int, string> > featList;
    for (auto fit = featureIndex.begin(); fit != featureIndex.end(); ++fit)
        featList.push_back(make_pair(fit->second, fit->first));
    sort(featList.begin(), featList.end());
    for (auto fit = featList.begin(); fit != featList.end(); ++fit)
        fprintf(f, "%d %s\n", fit->first, fit->second.c_str());
    fclose(f);
}

FeatureAnalyzer::FeatureAnalyzer() {}

void FeatureAnalyzer::Analyze() {
    featFile = fopen(string(outPrefix + ".instances").c_str(), "w");
    Produce();
    fclose(featFile);
    PrintFeatureList();

}


int FeatureAnalyzer::Setup(const char *bPath, int fSize, const char *oPre) {
    featSize = fSize;
    outPrefix = string(oPre);

    SymtabCodeSource *sts = new SymtabCodeSource((char *)bPath);
    if (sts == NULL) {
        fprintf(stderr, "Cannot create SymtabCodeSource for %s\n", bPath);
        return -1;
    }
    co = new CodeObject(sts);
    if (co == NULL) {
        fprintf(stderr, "Cannot create CodeObject for %s\n", bPath);
        return -1;
    }
    co->parse();
    return 0;
}

int FeatureAnalyzer::GetFeatureIndex(const std::string &feat) {
    auto it = featureIndex.find(feat);
    if (it == featureIndex.end()) {
        int newIndex = featureIndex.size() + 1;
        featureIndex[feat] = newIndex;
        return newIndex;
    } else {
        return it->second;
    }
}

void FeatureAnalyzer::Consume(InstanceDataType* it) {
    const std::lock_guard<std::mutex> lock(consumeMutex);
    fprintf(featFile, "%lx", it->f->addr());
    for (auto pair : it->featPair) {
        int index = GetFeatureIndex(pair.first);
        fprintf(featFile, " %d:%.3lf", index, pair.second);
    }
    fprintf(featFile, "\n");
}

void FeatureAnalyzer::Produce() {
    std::vector<ParseAPI::Function*> fvec;
    for (auto fit = co->funcs().begin(); fit != co->funcs().end(); ++fit) {
        if (InTextSection(*fit))
            fvec.push_back(*fit);
    }
    sort(fvec.begin(), fvec.end(), FuncSort());
//    fprintf(stderr, "total function %d\n", fvec.size());

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < fvec.size(); ++i) {
//        fprintf(stderr, "current function %s at %lx\n", fvec[i]->name().c_str(), fvec[i]->addr());
        InstanceDataType idt;
        idt.f = fvec[i];
        ProduceAFunction(&idt);
        Consume(&idt);
    }
}

bool FeatureAnalyzer::InTextSection(Dyninst::ParseAPI::Function *f) {
    if (co->cs()->linkage().find(f->addr()) == co->cs()->linkage().end()) return true;
    return false;
}
