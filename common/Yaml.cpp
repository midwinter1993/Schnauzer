#include "Yaml.h"
#include "auxiliary.h"
#include <fstream>
#include <regex>

namespace yaml {

struct FileReader {
    std::ifstream &fin_;
    std::ifstream::pos_type old_pos_;

    FileReader(std::ifstream &fin): fin_(fin) { }

    bool readLine(std::string &l) {
        old_pos_ = fin_.tellg();
        if (std::getline(fin_, l)) {
            return true;
        }
        return false;
    }

    void pushBackLine() {
        fin_.seekg(old_pos_);
    }
};

std::vector<Value::number_t> parseVector(const std::string &v) {
    std::vector<Value::number_t> vec;

    std::regex number_rgx("\\d+");

    auto it = std::sregex_iterator(v.begin(), v.end(), number_rgx);
    auto end = std::sregex_iterator();
    for (; it != end; ++it) {
        vec.push_back(std::stoi(it->str()));
    }
    return vec;
}

const Value* buildValue(const std::string &v) {
    if (Aux::isNumber(v)) {
        return new NumberValue(std::stoll(v));
    } else if (v.size() > 0 && v.front() == '[') {
        return new VectorValue(parseVector(v));
    } else if (v == "true") {
        return new BoolValue(true);
    } else if (v == "false") {
        return new BoolValue(false);
    } else {
        return new StringValue(v);
    }
}

static bool is_startswith_space(const std::string &s, size_t level) {
    if (s.size() < level) return false;
    for (size_t i = 0; i < level; ++i) {
        if (!std::isspace(s[i]))
            return false;
    }
    return true;
}

const YamlValue* parseImpl(FileReader &fr, size_t level) {
    YamlValue *yaml = new YamlValue();

    std::string line;
    while (fr.readLine(line)) {
        if (!line.size()) {
            continue;
        }
        if (!is_startswith_space(line, level)) {
            if (level == 0) {
                assert("Top level parse failure" && false);
            } else {
                fr.pushBackLine();
                return yaml;
            }
        }

        {
            // key: value
            std::regex rgx("([_a-zA-Z]+):\\s(.*)");
            std::smatch matches;
            if (std::regex_search(line, matches, rgx)) {
                assert(matches.size() == 3);
                auto &item_key = matches[1];
                auto &item_value = matches[2];
                yaml->addValue(item_key, buildValue(item_value));
                continue;
            }
        }

        {
            // subyaml
            std::regex rgx("([_a-zA-Z]+):\\s*");
            std::smatch matches;
            if (std::regex_search(line, matches, rgx)) {
                assert(matches.size() == 2);
                auto subyaml = parseImpl(fr, level+2);
                yaml->addValue(matches[1], subyaml);
                continue;
            }
        }
        assert("Error: Unknown line" && false);
    }

    return yaml;
}

const YamlValue* parse(const std::string &fpath) {
    std::ifstream fin;
    Aux::openIstream(fin, fpath);

    FileReader fr(fin);
    return parseImpl(fr, 0);
}

}
// int main() {
    // auto yaml = Yaml::parse("/home/dongjie/Documents/Oops/logs/test/fucking_config.yaml");
    // Yaml::dump(yaml);
    // return 0;
// }
