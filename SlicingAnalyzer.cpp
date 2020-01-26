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

static int SliceDepthAndDep(GraphPtr slice, unordered_map<string, double>& dataDep) {
    NodeIterator nbegin, nend;
    slice->exitNodes(nbegin, nend);
    map<Assignment::Ptr, int, AssignmentPtrValueComp> depth;
    queue<SliceNode::Ptr> q;
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr sn = boost::static_pointer_cast<SliceNode>(*nbegin);
        if (sn->assign()) {
            q.push(sn);
            depth[sn->assign()] = 1;
        }
    }
    int maxDepth = 1;
    char featStr[1024];
    while (!q.empty()) {
        SliceNode::Ptr cur = q.front();
        q.pop();
        cur->ins(nbegin, nend);
        for (; nbegin != nend; ++nbegin) {
            SliceNode::Ptr next = boost::static_pointer_cast<SliceNode>(*nbegin);
            if (!next->assign()) continue;
            if (depth.find(next->assign()) != depth.end()) {
                int nextDep = depth[cur->assign()] + 1;
                if (nextDep > maxDepth) maxDepth = nextDep;
                depth[next->assign()] = nextDep;
                q.push(next);
            }
            const vector<AbsRegion> &inputs = next->assign()->inputs();
            AbsRegion r2 = next->assign()->out();
            AbsRegion r3 = cur->assign()->out();
            for (auto iit = inputs.begin(); iit != inputs.end(); ++iit) {
                AbsRegion r1 = *iit;		
                snprintf(featStr, 1024, "DATA_CHAIN_%s_%s_%s", r1.format().c_str(), r2.format().c_str(), r3.format().c_str());
                string str1(featStr);
                dataDep[str1] += 1;
            }
        }
    }
    return maxDepth;
}

void SlicingAnalyzer::ProduceAFunction(InstanceDataType* idt) {
    ParseAPI::Function *f = idt->f;
    int totalSlices = 0;
    int maxNodes = 0;
    int maxDepth = 0;
    int totalNodes = 0;
    int sumDepth = 0;
    
    AssignmentConverter ac(true, true);

	for (auto b: f->blocks()) {
        try {
            Block::Insns insns;
            b->getInsns(insns);
            vector<Assignment::Ptr> assignList;
            set<Assignment::Ptr, AssignmentPtrValueComp> internal;
            for (auto iit : insns) {
                Instruction i = iit.second;
                Address addr = iit.first;
                vector<Assignment::Ptr> assigns;
                ac.convert(i, addr, f, b, assigns);
                assignList.insert(assignList.end(), assigns.begin(), assigns.end());
            }
            
            for (auto ait = assignList.rbegin(); ait != assignList.rend(); ++ait) {
                Assignment::Ptr assign = *ait;
                if (internal.find(assign) != internal.end()) continue;
                Slicer s(assign, b , f, &ac);
                Slicer::Predicates p;
                Graph::Ptr slice = s.backwardSlice(p);
                ++totalSlices;

                NodeIterator nbegin, nend;
                slice->allNodes(nbegin, nend);
                int realNodes = 0;
                for (; nbegin != nend; ++nbegin) {
                    SliceNode::Ptr sn = boost::static_pointer_cast<SliceNode>(*nbegin);
                    if (sn->assign()) {
                        realNodes++;
                        internal.insert(sn->assign());
                    }
                }
                int sliceDepth = SliceDepthAndDep(slice, idt->featPair);
                if (sliceDepth > maxDepth) maxDepth = sliceDepth;
                sumDepth += sliceDepth;
                totalNodes += realNodes;
                if (maxNodes < realNodes) maxNodes = realNodes;
            }
	    } catch (exception &e)
	    {
	    }
	}
    idt->featPair["TOTAL_SLICES"] = totalSlices;
    idt->featPair["MAX_SLICE_NODE_COUNT"] = maxNodes;
    if (totalSlices > 0)
        idt->featPair["AVG_SLICE_NODE_COUNT"] = 1.0 * totalNodes / totalSlices;
    idt->featPair["MAX_SLICE_DEPTH"] = maxDepth;
    if (totalSlices > 0)
        idt->featPair["AVG_SLICE_DEPTH"] = 1.0 * sumDepth / totalSlices;
}
