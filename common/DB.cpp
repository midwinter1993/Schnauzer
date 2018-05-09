#include "DB.h"
#include "extlibs/sqlite3.h"
#include "auxiliary.h"
#include <stdio.h>
#include <string>
#include <cassert>
#include <memory>
#include <cstring>
#include <stdarg.h>
#include <vector>

#define DO_CORRUPT exit(1)

int DB::showQueryResult(void *, int argc, char **argv, char **col_name) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

DB::DB(const std::string &db_name) {
    int ret = sqlite3_open(db_name.c_str(), &sqlite_db_);
    if (ret) {
        fprintf(stderr, "Can't open database: %s for %s\n",
                        sqlite3_errmsg(sqlite_db_),
                        db_name.c_str());
        sqlite3_close(sqlite_db_);
        DO_CORRUPT;
    }
    pragma();
}

DB::~DB() {
    if (sqlite_db_) {
        sqlite3_close(sqlite_db_);
    }
}

void DB::pragma() const {
    execOnly("PRAGMA synchronous=OFF");
    execOnly("PRAGMA count_changes=OFF");
    execOnly("PRAGMA journal_mode=MEMORY");
    execOnly("PRAGMA temp_store=MEMORY");
}

bool DB::tableExists(const std::string &table_name) const {
    static const char *stmt_format = "SELECT count(type) FROM sqlite_master WHERE type='table' AND name='%s'";
    static uint64_t table_cnt = 0;

    static auto callback = [](void*, int argc, char **argv, char **) {
        assert(argc == 1);
        table_cnt = std::stoull(argv[0]);
        return 0;
    };

    std::string stmt = Aux::format(stmt_format, table_name.c_str());

    exec(stmt.c_str(), callback);
    return table_cnt == 1;
}

std::vector<std::string> DB::tableNames() const {
    static const char *stmt_format = "SELECT name FROM sqlite_master where type='table'";
    static std::vector<std::string> table_names;

    static auto callback = [](void*, int argc, char **argv, char **) {
        for (int i = 0; i < argc; ++i) {
            table_names.push_back(argv[i]);
        }
        return 0;
    };

    table_names.clear();

    exec(stmt_format, callback);
    return table_names;
}

size_t DB::tableSize(const std::string &table_name) const {
    if (!tableExists(table_name)) {
        return 0;
    }

    static const char *stmt_format = "select count(*) FROM %s";
    static uint64_t table_sz = 0;

    static auto callback = [](void*, int argc, char **argv, char **) {
        assert(argc == 1);
        table_sz = std::stoul(argv[0]);
        return 0;
    };

    table_sz = 0;
    std::string stmt = Aux::format(stmt_format, table_name.c_str());

    exec(stmt.c_str(), callback);
    return table_sz;

}

int DB::exec(const std::string &stmt, Callback callback) const {
    assert(sqlite_db_);
    char *zErrMsg = NULL;
    int ret = sqlite3_exec(sqlite_db_, stmt.c_str(), callback, 0, &zErrMsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        DO_CORRUPT;
    }
    return ret;
}

int DB::beginTX() const {
    return exec("BEGIN TRANSACTION", NULL);
}

int DB::endTX() const {
    return exec("END TRANSACTION", NULL);
}

int DB::execOnly(const std::string &stmt) const {
    return exec(stmt, NULL);
}
