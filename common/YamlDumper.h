#ifndef __COMMON_YAML_DUMPER_H__
#define __COMMON_YAML_DUMPER_H__

#include <string>

namespace yaml {

class YamlValue;
class BoolValue;
class NumberValue;
class StringValue;
class VectorValue;

class YamlDumper {
public:
    virtual void dump(const YamlValue *v) const;

    virtual void dump(const NumberValue *v) const;

    virtual void dump(const BoolValue *v) const;

    virtual void dump(const StringValue *v) const;

    virtual void dump(const VectorValue *v) const;

protected:
    mutable int level_ = 0;
};

class YamlFileDumper: public YamlDumper {
public:
    YamlFileDumper(const std::string &fpath);

    void dump(const YamlValue *v) const;

    void dump(const NumberValue *v) const;

    void dump(const BoolValue *v) const;

    void dump(const StringValue *v) const;

    void dump(const VectorValue *v) const;

private:
    const std::string fpath_;
    FILE *fd_;
};

}
#endif /* ifndef __COMMON_YAML_DUMPER_H__ */
