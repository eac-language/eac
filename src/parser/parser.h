#ifndef EAC_PARSER_H
#define EAC_PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"

// Forward declaration
typedef struct Parser Parser;

/**
 * Initialize the parser with a lexer
 * 
 * @param lexer The lexer to read tokens from
 * @return Parser instance or NULL on failure
 */
Parser* initParser(Lexer* lexer);

/**
 * Parse the entire program and validate syntax
 * 
 * Uses Recursive Descent Parsing algorithm.
 * Reads input one token at a time from the lexer.
 * Validates syntax without building an AST.
 * 
 * @param parser The parser instance
 * @return true if syntax is valid, false if syntax errors were found
 */
bool parse(Parser* parser);

/**
 * Free parser resources
 * 
 * @param parser The parser instance to free
 */
void freeParser(Parser* parser);

#endif // EAC_PARSER_H