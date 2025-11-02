/**
 * lexer.h - Lexical Analyzer Interface for EaC Compiler
 * 
 * This file defines the public API for the EaC lexer.
 * The lexer is responsible for:
 *   - Tokenizing source code
 *   - Tracking indentation levels (Python-style significant whitespace)
 *   - Handling comments (single-line # and multi-line block comments)
 *   - Recognizing keywords, operators, literals, and identifiers
 */

#ifndef EAC_LEXER_H
#define EAC_LEXER_H

#include "../common/token.h"

/**
 * Lexer - Opaque structure representing the lexer state
 * 
 * Implementation details are hidden from the parser.
 * The parser only needs to know this type exists.
 */
typedef struct Lexer Lexer;

/**
 * initLexer - Initialize a new lexer with the given source code
 * 
 * @param source    Pointer to null-terminated source code string
 * @return          Pointer to initialized Lexer, or NULL on allocation failure
 * 
 * The caller is responsible for keeping the source code string valid
 * for the lifetime of the Lexer.
 */
Lexer* initLexer(const char* source);

/**
 * getNextToken - Retrieve the next token from the source code
 * 
 * @param lexer     Pointer to the Lexer
 * @return          The next Token
 * 
 * This function handles:
 *   - Skipping whitespace (except newlines and indentation)
 *   - Tracking indentation and emitting INDENT/DEDENT tokens
 *   - Recognizing all language tokens
 *   - Emitting TOKEN_EOF when the end of file is reached
 *   - Emitting TOKEN_ERROR for lexical errors
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
