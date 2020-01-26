#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <stdlib.h>
#include "FeatureAnalyzer.h"
#include "time.h"

#include "Symtab.h"
#include "Region.h"

float timeDiff(struct timespec &t1, struct timespec &t0) {
    return ((t1.tv_sec - t0.tv_sec) * 1000000000.0 + (t1.tv_nsec - t0.tv_nsec)) / 1000000000.0;
}

typedef enum {
    Idiom,
    Graphlet,
    Liveness,
    StackHeight,
    Slicing,
    UNKNOWN
} FeatureType;

map<string, FeatureType> featureMap;


using namespace std;
using namespace Dyninst::SymtabAPI;

void InitFeatureMap() {
    featureMap[string("idiom")] = Idiom;
    featureMap[string("graphlet")] = Graphlet;
    featureMap[string("liveness")] = Liveness;
    featureMap[string("stack")] = StackHeight;
    featureMap[string("slicing")] = Slicing;
}

void PrintCodeRange(char *bPath, char *oPre) {
    Symtab *obj;
    if (!Symtab::openFile(obj, string(bPath))) {
        fprintf(stderr, "Cannot open %s\n", bPath);
	exit(1);
    }
    Region* textReg;
    if (!obj->findRegion(textReg, ".text")) {
        fprintf(stderr, "Cannot find .text section\n");
	exit(1);
    }
    FILE *f = fopen(oPre, "w");
    fprintf(f, "%lx\n", textReg->getDiskOffset() + textReg->getDiskSize());
    fclose(f);
}


int main(int argc, char **argv){
    
    InitFeatureMap();
   
    if (argc != 5){
        fprintf(stderr, "Usage: BinFeat <binPath> <feature type> <feature size> <out file path>\n");
        exit(1);
    }

    int featSize = atoi(argv[3]);
    if (featSize == 0) {
        fprintf(stderr, "Wrong feature size!\n");
	exit(1);
    }

    FeatureAnalyzer *featAnalyzer = NULL;    
    auto fit = featureMap.find(string(argv[2]));
    FeatureType f = FeatureType::UNKNOWN;
    if (featureMap.find(string(argv[2])) != featureMap.end()) {
        f = fit -> second;
    }
    

    switch (f) {
        case Idiom:
            featAnalyzer = new IdiomAnalyzer();
            break;
        case Graphlet:
            featAnalyzer = new GraphletAnalyzer(true);
            break;
        case Liveness:
            featAnalyzer = new RegisterLivenessAnalyzer();
            break;
        case StackHeight:
            featAnalyzer = new StackHeightAnalyzer();
            break;
        case Slicing:
            featAnalyzer = new SlicingAnalyzer();
            break;
        default: 
            PrintCodeRange(argv[1], argv[4]);
            return 0;
    }
    struct timespec t0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    featAnalyzer->Setup(argv[1], featSize, argv[4]);
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    featAnalyzer->Analyze();
    struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t2);
    featAnalyzer->PrintFeatureList();
    struct timespec t3;
    clock_gettime(CLOCK_MONOTONIC, &t3);

    printf("ParseAPI time: %.6lf\n", timeDiff(t1, t0));
    printf("FeatureExtract time: %.6lf\n", timeDiff(t2, t1));
    printf("Total time: %.6lf\n", timeDiff(t3, t0));

}
