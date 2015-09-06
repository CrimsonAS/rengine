#include "tokenizer.h"
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

    Tokenizer::Token token = tokenizer.next();

    while (token != Tokenizer::Tk_EndOfStream) {
        if (token == Tokenizer::Tk_OpenCurly) cout << "{" << endl;
        else if (token == Tokenizer::Tk_CloseCurly) cout << "}" << endl;
        else if (token == Tokenizer::Tk_OpenSquare) cout << "[" << endl;
        else if (token == Tokenizer::Tk_CloseSquare) cout << "]" << endl;
        else if (token == Tokenizer::Tk_Colon) cout << ":" << endl;
        else if (token == Tokenizer::Tk_Comma) cout << "," << endl;
        else if (token == Tokenizer::Tk_String) cout << tokenizer.stringValue() << endl;
        else if (token == Tokenizer::Tk_Number) cout << tokenizer.numberValue() << endl;
        token = tokenizer.next();
    }
}