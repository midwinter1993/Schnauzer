#ifndef __COMMON_SRC_INFO_DB__
#define __COMMON_SRC_INFO_DB__

#include "DB.h"

class SrcInfoDB: public DB {
public:
    static const uint64_t UNVALID_MODULE_ID;

    SrcInfoDB(const std::string &db_path);
private:
    void createModuleTable();
    void createFunctionTable();
    void createInstructionTable();

public:

    virtual ~SrcInfoDB ();

    int insertModule(const std::string &module_name);
    uint64_t findModuleId(const std::string &module_name) const;
    bool moduleExists(const std::string &module_name) const;

    int insertFunction(uint64_t func_id, const std::string &func_info);
    int updateFunction(uint64_t func_id, const std::string &func_info);
    std::string findFunctionInfo(uint64_t func_id) const;
    bool functionExists(uint64_t func_id) const;

    int insertInstruction(uint64_t inst_id, const std::string &inst_info, char op);
    int updateInstruction(uint64_t inst_id, const std::string &inst_info);
    std::string findInstructionInfo(uint64_t inst_id) const;
    bool instructionExists(uint64_t inst_id) const;
};


#endif /* ifndef __COMMON_SRC_INFO_DB__ */
