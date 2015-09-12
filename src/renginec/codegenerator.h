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

#include <fstream>
#include "objectmodel.h"

class CodeGenerator
{
public:
    void setClasses(const std::map<std::string, Class *> &classes) { m_classes = classes; }

    void generate();

private:
    bool generateClass(Class *clazz);

    void writeDisclaimer(std::ostream &stream);
    void writeIncludes(std::ostream &stream);
    void writeBeginningOfClass(std::ostream &stream, Class *clazz);
    void writeEndOfClass(std::ostream &stream, Class *clazz);
    void writeFunctions(std::ostream &stream, Class *clazz);
    void writeProperties(std::ostream &stream, Class *clazz);
    void writePropertyMemberVars(std::ostream &ostream, Class *clazz);
    void writeSignals(std::ostream &stream, Class *clazz);
    void writeResources(std::ostream &stream, Class *clazz);
    void writeObjects(std::ostream &stream, Class *clazz);
    void writeObject(std::ostream &stream, Object *object);
    void writeObjectMemberVars(std::ostream &stream, Object *object);
    void writeSignalHandlers(std::ostream &stream, Object *object, Class *clazz);

    std::map<std::string, Class *> m_classes;

    std::string m_outputDir;
};

void CodeGenerator::generate()
{
    bool atLeastOne = false;
    for (auto i : m_classes) {
        if (!i.second->declarationOnly) {
            atLeastOne = true;
            generateClass(i.second);
        }
    }
    if (!atLeastOne) {
        std::cerr << "No class definitions present in input. Nothing generated" << std::endl;
        return;
    }
}

bool CodeGenerator::generateClass(Class *clazz)
{
    std::string dir = m_outputDir.empty() ? "." : m_outputDir;
    std::string fileName = dir + "/generated_" + clazz->name + ".h";
    std::ofstream stream(fileName, std::ofstream::out);
    if (!stream.is_open()) {
        std::cerr << "Failed to open file='" << fileName << "'" << std::endl;
        return false;
    }

    writeDisclaimer(stream);
    stream << "#pragma once" << std::endl << std::endl;
    writeIncludes(stream);
    writeBeginningOfClass(stream, clazz);
    writeFunctions(stream, clazz);
    writeProperties(stream, clazz);
    writeSignals(stream, clazz);

    writeResources(stream, clazz);
    writePropertyMemberVars(stream, clazz);

    writeObjectMemberVars(stream, clazz->root);

    writeSignalHandlers(stream, clazz->root, clazz);

    writeObjects(stream, clazz);

    writeEndOfClass(stream, clazz);

    stream.close();

    return true;
}

void CodeGenerator::writeDisclaimer(std::ostream &s)
{
    s << "/*" << std::endl
      << " * This file is generated!" << std::endl
      << " * Any changes you make to it will be lost when it is regenerated." << std::endl
      << " */" << std::endl << std::endl;
}

void CodeGenerator::writeIncludes(std::ostream &s)
{
    std::set<std::string> includes;
    for (auto i : m_classes) {
        if (!i.second->include.empty())
            includes.insert(i.second->include);
    }
    for (auto i : includes)
        s << "#include \"" << i << "\"" << std::endl;
    s << std::endl;
}

void CodeGenerator::writeBeginningOfClass(std::ostream &s, Class *clazz)
{
    s << "class " << clazz->name << " : public rengine::SignalEmitter" << std::endl
      << "{" << std::endl
      << "public:" << std::endl
      << "    // Constructor" << std::endl
      << "    " << clazz->name << "(ResourceHandler *handler)" << std::endl
      << "    {" << std::endl
      << "        initResources(handler);" << std::endl
      << "        initObjects()" << std::endl
      << "    }" << std::endl
      << std::endl;
}

void CodeGenerator::writeEndOfClass(std::ostream &s, Class *clazz)
{
    s << "}; // end of class " << clazz->name << std::endl << std::endl;
}

void CodeGenerator::writeFunctions(std::ostream &s, Class *clazz)
{
    s << "    // functions forward declarations (to be implemented by user)" << std::endl;
    for (auto i : clazz->functions) {
        s << "    " << i.signature << ";" << std::endl;
    }
    s << std::endl;
}

void CodeGenerator::writeSignals(std::ostream &s, Class *clazz)
{
    s << "    // signals" << std::endl;
    for (auto i : clazz->signals)
        s << "    static Signal<" << i.signature << "> " << i.name << ";" << std::endl;
    s << std::endl;
}

void CodeGenerator::writeProperties(std::ostream &s, Class *clazz)
{
    s << "    // properties" << std::endl;
    for (auto i : clazz->properties) {
        std::string name = i.name;
        name[0] = std::toupper(name[0]);
        s << "    static Signal<> on" << name << "Changed;" << std::endl
          << "    const " << i.type << " &" << i.name << "() const {" << std::endl
          << "        return m_" << i.name << ";" << std::endl
          << "    }" << std::endl
          << "    void set" << name << "(const " << i.type << " &" << i.name << ") {" << std::endl
          << "        if (m_" << i.name << " == " << i.name << ")" << std::endl
          << "            return;" << std::endl
          << "        m_" << i.name << " = " << i.name << ";" << std::endl
          << "        on" << name << "Changed.emit(this);" << std::endl
          << "    }" << std::endl
          << std::endl;
    }
}

void CodeGenerator::writeResources(std::ostream &s, Class *clazz)
{
    s << "    // resources" << std::endl;
    for (auto i : clazz->resources) {
        s << "    " << i.type << " *" << i.name << ";" << std::endl;
    }
    s << std::endl
      << "    void initResources(ResourceHandler *handler) {" << std::endl;
    for (auto i : clazz->resources) {
        s << "        " << i.name << " = handler->create<" << i.type << ">(\"" << i.initializer << "\");" << std::endl;
    }
    s << "    }" << std::endl
      << std::endl;
}

void CodeGenerator::writePropertyMemberVars(std::ostream &s, Class *clazz)
{
    s << "    // property member vars" << std::endl;
    for (auto i : clazz->properties)
        s << "    " << i.type << " m_" << i.name << ";" << std::endl;
    s << std::endl;
}

void CodeGenerator::writeObjects(std::ostream &s, Class *clazz)
{
    s << "    // The objects.." << std::endl
      << "    void initObjects() {" << std::endl;
    writeObject(s, clazz->root);
    s << "    }" << std::endl
      << std::endl;
}

void CodeGenerator::writeObjectMemberVars(std::ostream &s, Object *object)
{
    // the root (first) object
    if (!object->parent)
        s << "    // object member variables..." << std::endl;

    s << "    " << object->clazz->name << " *" << object->id << ";" << std::endl;
    for (auto i : object->children)
        writeObjectMemberVars(s, i);

    if (!object->parent)
        s << std::endl;
}

inline std::string cg_binding_name(const std::string &id, const std::string &property)
{
    return "binding_" + id + "_" + property;
}

void CodeGenerator::writeObject(std::ostream &s, Object *object)
{
    if (object->parent)
        s << std::endl;
    s << "        " << object->id << " = " << object->clazz->alloc << ";" << std::endl;
    for (auto i : object->propertyValues) {
        std::string name = i.first;
        name[0] = std::toupper(name[0]);
        const Value &value = i.second;
        std::cout << i.first << std::endl;
        if (value.type == Value::StringValue) {
            s << "        " << object->id << "->set" << name << "(" << value.stringValue << "):" << std::endl;
        } else if (value.type == Value::NumberValue) {
            s << "        " << object->id << "->set" << name << "(" << value.numberValue << "):" << std::endl;
        } else if (value.type == Value::BindingValue) {
            for (const Binding::Dependency &dep : value.bindingValue->dependencies) {
                std::string property = dep.propertyName;
                property[0] = std::toupper(property[0]);
                s << "        " << dep.objectId << "->on" << property << "Changed.connect(&"
                  << cg_binding_name(object->id, i.first) << ");" << std::endl;
            }
            s << "        " << cg_binding_name(object->id, i.first) << ".self = this;" << std::endl;
        }
    }
    if (object->parent)
        s << "        " << object->parent->id << "->add(" << object->id << ");" << std::endl;
    for (auto i : object->children)
        writeObject(s, i);
}

void CodeGenerator::writeSignalHandlers(std::ostream &s, Object *object, Class *clazz)
{
    // the root...
    if (!object->parent)
        s << "    // Signal handlers..." << std::endl;
    for (auto i : object->propertyValues) {
        std::string propertyName = i.first;
        const Value &value = i.second;
        if (value.type == Value::BindingValue) {
            propertyName[0] = std::toupper(propertyName[0]);
            s << "    struct : public SignalHandler<> {" << std::endl
              << "        " << clazz->name << " *self;" << std::endl
              << "        void onSignal() override {" << std::endl
              << "            self->" << object->id << "->set" << propertyName << "(" <<  value.bindingValue->expression << ");" << std::endl
              << "        }" << std::endl
              << "    } " << cg_binding_name(object->id, i.first) << ";" << std::endl;
        }
    }
    for (auto i : object->children)
        writeSignalHandlers(s, i, clazz);

    if (!object->parent)
        s << std::endl;
}
