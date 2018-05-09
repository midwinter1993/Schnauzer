#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#include <atomic>
#include <fstream>
#include <mutex>
#include <map>
#include <string>
#include <vector>
#include "Yaml.h"

using namespace yaml;

using Subconfig = YamlValue;

class Config {
public:

    // static const Config& getConfig();
    static const Config& getConfig(bool dump=true);
    static const Subconfig* getInfoConfig();
    static const Subconfig* getScsConfig();
    static const Subconfig* getPctConfig();
    static const Subconfig* getLoggerConfig();
    static const Subconfig* getRuntimeConfig();
    static const Subconfig* getSupervisorConfig();

    void dumpConfig() const;

    // trace files
    std::string tracePath() const;

    // check target race file
    std::string targetRacePath() const;

    // Source code info files
    // std::string srcInfoInstructionInfoPath() const;
    // std::string srcInfoModuleInfoPath() const;
    std::string srcInfoDbPath() const;
    std::string wrapFuncDefPath() const;
    // Detection result files
    std::string summaryPath() const;

private:
    const YamlValue* getYaml() const;

    bool checkConfig() const;

    static std::atomic<const Config*> instance_;
    static std::mutex mutex_;
    static const Config* loadConfig();

    const YamlValue *yaml_;
};

#endif /* ifndef __COMMON_CONFIG_H__ */
