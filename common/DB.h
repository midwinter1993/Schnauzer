#ifndef __COMMON_DB__
#define __COMMON_DB__

#include "extlibs/sqlite3.h"
#include <vector>
#include <string>

class DB {
public:
    typedef int (*Callback)(void *, int, char **, char **);

public:
    DB(const std::string &db_name);
    virtual ~DB ();
    int exec(const std::string &stmt, Callback callback=showQueryResult) const;
    int execOnly(const std::string &stmt) const;
    size_t tableSize(const std::string &table_name) const;
    bool tableExists(const std::string &table_name) const;
    std::vector<std::string> tableNames() const;

    int beginTX() const;
    int endTX() const;

private:
    static int showQueryResult(void *not_used, int argc, char **argv, char **col_name);

    void pragma() const;

private:
    sqlite3 *sqlite_db_;
};


#endif /* ifndef __COMMON_DB__ */
