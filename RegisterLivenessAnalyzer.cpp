#include "FeatureAnalyzer.h"

#include "liveness.h"
#include "bitArray.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::DataflowAPI;

void RegisterLivenessAnalyzer::ProduceAFunction(InstanceDataType* idt) {
    ParseAPI::Function *f = idt->f;
    int totalBlock = f->num_blocks();
    int inputCount = 0;
    int outputCount = 0;
    int internalCount = 0;
    int liveRegEntry = 0;
    int liveRegExit = 0;
    int blockUseReg = 0;
    int blockDefReg = 0;
	LivenessAnalyzer la(f->obj()->cs()->getAddressWidth());
	for (auto b : f->blocks()) {
	    livenessData data;
	    if (!la.query(b, data)) {
            continue;
	    }
	    bitArray input = data.use & data.in;
	    bitArray output = data.def & data.out;
	    bitArray internal = (data.use | data.def) - (input | output); 
        inputCount  += input.count();
        outputCount += output.count();
        internalCount += internal.count();
        liveRegEntry += data.in.count();
        liveRegExit += data.out.count();
        blockUseReg += data.use.count();
        blockDefReg += data.def.count();
    }
    idt->featPair["BLOCK_INPUT_REG_AVG"] = inputCount * 1.0 / totalBlock;
    idt->featPair["BLOCK_OUTPUT_REG_AVG"] = outputCount * 1.0 / totalBlock;
    idt->featPair["BLOCK_INTERAL_REG_AVG"] = internalCount * 1.0 / totalBlock;
    idt->featPair["BLOCK_LIVE_REG_ENTRY_AVG"] = liveRegEntry * 1.0 / totalBlock;
    idt->featPair["BLOCK_LIVE_REG_EXIT_AVG"] = liveRegExit * 1.0 / totalBlock;
    idt->featPair["BLOCK_USE_REG_AVG"] = blockUseReg * 1.0 / totalBlock;
    idt->featPair["BLOCK_DEF_REG_AVG"] = blockDefReg * 1.0 / totalBlock;
}
