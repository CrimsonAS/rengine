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

#include "objectmodel.h"
#include "picojson.h"

#include <cctype>

class ObjectModelBuilder
{
public:
    bool build(const picojson::value &value);
    bool buildClassDeclarations(const picojson::value &value);
    bool buildClassDeclaration(const picojson::value &value);
    bool buildProperty(const picojson::value &value, Class *clazz);

    static std::string findStringInObject(const picojson::object &o, const char *key);

private:
    std::map<std::string, Class *> m_classes;
};

bool ObjectModelBuilder::build(const picojson::value &value)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: toplevel json element is not a value" << value.to_str() << std::endl;
        return false;
    }

    const picojson::object &object = value.get<picojson::object>();
    for (auto i : object) {
        bool ok = true;
        if (i.first == "class-declarations") {
            ok = buildClassDeclarations(i.second);
        }

        if (!ok) {
            std::cerr << "error: failed to build '" << i.first << "', " << i.second.to_str() << std::endl;
        }
    }
    return true;
}

bool ObjectModelBuilder::buildClassDeclarations(const picojson::value &value)
{
    if (value.is<picojson::array>()) {
        const picojson::array &array = value.get<picojson::array>();
        for (auto i : array) {
            if (!buildClassDeclaration(i)) {
                std::cerr << "error: failed to build class declaration: " << i.to_str() << std::endl;
                return false;
            }
        }
    } else if (value.is<picojson::object>()) {
        if (!buildClassDeclaration(value)) {
            std::cerr << "error: failed to build class declaration: " << value.to_str() << std::endl;
            return false;
        }
    }
    return true;
}

std::string ObjectModelBuilder::findStringInObject(const picojson::object &o, const char *key)
{
    auto it = o.find(key);
    if (it == o.end())
        return std::string();
    return it->second.to_str();
}


inline std::string string_trim(const std::string &input)
{
    if (input.size() < 1)
        return std::string();
    int first = 0, last = input.size() - 1;
    while (std::isspace(input.at(first)) && first < last) ++first;
    while (std::isspace(input.at(last)) && first < last) --last;
    return input.substr(first, last+1);
}

bool ObjectModelBuilder::buildProperty(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: properties must be strings..: " << value.serialize() << std::endl;
        return false;
    }

    std::string prop = string_trim(value.get<std::string>());

    size_t space = prop.find_last_of(" ");
    if (space == std::string::npos) {
        std::cerr << "error: invalid property: " << prop << " in class: " << clazz->name << std::endl;
        return false;
    }
    Property property;
    property.clazz = clazz;
    property.type = prop.substr(0, space);
    property.name = prop.substr(space+1, prop.size());

    clazz->properties.push_back(property);

    std::cout << clazz->name << ": '" << property.type << "', '" << property.name << "'" << std::endl;


    return true;
}

bool ObjectModelBuilder::buildClassDeclaration(const picojson::value &value)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: class declaration is not an object.." << std::endl;
        return false;
    }

    const picojson::object &classObject = value.get<picojson::object>();

    // Class *clazz = new Class();
    std::string name = findStringInObject(classObject, "name");
    std::string include = findStringInObject(classObject, "include");
    std::string alloc = findStringInObject(classObject, "alloc");
    if (name.empty() || include.empty() || alloc.empty()) {
        std::cerr << "error: class declaration is not complete...: " << value.serialize() << std::endl;
        return false;
    }

    Class *clazz = new Class();
    clazz->name = name;
    clazz->include = include;
    clazz->alloc = alloc;
    clazz->declarationOnly = true;

    auto propertyIt = classObject.find("properties");
    if (propertyIt != classObject.end()) {
        const picojson::value &props = propertyIt->second;
        if (props.is<picojson::array>()) {
            for (auto prop : props.get<picojson::array>()) {
                if (!buildProperty(prop, clazz))
                    return false;
            }
        } else if (props.is<std::string>()) {
            buildProperty(props, clazz);
        } else {
            std::cerr << "error: invalid property: " << value.serialize() << std::endl;
            return false;
        }
    }

    return true;
}