#include "tokenizer.h"
#include "parser.h"
#include "fstream"

using namespace std;

struct Options {
    string inputFile;
} options;

void printHelp(char *cmd)
{
    cout << "Usage:" << endl
         << " > " << cmd << "[options] inputfile" << endl
         << endl;
}

int main(int argc, char **argv)
{
    for (int i=0; i<argc; ++i) {
        if (strstr(argv[i], "-h") != nullptr || strstr(argv[i], "--help")) {
            printHelp(argv[0]);
            return 0;
        } else {
            options.inputFile = std::string(argv[i]);
        }
    }

    if (options.inputFile.size() == 0) {
        printHelp(argv[0]);
        return 0;
    }

    cerr << "Reading file: " << options.inputFile << endl;

    ifstream stream(options.inputFile.c_str(), ifstream::in);
    Tokenizer tokenizer(stream);

    Parser parser;
    parser.parse(&tokenizer)
}