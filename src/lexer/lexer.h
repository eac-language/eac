#ifndef EAC_LEXER_H
#define EAC_LEXER_H

#include "../common/token.h"

typedef struct Lexer Lexer;

/**
 * @param source    Pointer to null-terminated source code string
 * @return          Pointer to initialized Lexer, or NULL on allocation failure
 */
Lexer* initLexer(const char* source);

/**
 *
 * 
 * @param lexer     Pointer to the Lexer
 * @return          The next Token
 */
Token getNextToken(Lexer* lexer);

/**
 * freeLexer - Free all memory associated with the lexer
 * 
 * @param lexer     Pointer to the Lexer to free
 * 
 * After calling this function, the lexer pointer is invalid.
 * Does not free the source code string (caller's responsibility).
 */
void freeLexer(Lexer* lexer);

#endif // EAC_LEXER_H
