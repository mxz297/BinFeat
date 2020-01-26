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

void GraphletAnalyzer::ProduceAFunction(InstanceDataType* idt) {
    ParseAPI::Function *f = idt->f;
	StackAnalysis sa(f);	
    for (auto b : f->blocks()) {
	    StackAnalysis::SummaryFunc blockStackSummary;
	    StackAnalysis::AbslocState blockStackIn;
	    StackAnalysis::AbslocState blockStackOut;
	    Absloc sp = Absloc::makeSP(f->isrc()->getArch());
	    try{
            if (sa.getBlockEffects(f, b, blockStackSummary) && sa.getBlockState(f, b, blockStackIn, blockStackOut)) {
                StackAnalysis::DefHeightSet &defSet = blockStackIn[sp];
                StackAnalysis::Height entrySPHeight = defSet.getHeightSet();
                if (entrySPHeight.isTop()) {
                    AddAndPrintFeat("BLOCK_SP_ENTRY_TOP", 1);
                }
                if (entrySPHeight.isBottom()) {
                    AddAndPrintFeat("BLOCK_SP_ENTRY_BOTTOM", 1);
                }
                if (!entrySPHeight.isTop() && !entrySPHeight.isBottom()) {
                    AddAndPrintFeat("BLOCK_SP_ENTRY_DEF_BOOL", 1);
                }
                StackAnalysis::Height exitSPHeight = blockStackOut[sp].getHeightSet();
                
                if (exitSPHeight.isTop()) {
                    AddAndPrintFeat("BLOCK_SP_EXIT_TOP", 1);
                }
                if (exitSPHeight.isBottom()) {
                    AddAndPrintFeat("BLOCK_SP_EXIT_BOTTOM", 1);
                }
                if (!exitSPHeight.isTop() && !exitSPHeight.isBottom()) {
                    AddAndPrintFeat("BLOCK_SP_EXIT_DEF_BOOL", 1);
                }
                if (blockStackSummary.accumFuncs.find(sp) != blockStackSummary.accumFuncs.end()) {
                    StackAnalysis::TransferFunc &trans = blockStackSummary.accumFuncs[sp];
                    AddAndPrintFeat("BLOCK_SP_TRANSFER_FUNC_BOOL", 1);
                    if (trans.isTop()) {
                        AddAndPrintFeat("BLOCK_SP_TRANSFER_TOP", 1);
                    }
                    if (trans.isBottom()) {
                        AddAndPrintFeat("BLOCK_SP_TRANSFER_BOTTOM", 1);
                    }
                    
                    bool value = !trans.isTop() && !trans.isBottom();
                    if (value && trans.isAbs()) {
                        AddAndPrintFeat("BLOCK_SP_TRANSFER_ABS_BOOL", 1);
                        if (trans.abs < 0) {
                            AddAndPrintFeat("BLOCK_SP_TRANSFER_ABS_NEGA_BOOL", 1);
                        } else {
                            AddAndPrintFeat("BLOCK_SP_TRANSFER_ABS_POSI_BOOL", 1);
                        }
                    }
                    if (value && trans.isDelta()) {
                        AddAndPrintFeat("BLOCK_SP_TRANSFER_DELTA_BOOL", 1);
                        if (trans.delta < 0) {
                            AddAndPrintFeat("BLOCK_SP_TRANSFER_DELTA_NEGA_BOOL", 1);
                        } else {
                            AddAndPrintFeat("BLOCK_SP_TRANSFER_DELTA_POSI_BOOL", 1);
                        }
                    }
                }
            }
        } catch (exception &e) 
        {
        }
	}
}
