#include "SrcInfoDB.h"
#include "auxiliary.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <limits>
#include <cassert>
#include <memory>
#include <cstring>
#include <stdarg.h>
#include <iostream>
#include <vector>


const uint64_t SrcInfoDB::UNVALID_MODULE_ID = std::numeric_limits<uint64_t>::max();

SrcInfoDB::SrcInfoDB(const std::string &db_path): DB(db_path) {
    createModuleTable();
    createFunctionTable();
    createInstructionTable();
}

SrcInfoDB::~SrcInfoDB () { }

// =============================================================
// Module Table
// =============================================================

void SrcInfoDB::createModuleTable() {
    const char *stmt_format = "CREATE TABLE IF NOT EXISTS modules (\
                               module_name text PRIMARY KEY);";

    exec(stmt_format);
}

int SrcInfoDB::insertModule(const std::string &module_name) {
    if (moduleExists(module_name)) {
        return 0;
    }
    const char* stmt_format = "INSERT INTO modules VALUES('%s')";
    std::string stmt = Aux::format(stmt_format, module_name.c_str());
    return exec(stmt.c_str());
}

uint64_t SrcInfoDB::findModuleId(const std::string &module_name) const {
    static uint64_t module_id = UNVALID_MODULE_ID;

    static auto callback = [](void*, int argc, char **argv, char **) {
        UNUSED(argc);
        assert(argc == 1);
        std::string value(argv[0] ? argv[0] : "NULL");
        module_id = std::stoull(value);
        return 0;
    };

    static const char *stmt_format = "SELECT rowid FROM modules WHERE module_name == '%s'";

    module_id = UNVALID_MODULE_ID;
    std::string stmt = Aux::format(stmt_format, module_name.c_str(), module_id);

    exec(stmt.c_str(), callback);
    return module_id;
}

bool SrcInfoDB::moduleExists(const std::string &module_name) const {
    return findModuleId(module_name) != UNVALID_MODULE_ID;
}

// =============================================================
// Instruction Table
// =============================================================

void SrcInfoDB::createFunctionTable() {
    const char *stmt_format = "CREATE TABLE IF NOT EXISTS functions ("
                              "func_id integer PRIMARY KEY,"
                              "func_info text NOT NULL"
                              ");";

    exec(stmt_format);

}

int SrcInfoDB::insertFunction(uint64_t func_id, const std::string &func_info) {
    if (functionExists(func_id)) {
        return updateFunction(func_id, func_info);
    }
    const char *stmt_format = "INSERT INTO functions VALUES(%lld, '%s')";
    std::string stmt = Aux::format(stmt_format, func_id, func_info.c_str());

    return exec(stmt.c_str());

}

int SrcInfoDB::updateFunction(uint64_t func_id, const std::string &func_info) {
    const char *stmt_format = "UPDATE functions SET func_info = '%s' \
                               WHERE func_id = %lld;";
    std::string stmt = Aux::format(stmt_format, func_info.c_str(), func_id);

    return exec(stmt.c_str());
}

std::string SrcInfoDB::findFunctionInfo(uint64_t func_id) const {
    static std::string func_info;

    static auto callback = [](void*, int argc, char **argv, char **) {
        UNUSED(argc);
        assert(argc == 1);
        std::string value(argv[0] ? argv[0] : "NULL");
        func_info = value;
        return 0;
    };
    static const char *stmt_format = "SELECT func_info FROM functions WHERE func_id == %lld";

    func_info = "NULL";
    std::string stmt = Aux::format(stmt_format, func_id);

    exec(stmt.c_str(), callback);

    return func_info;

}

bool SrcInfoDB::functionExists(uint64_t func_id) const {
    return findFunctionInfo(func_id) != "NULL";
}

// =============================================================
// Instruction Table
// =============================================================

void SrcInfoDB::createInstructionTable() {
    const char *stmt_format = "CREATE TABLE IF NOT EXISTS instructions ("
                              "inst_id integer PRIMARY KEY,"
                              "inst_info text NOT NULL,"
                              "inst_op text NOT NULL"
                              ");";

    exec(stmt_format);
}

int SrcInfoDB::insertInstruction(uint64_t inst_id, const std::string &inst_info, char op) {
    if (instructionExists(inst_id)) {
        return updateInstruction(inst_id, inst_info);
    }
    const char *stmt_format = "INSERT INTO instructions VALUES(%lld, '%s', '%c')";
    std::string stmt = Aux::format(stmt_format, inst_id, inst_info.c_str(), op);

    return exec(stmt.c_str());
}

int SrcInfoDB::updateInstruction(uint64_t inst_id, const std::string &inst_info) {
    const char *stmt_format = "UPDATE instructions SET inst_info = '%s' \
                               WHERE inst_id = %lld;";
    std::string stmt = Aux::format(stmt_format, inst_info.c_str(), inst_id);

    return exec(stmt.c_str());
}

std::string SrcInfoDB::findInstructionInfo(uint64_t inst_id) const {
    static std::string inst_info;

    static auto callback = [](void*, int argc, char **argv, char **) {
        UNUSED(argc);
        assert(argc == 1);
        std::string value(argv[0] ? argv[0] : "NULL");
        inst_info = value;
        return 0;
    };
    static const char *stmt_format = "SELECT inst_info FROM instructions WHERE inst_id == %lld";

    inst_info = "NULL";
    std::string stmt = Aux::format(stmt_format, inst_id);

    exec(stmt.c_str(), callback);

    return inst_info;
}

bool SrcInfoDB::instructionExists(uint64_t inst_id) const {
    return findInstructionInfo(inst_id) != "NULL";
}

/*
void SrcInfoDB::createRaceTable() {
    const char *stmt_format = "CREATE TABLE races (\
                               src_inst_id INTEGER NOT NULL,\
                               dst_inst_id INTEGER NOT NULL,\
                               is_harmful INTEGER DEFAULT 1,\
                               PRIMARY KEY (src_inst_id, dst_inst_id) );";
    exec(stmt_format);
}

std::vector<RaceInstPair> SrcInfoDB::getRace() const {
    static std::vector<RaceInstPair> races;

    static auto callback = [](void*, int argc, char **argv, char **) {
        assert(argc == 2);
        uint64_t src_inst_id = std::stoul(argv[0]);
        uint64_t dst_inst_id = std::stoul(argv[1]);
        races.emplace_back(src_inst_id, dst_inst_id);
        return 0;
    };
    static const char *stmt_format = "SELECT src_inst_id, dst_inst_id FROM races";

    races.clear();
    std::string stmt = format(stmt_format);

    exec(stmt.c_str(), callback);
    return races;
}

bool SrcInfoDB::raceExists(uint64_t src_inst_id, uint64_t dst_inst_id) const {
    static int cnt = 0;

    static auto callback = [](void*, int argc, char **argv, char **) {
        assert(argc == 1);
        cnt = std::stoul(argv[0]);
        return 0;
    };
    static const char *stmt_format = "SELECT count(*) FROM races WHERE\
                                      src_inst_id == %lld AND\
                                      dst_inst_id == %lld";

    cnt = 0;
    std::string stmt = format(stmt_format, src_inst_id, dst_inst_id);

    exec(stmt.c_str(), callback);
    return cnt == 1;
}

int SrcInfoDB::insertRace(uint64_t src_inst_id, uint64_t dst_inst_id) {
    if (src_inst_id > dst_inst_id) {
        std::swap(src_inst_id, dst_inst_id);
    }
    if (raceExists(src_inst_id, dst_inst_id)) {
        return 0;
    }
    const char *stmt_format = "INSERT INTO races VALUES(%lld, %lld)";
    std::string stmt = format(stmt_format, src_inst_id, dst_inst_id);
    return exec(stmt.c_str());
}
*/

// int main(int argc, char **argv) {
    // if (argc != 2) {
        // fprintf(stderr, "Usage: %s DATABASE\n", argv[0]);
        // return (1);
    // }
    // SrcInfoDB info_db(argv[1]);
    // info_db.insertModule("a.c", 0);
    // info_db.insertModule("b.c", 1);
    // info_db.insertModule("c.c", 2);
    // info_db.insertModule("d.c", 2000000000000);

    // // info_db.exec("SELECT * FROM modules");

    // // std::cout << info_db.findModuleId("c.c") << std::endl;
    // std::cout << info_db.tableSize("modules") << std::endl;
    // std::cout << info_db.tableSize("xx") << std::endl;

    // return 0;
// }
