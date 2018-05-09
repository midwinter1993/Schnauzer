#include "Instrument.h"
#include <regex>
#include "common/Config.h"
#include "common/auxiliary.h"
#include "common/SrcInfoDB.h"

uint64_t Instrument::loadModuleId(const std::string &ModName) {
    SrcInfoDB src_info_db(Config::getConfig().srcInfoDbPath());

    if (!src_info_db.moduleExists(ModName)){
        src_info_db.insertModule(ModName);
    }

    uint64_t mod_id = src_info_db.findModuleId(ModName);

    assert("Find module name failure\n" && mod_id != SrcInfoDB::UNVALID_MODULE_ID);
    return mod_id;
}

static std::string parse_func_name(const std::string &func_type) {
    std::regex rgx("[\\w\\*]+ (\\w+)\\(.*\\)");
    std::smatch matches;
    if (std::regex_search(func_type, matches, rgx)) {
        return matches[1].str();
    }
    std::cerr << func_type << "\n";
    assert(false);
    return "Match not found";
}

void Instrument::loadWrapFuncDefs() {
    if (WrapFuncNames_.size()) {
        return;
    }
    std::ifstream fin;
    Aux::openIstream(fin, Config::getConfig().wrapFuncDefPath());
    std::string line;
    while (std::getline(fin, line)) {
        WrapFuncNames_.insert(parse_func_name(line));
    }
    assert(WrapFuncNames_.size());

    WrapFuncNames_.insert("pthread_create");
    WrapFuncNames_.insert("pthread_join");
    WrapFuncNames_.insert("pthread_exit");
    WrapFuncNames_.insert("free");
    WrapFuncNames_.insert("malloc");
    WrapFuncNames_.insert("realloc");
    WrapFuncNames_.insert("calloc");
    WrapFuncNames_.insert("pthread_mutex_destroy");
}
