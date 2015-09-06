#pragma once

#include <string>
#include <iostream>

class Tokenizer
{
public:
    enum Token {
        Tk_None,          // default value
        Tk_EndOfStream,   // when we're done parsing
        Tk_OpenCurly,     // {
        Tk_CloseCurly,    // }
        Tk_OpenSquare,    // [
        Tk_CloseSquare,   // ]
        Tk_Colon,         // :
        Tk_Comma,         // ,
        Tk_Number,        // well.. a number...
        Tk_String,        // "string-without-amps"
    };

    Tokenizer(std::istream &stream)
        : m_stream(stream)
        , m_token(Tk_None)
        , m_number(0.0)
        , m_line(0)
        , m_position(0)
        , m_stringStartLine(0)
        , m_stringStartPosition(0)
    {
    }

    Token next();

    Token token() const { return m_token; }
    std::string stringValue() const { return m_string; }
    double numberValue() const { return m_number; }

    unsigned line() const { return m_line; }
    unsigned position() const { return m_position; }

    unsigned stringStartLine() const { return m_stringStartLine; }
    unsigned stringStartPosition() const { return m_stringStartPosition; }

public:
    char readChar() {
        if (m_stream.eof())
            return 0;
        char c = m_stream.get();
        if (c == '\n') {
            ++m_line;
            m_position = 0;
        } else {
            ++m_position;
        }
        return c;
    }

    std::istream &m_stream;
    Token m_token;
    std::string m_string;
    double m_number;

    unsigned m_line;
    unsigned m_position;
    unsigned m_stringStartLine;
    unsigned m_stringStartPosition;
};

inline Tokenizer::Token Tokenizer::next() {

    if (m_token == Tk_EndOfStream)
        return Tk_EndOfStream;

    char current = readChar();

    Token token = Tk_None;
    while (token == Tk_None) {
        if (current == 0)        token = Tk_EndOfStream;
        else if (current == '{') token = Tk_OpenCurly;
        else if (current == '}') token = Tk_CloseCurly;
        else if (current == '[') token = Tk_OpenSquare;
        else if (current == ']') token = Tk_CloseSquare;
        else if (current == ':') token = Tk_Colon;
        else if (current == ',') token = Tk_Comma;
        else if (current == '"') {
            // string..
            token = Tk_String;
            m_stringStartLine = m_line;
            m_stringStartPosition = m_position;
            m_string.clear();
            current = readChar();
            while (current != '"' && current != 0) {
                m_string.push_back(current);
                current = readChar();
            }
        } else if ((current >= '0' && current <= '9') || current == '-') {
            token = Tk_Number;
            std::string number;
            number.push_back(current);
            current = readChar();
            while ((current >= '0' && current <= '9') || current == 'e' || current == '.') {
                number.push_back(current);
                current = readChar();
            }
            m_number = stof(number);

        } else {
            current = readChar();
        }
    }

    m_token = token;
    return m_token;
}