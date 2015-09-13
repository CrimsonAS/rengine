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

#include "picojson.h"
#include <fstream>
#include "objectmodel.h"
#include "objectmodelbuilder.h"
#include "codegenerator.h"

using namespace std;
using namespace picojson;

struct Options {
    string inputFile;
    vector<string> includeDirs;
    bool verbose;
    bool dumpJson;
    bool includeMain;
} options;

static void printHelp(char *cmd)
{
    cout << "Usage:" << endl
         << " > " << cmd << "[options] inputfile" << endl
         << endl
         << "Options: " << endl
         << "  -v   --verbose        Print extra output to stderr" << endl
         << "  -d   --dump-json      Dump the complete json file" << endl
         << "  -m   --include-main   Include a dummy main() for easy testing" << endl
         << endl;
}

static bool loadFile(const string &file, value *v)
{
    cerr << "loading file: " << file << endl;
    ifstream stream(file.c_str(), ifstream::in);
    if (stream.eof()) {
        cerr << "error(" << file << "): could not open file..." << endl;
        return false;
    }

    stream >> *v;

    string error = get_last_error();
    if (!error.empty()) {
        cerr << "error(" << file << "): " << error << endl;
        return false;
    }

    if (!v->is<object>()) {
        cerr << "error(" << file << "): no toplevel object..." << endl;
        return false;
    }

    return true;
}

static bool fileExists(const std::string& name) {
    ifstream f(name.c_str());
    bool exists = f.good();
    f.close();
    return exists;
}

static bool loadFileInIncludeDirs(const std::string &file, value *v)
{
    for (auto i : options.includeDirs) {
        *v = value();
        std::string fileToTry = i + file;
        if (fileExists(fileToTry) && loadFile(fileToTry, v))
            return true;
    }
    return false;
}

static bool substituteImports(value *v)
{
    if (v->is<object>()) {
        object &o = v->get<object>();

        auto it = o.find("import");
        if (it != o.end()) {
            std::string importFile = it->second.get<std::string>();
            o.erase(it);

            value importValue;
            if (!loadFileInIncludeDirs(importFile, &importValue)) {
                cerr << "error: failed 'import' substitution of '" << importFile << "'" << endl;
                return false;
            }
            if (importValue.is<object>()) {
                for (auto impit : importValue.get<object>())
                    o.insert(pair<string,value>(impit.first, impit.second));
            }
        } else {
            for (auto i : o) {
                bool ok = substituteImports(&i.second);
                if (!ok)
                    return false;
            }
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    options.includeDirs.push_back("./");

    for (int i=1; i<argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            options.verbose = true;
        } else if (arg == "-d" || arg == "--dump-json") {
            options.dumpJson = true;
        } else if (arg == "-m" || arg == "--include-main") {
            options.includeMain = true;
        } else {
            std::string file(argv[i]);
            options.inputFile = file;
            size_t slash = file.find_last_of("/");
            if (slash != string::npos) {
                std::string dir = file.substr(0, slash+1);
                options.includeDirs.push_back(dir);
            }
        }
    }

    if (options.inputFile.size() == 0) {
        printHelp(argv[0]);
        return 0;
    }

    value content;
    if (!loadFile(options.inputFile, &content))
        return 1;

    substituteImports(&content);

    if (options.dumpJson)
        cout << content.serialize(true);

    ObjectModelBuilder builder;
    builder.setVerbose(options.verbose);
    builder.build(content);

    CodeGenerator generator;
    generator.setClasses(builder.classes());
    generator.setIncludeMain(options.includeMain);
    generator.generate();

    return 0;
}