#include "picojson.h"
#include <fstream>
#include "objectmodel.h"

using namespace std;
using namespace picojson;

struct Options {
    string inputFile;
    vector<string> includeDirs;
} options;

static void printHelp(char *cmd)
{
    cout << "Usage:" << endl
         << " > " << cmd << "[options] inputfile" << endl
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
        if (strstr(argv[i], "-h") != nullptr || strstr(argv[i], "--help")) {
            printHelp(argv[0]);
            return 0;
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

    for (auto i : options.includeDirs)
        cerr << "include dir: " << i << endl;

    value content;
    if (!loadFile(options.inputFile, &content))
        return 1;

    substituteImports(&content);

    cout << content.serialize(true);

    return 0;
}