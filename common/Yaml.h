#ifndef __COMMON_YAML_H__
#define __COMMON_YAML_H__

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include "YamlDumper.h"

namespace yaml {

class YamlValue;
class BoolValue;
class NumberValue;
class StringValue;
class VectorValue;

class YamlDumper;

class Value {
public:
    enum ValueType { NUMBER, BOOL,  STRING, VECTOR, YAML };

    typedef int64_t number_t;

    Value(ValueType ty): type_(ty) { }

    ValueType getType() const { return type_; }

    NumberValue* asNumberValue() const {
        assert (type_ == NUMBER);
        return (NumberValue*)this;
    }

    BoolValue* asBoolValue() const {
        assert (type_ == BOOL);
        return (BoolValue*)this;
    }

    StringValue* asStringValue() const {
        assert (type_ == STRING);
        return (StringValue*)this;
    }

    VectorValue* asVectorValue() const {
        assert(type_ == VECTOR);
        return (VectorValue*)this;
    }

    YamlValue* asYamlValue() const {
        assert (type_ == YAML);
        return (YamlValue*)this;
    }

    virtual void accept(const YamlDumper&) const = 0;
private:
    ValueType type_;
};

class NumberValue: public Value {
public:
    NumberValue(number_t v): Value(Value::NUMBER), value_(v) { }
    number_t get() const {
        return value_;
    }

    void set(number_t v) {
        value_ = v;
    }

    void accept(const YamlDumper &d) const override {
        d.dump(this);
    }
private:
    number_t value_;
};

class BoolValue: public Value {
public:
    BoolValue(bool v): Value(Value::BOOL), value_(v) { }
    bool get() const {
        return value_;
    }

    void set(bool v) {
        value_ = v;
    }

    void accept(const YamlDumper &d) const override {
        d.dump(this);
    }
private:
    bool value_;
};

class StringValue: public Value {
public:
    StringValue(const std::string &v): Value(Value::STRING), value_(v) { }
    std::string get() const {
        return value_;
    }

    void set(const std::string &v) {
        value_ = v;
    }

    void accept(const YamlDumper &d) const override {
        d.dump(this);
    }
private:
    std::string value_;
};

class VectorValue: public Value {
public:
    VectorValue(const std::vector<number_t> &v): Value(Value::VECTOR), value_(v) { }

    std::vector<number_t> get() const {
        return value_;
    }

    void set(const std::vector<number_t> &v) {
        value_ = v;
    }

    void accept(const YamlDumper &d) const override {
        d.dump(this);
    }
private:
    std::vector<number_t> value_;
};

class YamlValue: public Value {
public:
    YamlValue(): Value(Value::YAML) { }

    number_t getNumber(const std::string &k, bool check=false) const {
        number_t n = lookup(k)->asNumberValue()->get();
        if (check) {
            assert(n < (number_t)std::numeric_limits<int>::max());
        }
        return n;
    }

    void setNumber(const std::string &k, number_t v) {
        lookup(k)->asNumberValue()->set(v);
    }

    bool getBool(const std::string &k) const {
        return lookup(k)->asBoolValue()->get();
    }

    void setBool(const std::string &k, bool v) {
        lookup(k)->asBoolValue()->set(v);
    }

    std::string getString(const std::string &k) const {
        return lookup(k)->asStringValue()->get();
    }

    void setString(const std::string &k, const std::string &v) {
        lookup(k)->asStringValue()->set(v);
    }

    std::vector<number_t> getVector(const std::string &k) const {
        return lookup(k)->asVectorValue()->get();
    }

    void setVector(const std::string &k, const std::vector<number_t> &v) {
        lookup(k)->asVectorValue()->set(v);
    }

    YamlValue* getYaml(const std::string &k) const {
        return lookup(k)->asYamlValue();
    }

    bool addValue(const std::string &k, const Value *v) {
        table_[k] = v;
        return true;
    }

    bool addNumber(const std::string &k, number_t v) {
        return add<NumberValue, number_t>(k, v);
    }

    bool addBool(const std::string &k, bool v) {
        return add<BoolValue, bool>(k, v);
    }

    bool addString(const std::string &k, const std::string &v) {
        return add<StringValue, std::string>(k, v);
    }

    bool addVector(const std::string &k, const std::vector<number_t> &v) {
        return add<VectorValue, std::vector<number_t>>(k, v);
    }

    bool addYaml(const std::string &k, const YamlValue *v) {
        table_[k] = v;
        return true;
    }

    std::map<std::string, const Value*> get() const {
        return table_;
    }

    void accept(const YamlDumper &d) const override {
        d.dump(this);
    }

    void dump() const {
        YamlDumper d;
        accept(d);
    }

    void save(const std::string &fpath) const {
        YamlFileDumper d(fpath);
        accept(d);
    }

private:
    const Value* lookup(const std::string & k) const {
        auto it = table_.find(k);
        if (it == table_.end()) {
            std::cerr << k << " FUCK here\n";
            assert(0);
        }
        return it->second;
    }

    template<typename T1, typename T2>
    bool add(const std::string &k, const T2 &v) {
        table_[k] = new T1(v);
        return true;
    }

    std::map<std::string, const Value*> table_;
};

const YamlValue* parse(const std::string &fpath);

}

#endif /* ifndef __COMMON_YAML_H__ */
