#include "Instrument.h"

#include "common/Config.h"
#include "common/auxiliary.h"
#include "common/SrcInfoDB.h"

uint64_t Instrument::computeUniqueId(uint64_t ModId, uint64_t Id) {
    uint64_t NewId = 0;

    // Sign bit must be 0
    assert(ModId <= (1LL << 20) - 1);
    // assert(ModId <= (1LL << 21) - 1);
    assert(Id <= (1LL << 45) - 1);

    NewId |= ModId << 44;
    NewId |= Id;

    return NewId;
}

uint64_t Instrument::computeModId(uint64_t id) {
    uint64_t mask = 0xFFFFF;
    return (id >> 44) & mask;
}

uint64_t Instrument::computeInstId(uint64_t id) {
    uint64_t mask = 0xFFFFF;
    return id & mask;
}

void Instrument::collectSrcInfo(Module &M) {
    uint64_t InstId = 0;
    uint64_t FuncId = 0;

    SrcInfoDB src_info_db(Config::getConfig().srcInfoDbPath());
    src_info_db.beginTX();

    for (auto &F : M) {
        if (!F.isDeclaration()) {
            FuncIdMap_[&F] = FuncId;

            // auto id = computeUniqueId(ModId_, FuncId);
            // src_info_db.insertFunction(id, IRHelper::getSrcLineInfo(&F));
        }

        for (auto &B : F) {
            for (auto &I : B) {
                Instruction *inst = &I;
                InstIdMap_[inst] = InstId;

                // We just collect Load, Store and Call instructions.
                if (dyn_cast<LoadInst>(inst)) {
                    auto id = computeUniqueId(ModId_, InstId);
                    src_info_db.insertInstruction(id, IRHelper::getSrcLineInfo(inst), 'r');
                } else if (dyn_cast<StoreInst>(inst)) {
                    auto id = computeUniqueId(ModId_, InstId);
                    src_info_db.insertInstruction(id, IRHelper::getSrcLineInfo(inst), 'w');
                } else if (dyn_cast<CallInst>(inst)) {
                    auto id = computeUniqueId(ModId_, InstId);
                    src_info_db.insertInstruction(id, IRHelper::getSrcLineInfo(inst), 'c');
                }

                InstId += 1;
            }
        }
        FuncId += 1;
    }

    src_info_db.endTX();
    errs() << "# TOTAL INSTRUCTIONS: " << InstId << "\n";
}

ConstantInt *Instrument::getFuncId(Function *F) {
    auto it = FuncIdMap_.find(F);
    if (it == FuncIdMap_.end()) {
        return nullptr;
    }
    auto id = computeUniqueId(ModId_, it->second);
    return IRHelper_.getConstantInt(id);
}

ConstantInt *Instrument::getInstId(Instruction *I) {
    auto it = InstIdMap_.find(I);
    if (it == InstIdMap_.end()) {
        errs() << IRHelper_.getSrcLineInfo(I) << "\n";
    }
    assert(it != InstIdMap_.end());
    auto id = computeUniqueId(ModId_, it->second);
    return IRHelper_.getConstantInt(id);
}
