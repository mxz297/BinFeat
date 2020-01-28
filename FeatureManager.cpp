#include "FeatureManager.h"
#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"

struct FuncSort {
    bool operator() (ParseAPI::Function *a, ParseAPI::Function *b) {
        return a->num_blocks() > b->num_blocks();
    }
};

void FeatureManager::parse() {
    SymtabCodeSource *sts = new SymtabCodeSource(symObj);
    co = new CodeObject(sts);
    co->parse();
}

void FeatureManager::sortFuncs() {
    for (auto f: co->funcs()) {
        if (InTextSection(f))
            fvec.push_back(f);
    }
    sort(fvec.begin(), fvec.end(), FuncSort());
}


int FeatureManager::setup(const char *bPath, int fSize, const char *oPre) {
    featSize = fSize;
    outPrefix = string(oPre);
    
    symObj = NULL;
    if (!SymtabAPI::Symtab::openFile(symObj, bPath)) {
        fprintf(stderr, "Cannot open file %s\n", bPath);
        return -1; 
    }   


    featFile = fopen(string(outPrefix + ".instances").c_str(), "w");
    consumer = new std::thread(&FeatureManager::Consume, this);
    return 0;
}

void FeatureManager::printFeatureList() {
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

int FeatureManager::GetFeatureIndex(const std::string &feat) {
    auto it = featureIndex.find(feat);
    if (it == featureIndex.end()) {
        int newIndex = featureIndex.size() + 1;
        featureIndex[feat] = newIndex;
        return newIndex;
    } else {
        return it->second;
    }
}

void FeatureManager::finish() {
    q.finish();
    consumer->join();
    delete consumer;
    fclose(featFile);
}

void FeatureManager::Consume() {
    while (true) {
        InstanceDataType* it = (InstanceDataType*) q.dequeue();
        if (it == NULL) break;
        fprintf(featFile, "%lx", it->f->addr());
        for (auto pair : it->featPair) {
            int index = GetFeatureIndex(pair.first);
            fprintf(featFile, " %d:%.3lf", index, pair.second);
        }
        fprintf(featFile, "\n");
        delete it;
    }
}

void FeatureManager::runPass(int pass) {
    FeatureAnalyzer *p = passes[pass];
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < fvec.size(); ++i) {
        InstanceDataType* idt = new InstanceDataType();
        idt->f = fvec[i];
        p->ProduceAFunction(idt);
        q.enqueue((void*)idt);
    }
}

bool FeatureManager::InTextSection(Dyninst::ParseAPI::Function *f) {
    if (co->cs()->linkage().find(f->addr()) == co->cs()->linkage().end()) return true;
    return false;
}

void FeatureManager::addPass(FeatureAnalyzer* fa) {
    passes.push_back(fa);
}
