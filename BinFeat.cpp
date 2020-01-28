#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <stdlib.h>
#include "FeatureManager.h"
#include "time.h"

float timeDiff(struct timespec &t1, struct timespec &t0) {
    return ((t1.tv_sec - t0.tv_sec) * 1000000000.0 + (t1.tv_nsec - t0.tv_nsec)) / 1000000000.0;
}

using namespace std;

int main(int argc, char **argv){
   
    if (argc != 5){
        fprintf(stderr, "Usage: BinFeat <binPath> <feature type> <feature size> <out file path>\n");
        exit(1);
    }

    int featSize = atoi(argv[3]);
    if (featSize == 0) {
        fprintf(stderr, "Wrong feature size!\n");
        exit(1);
    }
    struct timespec t_start;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    FeatureManager fm;
    fm.addPass(new IdiomAnalyzer());
    fm.addPass(new GraphletAnalyzer(true));
    fm.addPass(new RegisterLivenessAnalyzer());
//    fm.addPass(new StackHeightAnalyzer());
    fm.addPass(new SlicingAnalyzer());
    fm.setup(argv[1], featSize, argv[4]);
    struct timespec t_before_parse;
    clock_gettime(CLOCK_MONOTONIC, &t_before_parse);
    fm.parse();
    struct timespec t_after_parse;
    clock_gettime(CLOCK_MONOTONIC, &t_after_parse);
    /*
    fm.sortFuncs();
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    // Run machine instruction feature analyzer
    fm.runPass(0);
    struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t2);

    // Run CFG feature analyzer
    fm.runPass(1);
    struct timespec t3;
    clock_gettime(CLOCK_MONOTONIC, &t3);

    // Run data flow feature analyzer
    fm.runPass(2);
    fm.runPass(3);
//    fm.runPass(4);
    struct timespec t4;
    clock_gettime(CLOCK_MONOTONIC, &t4);
    fm.finish();
    fm.printFeatureList();
    struct timespec t5;
    clock_gettime(CLOCK_MONOTONIC, &t5);
*/
    printf("ParseAPI time: %.6lf\n", timeDiff(t_after_parse, t_before_parse));
    /*
    printf("InstructionFeature time: %.6lf\n", timeDiff(t2, t1));
    printf("CFGFeature time: %.6lf\n", timeDiff(t3, t2));
    printf("DFFeature time: %.6lf\n", timeDiff(t4, t3));
    printf("Total time: %.6lf\n", timeDiff(t5, t_start));
    */
}
