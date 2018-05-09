#include "Config.h"
#include <atomic>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include "auxiliary.h"

std::atomic<const Config*> Config::instance_(nullptr);
std::mutex Config::mutex_;


const Subconfig* Config::getInfoConfig() {
    return getConfig().getYaml()->getYaml("INFO");
}

const Subconfig* Config::getScsConfig() {
    return getConfig().getYaml()->getYaml("SCS");
}

const Subconfig* Config::getPctConfig() {
    return getConfig().getYaml()->getYaml("PCT");
}

const Subconfig* Config::getLoggerConfig() {
    return getConfig().getYaml()->getYaml("LOGGER");
}

const Subconfig* Config::getRuntimeConfig() {
    return getConfig().getYaml()->getYaml("RUNTIME");
}

const Subconfig* Config::getSupervisorConfig() {
    return getConfig().getYaml()->getYaml("SUPERVISOR");
}

const Config& Config::getConfig(bool dump) {
    const Config* tmp = instance_.load();
    if (!tmp) {
        std::lock_guard<std::mutex> lk(mutex_);
        tmp = instance_.load();
        if (!tmp) {
            tmp = loadConfig();
            instance_.store(tmp);

            assert(tmp->checkConfig());

            if (dump) {
                // tmp->dumpConfig();
            }
        }
    }
    return *tmp;
}

const Config* Config::loadConfig() {
    std::string config_path = Aux::env("LN_CONFIG");
    // config_path = "/home/dongjie/Documents/Oops/logs/test/LN.yaml";
    assert(Aux::endswith(config_path, "LN.yaml"));
    // assert(Aux::contain(config_path, Aux::getProgName()));

    Config *config = new Config();
    config->yaml_ = yaml::parse(config_path);
    return config;
}

const YamlValue* Config::getYaml() const {
    return yaml_;
}

bool Config::checkConfig() const {
    if (getLoggerConfig()->getBool("ENABLE")) {
        return getRuntimeConfig()->getBool("REUSE_TID") == false &&
               getRuntimeConfig()->getBool("FREE_MEM") == false;
    }
    return true;
}

std::string Config::targetRacePath() const {
    auto dir = Aux::pathJoin(getInfoConfig()->getString("LOG_DIR").c_str(),
                             getInfoConfig()->getString("NAME").c_str(),
                             "check",
                             nullptr);
    std::string target_file = "target.race";
    std::string path = Aux::pathJoin(dir.c_str(), target_file.c_str(), nullptr);
    return path;
}

std::string Config::tracePath() const {
    std::string path;
    auto trace_dir = Aux::pathJoin(getInfoConfig()->getString("LOG_DIR").c_str(),
                                   getInfoConfig()->getString("NAME").c_str(),
                                   "detect",
                                   nullptr);
    do {
        std::string trace_name =
            Aux::format("%s_%s_%s.trace",
                        getInfoConfig()->getString("NAME").c_str(),
                        Aux::getCurrentTime().c_str(),
                        getRuntimeConfig()->getString("SCHEDULER").c_str());
        path = Aux::pathJoin(trace_dir.c_str(), trace_name.c_str(), nullptr);
    } while (Aux::fileExists(path));
    return path;
}

std::string Config::wrapFuncDefPath() const {
    // return Aux::format("%s/wrap_func.def", Aux::envProjDir().c_str());
    return Aux::format("%s/wrap_func.def",
                       getInfoConfig()->getString("PROJ_DIR").c_str());
}

std::string Config::srcInfoDbPath() const {
    return Aux::pathJoin(getInfoConfig()->getString("LOG_DIR").c_str(),
                         getInfoConfig()->getString("NAME").c_str(),
                         "srcinfo",
                         "info.db",
                         nullptr);
}

std::string Config::summaryPath() const {
    auto summary_dir = Aux::pathJoin(getInfoConfig()->getString("LOG_DIR").c_str(),
                         getInfoConfig()->getString("NAME").c_str(),
                         "summary",
                         nullptr);
    std::string name = Aux::format("%s_%s.summary",
                                   getInfoConfig()->getString("NAME").c_str(),
                                   getRuntimeConfig()->getString("SCHEDULER").c_str());
    std::string path = Aux::pathJoin(summary_dir.c_str(), name.c_str(), nullptr);
    return path;
}

void Config::dumpConfig() const {
    yaml_->dump();
}
