/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <string>

struct Class;
struct Object;

struct Property
{
    Class *clazz = nullptr;
    std::string name;
    std::string type;
};

struct Signal
{
    Class *clazz = nullptr;
    std::string name;
    std::string signature;
};

struct Function
{
    Class *clazz = nullptr;
    std::string signature;
};

struct Resource
{
    std::string type;
    std::string name;
    std::string initializer;
};

struct Binding
{
    struct Dependency {
        std::string propertyName;
        std::string objectId;
    };
    std::string expression;
    std::vector<Dependency> dependencies;
};

struct Value
{
    enum {
        InvalidValue,
        NumberValue,
        StringValue,
        BindingValue
    };

    // Should have been a union, but then we'd have to memory manage the
    // the string and stuff, so keep it simple and waste memory..
    std::string stringValue;
    double numberValue;
    std::shared_ptr<Binding> bindingValue;
    unsigned type = InvalidValue;

    static Value number(double dval) {
        Value v;
        v.type = NumberValue;
        v.numberValue = dval;
        return v;
    }
    static Value string(const std::string &sval) {
        Value v;
        v.type = StringValue;
        v.stringValue = sval;
        return v;
    }
    static Value binding(const std::shared_ptr<Binding> &binding) {
        Value v;
        v.type = BindingValue;
        v.bindingValue = binding;
        return v;
    }
};


struct Class
{
    std::string name;
    std::string include;
    std::string alloc;
    std::vector<Property> properties;
    std::vector<Signal> signals;
    std::vector<Function> functions;
    std::vector<Resource> resources;

    Object *root;

    bool declarationOnly = false;
};

struct Object
{
    Class *clazz = nullptr;
    Object *parent;
    std::string id;
    std::map<std::string, Value> propertyValues;
    std::vector<Object *> children;
};

