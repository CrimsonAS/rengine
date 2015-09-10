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

#include <set>
#include <cctype>

class ObjectModelBuilder
{
public:
    void setVerbose(bool v) { m_verbose = v; }

    bool build(const picojson::value &value);


private:
    bool buildClasses(const picojson::value &value, bool isDeclaration);
    bool buildMaybeClass(const picojson::value &value, bool isDeclaration);
    bool buildClass(const picojson::object &object, bool declaration);

    bool buildProperty(const std::string &property, Class *clazz);
    bool buildMaybeProperty(const picojson::value &value, Class *clazz);

    bool buildSignal(const std::string &property, Class *clazz);
    bool buildMaybeSignal(const picojson::value &value, Class *clazz);

    static std::string findStringInObject(const picojson::object &o, const char *key);

    std::set<std::string> m_includes;
    std::map<std::string, Class *> m_classes;

    bool m_verbose = false;
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
            ok = buildClasses(i.second, true);
        } else if (i.first == "classes") {
            ok = buildClasses(i.second, false);
        }

        if (!ok) {
            std::cerr << "error: failed to build '" << i.first << "', " << i.second.to_str() << std::endl;
        }
    }
    return true;
}

bool ObjectModelBuilder::buildClasses(const picojson::value &value, bool declaration)
{
    if (value.is<picojson::array>()) {
        const picojson::array &array = value.get<picojson::array>();
        for (auto i : array) {
            if (!buildMaybeClass(i, declaration)) {
                std::cerr << "error: failed to build class " << (declaration ? " declaration" : "") << ", " << i.to_str() << std::endl;
                return false;
            }
        }
    } else if (value.is<picojson::object>()) {
        if (!buildClass(value.get<picojson::object>(), declaration)) {
            std::cerr << "error: failed to build class " << (declaration ? " declaration" : "") << ", " << value.to_str() << std::endl;
            return false;
        }
    }
    return true;
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

std::string ObjectModelBuilder::findStringInObject(const picojson::object &o, const char *key)
{
    auto it = o.find(key);
    if (it == o.end())
        return std::string();
    return string_trim(it->second.to_str());
}

bool ObjectModelBuilder::buildMaybeClass(const picojson::value &value, bool declaration)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: class" << (declaration ? " declaration" : "") << " is not a JSON object.." << std::endl;
        return false;
    }
    return buildClass(value.get<picojson::object>(), declaration);
}

bool ObjectModelBuilder::buildClass(const picojson::object &classObject, bool declaration)
{
    // Class *clazz = new Class();
    std::string name = findStringInObject(classObject, "name");
    if (name.empty()) {
        std::cerr << "error: class " << (declaration ? "declaration" : "") << " missing 'name'" << std::endl;
        return false;
    }

    Class *clazz = new Class();
    clazz->name = name;

    std::unique_ptr<Class> clazzCleaner(clazz);

    if (declaration) {
        std::string include = findStringInObject(classObject, "include");
        if (include.empty()) {
            std::cerr << "error: class " << (declaration ? "declaration" : "") << " missing 'include', name=" << name << std::endl;
            return false;
        }
        std::string alloc = findStringInObject(classObject, "alloc");
        if (alloc.empty()) {
            std::cerr << "error: class " << (declaration ? "declaration" : "") << " missing 'alloc', name=" << name << std::endl;
            return false;
        }
        clazz->include = include;
        clazz->alloc = alloc;
        clazz->declarationOnly = true;

        m_includes.insert(include);
        if (m_verbose)
            std::cerr << "Class (declaration): name='" << name << "' include='" << include << "' alloc='" << alloc << "'" << std::endl;
    } else {
        if (m_verbose)
            std::cerr << "Class: name='" << name << "'" << std::endl;
    }


    auto propertyIt = classObject.find("properties");
    if (propertyIt != classObject.end()) {
        const picojson::value &props = propertyIt->second;
        if (props.is<picojson::array>()) {
            for (auto prop : props.get<picojson::array>()) {
                if (!buildMaybeProperty(prop, clazz)) {
                    return false;
                }
            }
        } else if (props.is<std::string>()) {
            if (!buildProperty(props.get<std::string>(), clazz)) {
                return false;
            }
        } else {
            std::cerr << "error: invalid property in class=" << name << std::endl;
            return false;
        }
    }

    auto signalIt = classObject.find("signals");
    if (signalIt != classObject.end()) {
        const picojson::value &signals = signalIt->second;
        if (signals.is<picojson::array>()) {
            for (auto signal : signals.get<picojson::array>()) {
                if (!buildMaybeSignal(signal, clazz)) {
                    return false;
                }
            }
        } else if (signals.is<std::string>()) {
            if (!buildSignal(signals.get<std::string>(), clazz)) {
                return false;
            }
        } else {
            std::cerr << "error: invalid signal in class=" << name << std::endl;
            return false;
        }
    }

    m_classes.insert(std::pair<std::string, Class *>(name, clazz));
    clazzCleaner.reset(); // to avoid the auto-cleanup on out-of-scope.

    return true;
}

bool ObjectModelBuilder::buildMaybeProperty(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: property in class='" << clazz->name << "' is not a JSON string.." << std::endl;
        return false;
    }
    return buildProperty(string_trim(value.get<std::string>()), clazz);
}

bool ObjectModelBuilder::buildProperty(const std::string &prop, Class *clazz)
{
    size_t space = prop.find_last_of(" ");
    if (space == std::string::npos) {
        std::cerr << "error: invalid property: " << prop << " in class: " << clazz->name << std::endl;
        return false;
    }
    Property property;
    property.clazz = clazz;
    property.type = prop.substr(0, space);
    property.name = prop.substr(space+1);

    clazz->properties.push_back(property);

    if (m_verbose)
        std::cerr << " - property: type='" << property.type << "', name='" << property.name << "'" << std::endl;

    return true;
}

bool ObjectModelBuilder::buildMaybeSignal(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: signal in class='" << clazz->name << "' is not a JSON string.." << std::endl;
        return false;
    }
    return buildSignal(string_trim(value.get<std::string>()), clazz);
}

bool ObjectModelBuilder::buildSignal(const std::string &signal, Class *clazz)
{
    size_t argumentsStart = signal.find_last_of("<");
    size_t argumentsEnd = signal.find_last_of(">");

    std::string name;
    std::string args;


    if (argumentsStart == std::string::npos && argumentsEnd == std::string::npos) {
        // no arguments, equal to <>, aka void
        name = signal;
    } else if (argumentsStart == std::string::npos || argumentsEnd == std::string::npos
               || argumentsStart > argumentsEnd) {
        std::cerr << "error: malformed signal declaration in class='" << clazz->name << "', '" << signal << "'" << std::endl;
        return false;
    } else {
        args = string_trim(signal.substr(argumentsStart + 1, argumentsEnd - argumentsStart - 1));
        name = string_trim(signal.substr(0, argumentsStart));
    }

    Signal s;
    s.name = name;
    s.signature = args;
    clazz->signals.push_back(s);

    if (m_verbose)
        std::cerr << " - signal: type='" << args << "', name='" << name << "'" << std::endl;

    return true;
}


