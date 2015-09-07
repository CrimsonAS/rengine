#pragma once

#include <vector>
#include <string>

class Value
{
public:
    enum Type {
        Invalid,
        Number,
        String,
        Array,
        Object
    };

    virtual ~Value() { }
    virtual Type type() const = 0;
};

class NumberValue : public Value
{
public:
    static NumberValue *from(Value *v) { return v->type() == Number ? static_cast<NumberValue *>(v) : 0; }
    Type type() const override { return Number; }

    void setNumber(double n) { m_number = n; }
    double number() const { return m_number; }

private:
    double m_number = 0;
};


class StringValue : public Value
{
public:
    static StringValue *from(Value *v) { return v->type() == String ? static_cast<StringValue *>(v) : 0; }
    Type type() const { return String; }

    void setString(const std::string &string) { m_string = string; }
    const std::string &string() const { return m_string; }

private:
    std::string m_string;
};


class ArrayValue : public Value
{
public:
    ~ArrayValue()
    {
        for (auto v : m_array)
            delete v;
    }

    Type type() const { return Array; }

    static ArrayValue *from(Value *v) { return v->type() == Array ? static_cast<ArrayValue *>(v) : 0; }

    void add(Value *value) { m_array.push_back(value); }
    const std::vector<Value *> &array() const { return m_array; }

private:
    std::vector<Value *> m_array;
};


class ObjectValue : public Value
{
public:
    ~ObjectValue()
    {
        for (auto v : m_object)
            delete v.value;
    }

    struct KeyedValue {
        std::string key;
        Value *value;
    };

    Type type() const { return Object; }

    static ObjectValue *from(Value *v) { return v->type() == Object ? static_cast<ObjectValue *>(v) : 0; }

    void add(const std::string &key, Value *value) {
        KeyedValue kv;
        kv.key = key;
        kv.value = value;
        m_object.push_back(kv);
    }
    const std::vector<KeyedValue> &object() const { return m_object; }

private:
    std::vector<KeyedValue> m_object;
};

