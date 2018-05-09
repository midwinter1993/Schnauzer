#ifndef __COMMON_AUXILIARY_H__
#define __COMMON_AUXILIARY_H__

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <set>

namespace Aux {

#define UNUSED(v) ((void)v)

//
// Environment operation
//
std::string env(const std::string &env_name);

// std::string envProjDir();

bool isRoot();

//
// File operation
//
bool makeDir(const char *dpath);

std::string basename(const std::string &pathname);

std::string basenameWithoutExt(const std::string &pathname);

// std::string rmExt(const std::string &basename);

void openOstream(std::ofstream &fout, const std::string &fpath,
                  std::ios_base::openmode m = std::ios_base::out,
                  bool create_dir = false);

void openIstream(std::ifstream &fin, const std::string &fpath,
                  std::ios_base::openmode m = std::ios_base::in);

bool fileExists(const std::string &fpath);

size_t fileSize(const std::string &fpath);

//
// NOTE: nullptr should be the last argument
//
std::string pathJoin(const char *path_1, ...);

//
// String operation
//
void replaceAll(std::string &str, const std::string &from,
                 const std::string &to);

void replaceLast(std::string &str, const std::string &from,
                  const std::string &to);

bool startswith(const std::string &s, const std::string &start);

bool endswith(const std::string &s, const std::string &ending);

bool contain(const std::string &s, const std::string &subs);

bool isNumber(const std::string& s);

std::string format(const char *fmt, ...);

//
// Get program info
//
std::string getCurrentTime();

std::string getProgName();

template<typename T>
void fillUniqueValue(T *arr, size_t n, T beg, T end) {
    // Random seed
    std::random_device rd;
    // Initialize Mersenne Twister pseudo-random number generator
    std::mt19937 gen(rd());
    // Generate pseudo-random numbers
    // uniformly distributed in range (beg, end)
    std::uniform_int_distribution<> dist(beg, end);

    size_t i = 0;
    std::set<T> unique_nums;

    while (i < n) {
        T x = dist(gen);
        if (!unique_nums.count(x)) {
            unique_nums.insert(x);
            arr[i] = x;
            i += 1;
        }
    }
    std::sort(arr, arr + n);
}

}

#endif /* ifndef __AUXILIARY_H__ */
