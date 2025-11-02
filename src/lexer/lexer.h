#ifndef LEXER_H
#define LEXER_H

#include "../common/token.h"

void initLexer(const char* source);
Token scanToken();

#endif
