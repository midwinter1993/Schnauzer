#include "YamlDumper.h"
#include "auxiliary.h"
#include "Console.h"
#include "Yaml.h"

#include <stdio.h>

namespace yaml {

void YamlDumper::dump(const NumberValue *v) const {
    Console::info("%ld", v->get());
}

void YamlDumper::dump(const BoolValue *v) const {
    Console::info("%s", v->get() ? "true" : "false");
}

void YamlDumper::dump(const StringValue *v) const {
    Console::info("%s", v->get().c_str());
}

void YamlDumper::dump(const VectorValue *v) const {
    Console::info("[");
    for (auto i: v->get()) {
        Console::info("%ld, ", i);
    }
    Console::info("]");
}

void YamlDumper::dump(const YamlValue *v) const {
    for (auto &pr: v->get()) {
        for (int i = 0; i < level_; ++i) {
            Console::info(" ");
        }
        Console::info("%s: ", pr.first.c_str());
        if (pr.second->getType() == Value::YAML) {
            Console::info("\n");
        }
        level_ += 2;
        pr.second->accept(*this);
        level_ -= 2;
        Console::info("\n");
    }
}

///////////////////////////////////////////////

YamlFileDumper::YamlFileDumper(const std::string &fpath): fpath_(fpath) {
    fd_ = fopen(fpath.c_str(), "w");
    if (!fd_) {
        Console::err("Open file:%s failure\n", fpath_.c_str());
        assert(0);
    }
}

void YamlFileDumper::dump(const NumberValue *v) const {
    fprintf(fd_, "%ld", v->get());
}

void YamlFileDumper::dump(const BoolValue *v) const {
    fprintf(fd_, "%s", v->get() ? "true" : "false");
}

void YamlFileDumper::dump(const StringValue *v) const {
    fprintf(fd_, "%s", v->get().c_str());
}

void YamlFileDumper::dump(const VectorValue *v) const {
    fprintf(fd_, "[");
    for (auto i: v->get()) {
        fprintf(fd_, "%ld, ", i);
    }
    fprintf(fd_, "]");
}

void YamlFileDumper::dump(const YamlValue *v) const {
    for (auto &pr: v->get()) {
        for (int i = 0; i < level_; ++i) {
            fprintf(fd_, " ");
        }
        fprintf(fd_, "%s: ", pr.first.c_str());
        if (pr.second->getType() == Value::YAML) {
            fprintf(fd_, "\n");
        }
        level_ += 2;
        pr.second->accept(*this);
        level_ -= 2;
        fprintf(fd_, "\n");
    }
}

}
