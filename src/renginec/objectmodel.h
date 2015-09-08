#pragma once

#include <string>

struct Property
{
    std::string name;
    std::string type;
};

struct Signal
{
    std::string name;
    std::string signature;
};

struct Function
{
    std::string name;
    std::string signature;
    std::string returnValue;
};

struct Resource
{
    std::string type;
    std::string name;
    std::string initializer;
};

struct Value
{
    enum {
        InvalidValue,
        NumberValue,
        StringValue,
        BindingValue
    };
    union {
        double numberValue;
        std::string *stringValue;
    };
    unsigned type = InvalidValue;
};

struct Class
{
    std::string name;
    std::string nspace;
    std::vector<Property> properties;
    std::vector<Signal> signals;
    std::vector<Function> functions;
    std::vector<Resource> resources;

};

struct ClassDeclaration
{
    std::string include;
    std::string name;
    std::string nspace;
    std::vector<Property> properties;
    std::vector<Signal> signals;
    std::vector<Function> functions;
};

struct Object
{
    Class *objectClass = 0;
    std::map<std::string, Value> properties;
    std::vector<Object *> children;
};

