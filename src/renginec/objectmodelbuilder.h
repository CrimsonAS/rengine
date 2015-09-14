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
#include <sstream>

class ObjectModelBuilder
{
public:
    void setVerbose(bool v) { m_verbose = v; }

    bool build(const picojson::value &value);

    const std::map<std::string, Class *> &classes() const { return m_classes; }

private:
    bool registerId(Class *clazz, const std::string &id);
    std::string uniqueIdentifier(Class *clazz, const std::string &id);
    bool buildClasses(const picojson::value &value, bool isDeclaration);
    bool buildMaybeClass(const picojson::value &value, bool isDeclaration);
    bool buildClass(const picojson::object &object, bool declaration);
    bool buildProperty(const std::string &property, Class *clazz);
    bool buildMaybeProperty(const picojson::value &value, Class *clazz);
    bool buildSignal(const std::string &signal, Class *clazz);
    bool buildMaybeSignal(const picojson::value &value, Class *clazz);
    bool buildFunction(const std::string &function, Class *clazz);
    bool buildMaybeFunction(const picojson::value &value, Class *clazz);
    bool buildResource(const std::string &resource, Class *clazz);
    bool buildMaybeResource(const picojson::value &value, Class *clazz);
    bool buildObject(const picojson::value &value, Object *parent, Class *clazz);
    bool buildBindingDependency(Binding *binding, const std::string &dependency, Object *instance, Class *clazz);
    bool buildMaybeReplicator(const picojson::value &value, Class *clazz);
    bool buildReplicator(const picojson::object &object, Class *clazz);

    static std::string findStringInObject(const picojson::object &o, const char *key);

    std::map<Class *, std::set<std::string>> m_ids;
    std::set<std::string> m_includes;
    std::map<std::string, Class *> m_classes;

    int m_unnamedCounter;

    bool m_verbose = false;
};

bool ObjectModelBuilder::registerId(Class *clazz, const std::string &id)
{
    auto ids = m_ids[clazz];
    if (ids.find(id) != ids.end())
        return false;
    ids.insert(id);
    return true;
}

std::string ObjectModelBuilder::uniqueIdentifier(Class *clazz, const std::string &id)
{
    if (id.empty()) {
        ++m_unnamedCounter;
        return std::string("unnamable") + std::to_string(m_unnamedCounter);
    }
    return id;
}

bool ObjectModelBuilder::build(const picojson::value &value)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: toplevel json element is not a value" << value.to_str() << std::endl;
        return false;
    }

    bool ok = true;
    const picojson::object &object = value.get<picojson::object>();
    for (auto i : object) {
        if (i.first == "class-declarations") {
            ok = buildClasses(i.second, true);
        } else if (i.first == "classes") {
            ok = buildClasses(i.second, false);
        }

        if (!ok) {
            std::cerr << "error: failed to build '" << i.first << "', " << i.second.to_str() << std::endl;
            break;
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
    m_classes[name] = clazz;

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
                if (!buildMaybeProperty(prop, clazz))
                    return false;
            }
        } else if (props.is<std::string>()) {
            if (!buildProperty(props.get<std::string>(), clazz))
                return false;
        } else {
            std::cerr << "error: invalid property in class=" << name << std::endl;
            return false;
        }
    }

    auto signalIt = classObject.find("signals");
    if (signalIt != classObject.end()) {
        const picojson::value &signals = signalIt->second;
        if (signals.is<picojson::array>()) {
            for (auto signal : signals.get<picojson::array>())
                if (!buildMaybeSignal(signal, clazz))
                    return false;
        } else if (signals.is<std::string>()) {
            if (!buildSignal(signals.get<std::string>(), clazz))
                return false;
        } else {
            std::cerr << "error: invalid signal in class=" << name << std::endl;
            return false;
        }
    }

    auto functionsIt = classObject.find("functions");
    if (functionsIt != classObject.end()) {
        const picojson::value &functions = functionsIt->second;
        if (functions.is<picojson::array>()) {
            for (auto function : functions.get<picojson::array>())
                if (!buildMaybeFunction(function, clazz))
                    return false;
        } else if (functions.is<std::string>()) {
            if (!buildFunction(functions.get<std::string>(), clazz))
                return false;
        } else {
            std::cerr << "error: invalid function in class=" << name << std::endl;
            return false;
        }
    }

    auto resourcesIt = classObject.find("resources");
    if (resourcesIt != classObject.end()) {
        const picojson::value &resources = resourcesIt->second;
        if (resources.is<picojson::array>()) {
            for (auto resource : resources.get<picojson::array>())
                if (!buildMaybeResource(resource, clazz))
                    return false;
        } else if (resources.is<std::string>()) {
            if (!buildResource(resources.get<std::string>(), clazz))
                return false;
        } else {
            std::cerr << "error: invalid resource in class=" << name << std::endl;
            return false;
        }
    }

    if (!declaration) {
        auto rootIt = classObject.find("root");
        if (rootIt != classObject.end()) {
            const picojson::value &root = rootIt->second;
            if (!buildObject(root, 0, clazz))
                return false;
        }

    }

    auto replicatorsIt = classObject.find("replicators");
    if (replicatorsIt != classObject.end()) {
        const picojson::value &replicators = replicatorsIt->second;
        if (replicators.is<picojson::array>()) {
            for (auto replicator : replicators.get<picojson::array>())
                if (!buildMaybeReplicator(replicator, clazz))
                    return false;
        } else if (replicators.is<picojson::object>()) {
            if (!buildReplicator(replicators.get<picojson::object>(), clazz))
                return false;
        } else {
            std::cerr << "error: invalid 'replicators' in class=" << name << std::endl;
            return false;
        }
    }

    return true;
}

bool ObjectModelBuilder::buildMaybeProperty(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: property in class='" << clazz->name << "' is not a JSON string.." << std::endl;
        return false;
    }
    return buildProperty(value.get<std::string>(), clazz);
}

bool ObjectModelBuilder::buildProperty(const std::string &propertyString, Class *clazz)
{
    std::string prop = string_trim(propertyString);
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

bool ObjectModelBuilder::buildSignal(const std::string &signalString, Class *clazz)
{
    std::string signal = string_trim(signalString);

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
    s.clazz = clazz;
    s.name = name;
    s.signature = args;
    clazz->signals.push_back(s);

    if (m_verbose)
        std::cerr << " - signal: type='" << args << "', name='" << name << "'" << std::endl;

    return true;
}

bool ObjectModelBuilder::buildMaybeFunction(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: function in class='" << clazz->name << "' is not a JSON string.." << std::endl;
        return false;
    }
    return buildFunction(string_trim(value.get<std::string>()), clazz);
}

bool ObjectModelBuilder::buildFunction(const std::string &functionString, Class *clazz)
{
    std::string function = string_trim(functionString);

    Function f;
    f.clazz = clazz;
    f.signature = function;
    clazz->functions.push_back(f);

    if (m_verbose)
        std::cerr << " - function: '" << function << "'" << std::endl;

    return true;
}

bool ObjectModelBuilder::buildMaybeResource(const picojson::value &value, Class *clazz)
{
    if (!value.is<std::string>()) {
        std::cerr << "error: resource in class='" << clazz->name << "' is not a JSON string.." << std::endl;
        return false;
    }
    return buildResource(value.get<std::string>(), clazz);
}

bool ObjectModelBuilder::buildResource(const std::string &resourceString, Class *clazz)
{
    std::string resource = string_trim(resourceString);

    std::istringstream iss(resource);
    std::string type, name, initializer;
    char assign;
    iss >> type >> name >> assign >> initializer;

    if (type.empty() || name.empty() || initializer.empty() || assign != '=') {
        std::cerr << "error: malformed 'resource' string in class='" << clazz->name
                  << "', expected: '[type] [name] = [initialier]'" << std::endl;
        return false;
    }

    Resource r;
    r.name = name;
    r.type = type;
    r.initializer = initializer;
    clazz->resources.push_back(r);
    registerId(clazz, name);

    if (m_verbose)
        std::cerr << " - resource: type='" << type << "', name='" << name << "', initializer='" << initializer << "'" << std::endl;

    return true;
}

bool ObjectModelBuilder::buildObject(const picojson::value &value, Object *parent, Class *clazz)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: 'root' entry in class='" << clazz->name << "' is not a JSON object..." << std::endl;
        return false;
    }

    const picojson::object &object = value.get<picojson::object>();

    std::string typeName = findStringInObject(object, "class");
    if (typeName.empty()) {
        std::cerr << "error: object has empty or missing 'class' in class='" << clazz->name << "', " << std::endl;
        return false;
    }

    std::string id = uniqueIdentifier(clazz, (findStringInObject(object, "id")));

    Object *instance = new Object();
    instance->parent = parent;
    instance->id = id;

    auto classIt = m_classes.find(typeName);
    if (classIt == m_classes.end()) {
        std::cerr << "error: unknown class=" << typeName << ", for object id=" << id << std::endl;
        return false;
    }
    instance->clazz = classIt->second;

    // Parse out the properties and values..
    for (auto member : object) {
        const std::string &key = member.first;

        // These are special cased above and below
        if (key == "children" || key == "id" || key == "class")
            continue;

        // Verify that the class in question actually has the property
        bool exists = false;
        for (auto classProperty : instance->clazz->properties) {
            if (classProperty.name == key) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            std::cerr << "error: object id=" << id << ", class=" << typeName << ", unknown property=" << key << std::endl;
            return false;
        }

        const picojson::value &propertyValue = member.second;
        if (propertyValue.is<double>()) {
            instance->propertyValues[key] = Value::number(propertyValue.get<double>());
        } else if (propertyValue.is<std::string>()) {
            instance->propertyValues[key] = Value::string(propertyValue.get<std::string>());
        } else if (propertyValue.is<picojson::object>()) {
            const picojson::object &binding = propertyValue.get<picojson::object>();
            auto bind = binding.find("bind");
            auto to = binding.find("to");

            if (bind != binding.end() && bind->second.is<std::string>()
                && to != binding.end() && (to->second.is<std::string>() || to->second.is<picojson::array>())) {
                std::shared_ptr<Binding> binding(new Binding());
                if (to->second.is<std::string>()) {
                    if (!buildBindingDependency(binding.get(), to->second.get<std::string>(), instance, clazz)) {
                        std::cerr << "error: malformed binding dependency=" << to->second.get<std::string>() << ", in object id="
                                  << id << ", class=" << typeName << ", property=" << key << std::endl;
                        return false;
                    }
                } else {
                    for (auto i : to->second.get<picojson::array>()) {
                        if (!i.is<std::string>()) {
                            std::cerr << "error: binding dependency needs to be a JSON string, id=" << id << ", property="
                                      << key << ", class=" << typeName << std::endl;
                            return false;
                        }
                        if (!buildBindingDependency(binding.get(), i.get<std::string>(), instance, clazz)) {
                            std::cerr << "error: malformed binding dependency=" << i.get<std::string>() << ", in object id="
                                      << id << ", class=" << typeName << ", property=" << key << std::endl;
                            return false;
                        }
                    }
                }
                binding->expression = bind->second.get<std::string>();
                instance->propertyValues[key] = Value::binding(binding);
            } else {
                std::cerr << "error: malformed binding in object id=" << id << ", class=" << typeName
                          << ", property=" << key << std::endl;
                return false;
            }
        }
    }

    // Recursively traverse the children..
    auto childrenIt = object.find("children");
    if (childrenIt != object.end()) {
        const picojson::value &children = childrenIt->second;
        if (children.is<picojson::array>()) {
            for (auto child : children.get<picojson::array>())
                if (!buildObject(child, instance, clazz))
                    return false;
        } else if (children.is<picojson::object>()) {
            if (!buildObject(children, instance, clazz))
                return false;
        } else {
            std::cerr << "error: object id=" << id << " of class=" << typeName << ", has invalid children, must be JSON array or object" << std::endl;
            return false;
        }
    }

    if (m_verbose) {
        std::string padding;
        Object *p = parent;
        while (p) {
            padding += "  ";
            p = p->parent;
        }
        std::cerr << " - object: " << padding << "id=" << id << ", class=" << typeName << ", children=" << instance->children.size() << std::endl;
    }

    if (parent)
        parent->children.push_back(instance);
    else
        clazz->root = instance;

    return true;
}

bool ObjectModelBuilder::buildBindingDependency(Binding *binding, const std::string &dep, Object *object, Class *clazz)
{
    // parse out "objectId->propertyName" and add as a Binding::Depencency to the Binding
    std::string dependency = string_trim(dep);
    size_t pos = dependency.find("->");

    Binding::Dependency d;

    // If we didn't find a ->, look in the class we're generating to see if the
    // binding matches that..
    if (pos == std::string::npos) {
        std::cout << "didn't find any match.." << std::endl;
        bool propertyInInstance = false;
        for (auto i : object->clazz->properties) {
            std::cout << "looking object, matching " << i.name << dependency << std::endl;
            if (i.name == dependency) {
                propertyInInstance = true;
                d.objectId = object->id;
                d.propertyName = dependency;
                break;
            }
        }
        if (!propertyInInstance) {
            for (auto i : clazz->properties) {
                std::cout << "looking class, matching " << i.name << dependency << std::endl;
                if (i.name == dependency) {
                    d.objectId = "this"; // refers to the instance of the class being generated
                    d.propertyName = dependency;
                    break;
                }
            }
        }
    } else {
        d.objectId = dependency.substr(0, pos);
        d.propertyName = dependency.substr(pos + 2);
    }
    if (d.objectId.empty() || d.propertyName.empty())
        return false;
    binding->dependencies.push_back(d);
    return true;
}

bool ObjectModelBuilder::buildMaybeReplicator(const picojson::value &value, Class *clazz)
{
    if (!value.is<picojson::object>()) {
        std::cerr << "error: 'replicator' in class=" << clazz->name << " is no a JSON object" << std::endl;
        return false;
    }

    return buildReplicator(value.get<picojson::object>(), clazz);
}

bool ObjectModelBuilder::buildReplicator(const picojson::object &object, Class *clazz)
{
    std::string id = uniqueIdentifier(clazz, findStringInObject(object, "id"));
    std::string className = findStringInObject(object, "class");
    if (className.empty()) {
        std::cerr << "error: 'replciator' missing 'class' tag, id=" << id << std::endl;
        return false;
    }
    std::string parentName = findStringInObject(object, "parent");
    if (parentName.empty()) {
        std::cerr << "error: 'replciator' missing 'parent' tag, id=" << id << std::endl;
        return false;
    }

    Replicator r;
    r.id = id;
    r.clazz = className;
    r.parentId = parentName;

    auto countIt = object.find("count");
    if (countIt != object.end()) {
        const picojson::value &countValue = countIt->second;
        if (countValue.is<double>()) {
            r.count = Value::number(countValue.get<double>());
        } else {
            std::cerr << "error: malformed value for replicator id=" << id << ", in class=" << className << std::endl;
            return false;
        }
    } else {
        r.count = Value::number(0);
    }

    r.initializor = findStringInObject(object, "initializor");
    r.terminator = findStringInObject(object, "terminator");

    if (m_verbose) {
        std::cerr << " - replicator, id=" << id << ": " << r.count.numberValue << "x '" << className << "' under parent=" << parentName << std::endl;
    }

    clazz->replicators.push_back(r);

    return true;
}