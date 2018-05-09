#include "auxiliary.h"
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include "Config.h"
#include "Console.h"


namespace Aux {

std::string env(const std::string &env_name) {
    char *ptr = getenv(env_name.c_str());
    if (!ptr) {
        Console::redErr("Get ENV error for %s\n", env_name.c_str());
        exit(0);
    }
    return ptr;
}

std::string envProjDir() {
    std::string proj_dir = env("PROJ_DIR");
    if (proj_dir.back() == '/') {
        proj_dir.pop_back();
    }
    return proj_dir;
}

bool isRoot() {
    uid_t uid=getuid(), euid=geteuid();
    if (uid != 0 || uid!=euid) {
        return false;
    } else {
        return true;
    }
}

bool makeDir(const char *dpath) {
    // file exists
    if (access(dpath, F_OK) != -1)
        return true;
    // file doesn't exist
    if (mkdir(dpath, 0775) == -1) {
        Console::redErr("Make dir error: %s [%s]\n", strerror(errno), dpath);
        exit(1);
    }
    return true;
}

void makeDirRecursive(const char *dir) {
    assert(dir && dir[0] == '/');

    char tmp[PATH_MAX];
    size_t len = snprintf(tmp, sizeof(tmp), "%s", dir);
    assert(len < PATH_MAX);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            makeDir(tmp);
            *p = '/';
        }
    }
    makeDir(tmp);
}

std::string dirname(const std::string &path) {
    return path.substr(0, path.find_last_of('/'));
}

std::string basename(const std::string &path) {
    return path.substr(path.find_last_of('/') + 1);
}

std::string rmExt(const std::string &basename) {
    return basename.substr(0, basename.find_last_of('.'));
}

std::string basenameWithoutExt(const std::string &path) {
    return rmExt(basename(path));
}

void openOstream(std::ofstream &fout, const std::string &fpath,
                  std::ios_base::openmode m, bool create_dir) {
    if (create_dir) {
        // To create dir, path must be absolute.
        assert(fpath[0] == '/');
        makeDirRecursive(dirname(fpath).c_str());
    }
    fout.open(fpath, m);
    if (!fout) {
        Console::redErr("OPEN [ OUT ] file error for %s\n", fpath.c_str());
        exit(1);
    }
}

void openIstream(std::ifstream &fin, const std::string &fpath,
                  std::ios_base::openmode m) {
    fin.open(fpath, m);
    if (!fin) {
        Console::redErr("OPEN [ IN ] file error for %s\n", fpath.c_str());
        exit(1);
    }
}

bool fileExists(const std::string &fpath) {
    std::ifstream infile(fpath);
    return infile.good();
}

size_t fileSize(const std::string &fpath) {
    struct stat st;
    stat(fpath.c_str(), &st);
    return st.st_size;
}

std::string pathJoin(const char *path_1, ...) {
    std::string tot_path(path_1);

    auto clear_path_postfix = [&tot_path]() {
        while (tot_path.size() && tot_path.back() == '/') {
            tot_path.pop_back();
        }
    };

    clear_path_postfix();

    va_list vlist;
    va_start(vlist, path_1);
    while (1) {
        const char *path = va_arg(vlist, const char *);
        if (!path)
            break;

        tot_path.push_back('/');
        tot_path.append(path);

        clear_path_postfix();
    }
    va_end(vlist);

    return tot_path;
}
//
// Replace substring
// http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
//
void replaceAll(std::string &str, const std::string &from,
                 const std::string &to) {
    if (from.empty())
        return;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like
                                   // replacing 'x' with 'yx'
    }
}

void replaceLast(std::string &str, const std::string &from,
                  const std::string &to) {
    if (from.empty())
        return;

    size_t start_pos = str.rfind(from);
    if (start_pos != std::string::npos) {
        str.replace(start_pos, from.length(), to);
    }
}

bool startswith(const std::string &s, const std::string &start) {
    if (start.size() > s.size())
        return false;
    return s.find(start) == 0;
}

bool endswith(const std::string &s, const std::string &ending) {
    if (ending.size() > s.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), s.rbegin());
}

bool contain(const std::string &s, const std::string &subs) {
    return s.find(subs) != std::string::npos;
}

bool isNumber(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string format(const char *fmt, ...) {
    const int buf_sz = 1024;
    char *buf = new char[buf_sz];

    va_list args;
    va_start(args, fmt);
    auto l = vsnprintf(buf, buf_sz, fmt, args);
    UNUSED(l);
    va_end(args);

    assert(0 <= l && l < buf_sz - 1 && "Format failure");

    auto s = std::string(buf);
    delete[] buf;
    return s;
}

std::string getCurrentTime() {
    time_t now = time(0);
    char buf[80];

    struct tm *tstruct = localtime(&now);

    snprintf(buf, sizeof(buf), "%d-%02d-%02d_%02d:%02d:%02d",
                                tstruct->tm_year + 1900,
                                tstruct->tm_mon + 1,
                                tstruct->tm_mday,
                                tstruct->tm_hour,
                                tstruct->tm_min,
                                tstruct->tm_sec);
    return std::string(buf);
}



extern "C" char *__progname;
std::string getProgName() { return __progname; }

void fillUniqueValue(int *arr, size_t n, int beg, int end) {
    // Random seed
    std::random_device rd;
    // Initialize Mersenne Twister pseudo-random number generator
    std::mt19937 gen(rd());
    // Generate pseudo-random numbers
    // uniformly distributed in range (beg, end)
    std::uniform_int_distribution<> dist(beg, end);

    size_t i = 0;
    std::set<int> unique_nums;

    while (i < n) {
        int x = dist(gen);
        if (!unique_nums.count(x)) {
            unique_nums.insert(x);
            arr[i] = x;
            i += 1;
        }
    }
}

}
