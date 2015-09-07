#pragma once

#include "tokenizer.h"
#include "value.h"

class Parser
{
public:
    Value *parse(Tokenizer *tokenizer);
    bool parseObject(Tokenizer *tokenizer, ObjectValue *value);
};

Value *Parser::parse(Tokenizer *tokenizer)
{
    Tokenizer::Token t = tokenizer->next();

    if (t != Tokenizer::Tk_OpenCurly) {
        std::cerr << "error: expected '{' at the start of the file, was='" << tokenizer->tokenAsString() << "'" << std::endl;
        return 0;
    }

    ObjectValue *value = new ObjectValue();
    parseObject(tokenizer, value);

    return value;
}

bool Parser::parseObject(Tokenizer *tokenizer, ObjectValue *value)
{
    Tokenizer::Token t = tokenizer->next();
    while (t != Tokenizer::Tk_EndOfStream && t != Tokenizer::Tk_CloseCurly) {

        if (t != Tokenizer::Tk_String) {
            std::cerr << "error: expected string value, line="
                     << tokenizer->line() << ", position=" << tokenizer->position() << std::endl;
            return false;
        }

        std::string key = tokenizer->stringValue();

        t = tokenizer->next();
        if (t != Tokenizer::Tk_Colon) {
            std::cerr << "error: expected string value, line="
                     << tokenizer->line() << ", position=" << tokenizer->position() << std::endl;
            return false;
        }

        t = tokenizer->next();
        if (t == Tokenizer::Tk_Number) {
            NumberValue *nv = new NumberValue();
            nv->setNumber(tokenizer->numberValue());
            value->add(key, nv);
        } else if (t == Tokenizer::Tk_String) {
            StringValue *sv = new StringValue();
            sv->setString(tokenizer->stringValue());
            value->add(key, sv);
        } else if (t == Tokenizer::Tk_OpenCurly) {
            ObjectValue *ov = new ObjectValue();
            parseObject(tokenizer, ov);
            value->add(key, ov);
        } else if (t == Tokenizer::Tk_OpenSquare) {
            while (tokenizer->token() != Tokenizer::Tk_CloseSquare)
                tokenizer->next();
        }

        t = tokenizer->next();
        if (t != Tokenizer::Tk_Colon && t != Tokenizer::Tk_CloseCurly) {
            std::cerr << "error: expected ',' or '}', line="
                     << tokenizer->line() << ", position=" << tokenizer->position() << std::endl;
            return false;
        }
    }

    return true;
}
