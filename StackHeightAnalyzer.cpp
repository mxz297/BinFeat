#include "FeatureAnalyzer.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Operand.h"
#include "Visitor.h"
#include "Expression.h"
#include "BinaryFunction.h"
#include "Register.h"
#include "stackanalysis.h"
#include "Immediate.h"
#include "Result.h"

#include "liveness.h"
#include "bitArray.h"
#include "Location.h"

#include "slicing.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include "Graph.h"
#include "Node.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DataflowAPI;

void StackHeightAnalyzer::ProduceAFunction(InstanceDataType* idt) {
    ParseAPI::Function *f = idt->f;
	StackAnalysis sa(f);	
    int maxHeight = 0;
    for (auto b : f->blocks()) {
        Block::Insns insns;
        b->getInsns(insns);
        for (auto iit : insns) {
            try{
                StackAnalysis::Height h = sa.findSP(b, iit.first);
                if (!h.isTop() && !h.isBottom()) {
                    int height = -h.height();
                    if (height > maxHeight) maxHeight = height;
                }
            }catch (exception &e) 
            {
            }
        }
	}
    idt->featPair["STACK_HEIGHT_MAX"] = maxHeight;
}
