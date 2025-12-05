#ifndef EAC_PARSER_H
#define EAC_PARSER_H

#include "../common/token.h"
#include "../lexer/lexer.h"
#include "ast.h"

typedef struct Parser Parser;

/**
 * initParser - Initialize a new parser with a lexer
 * 
 * @param lexer     Pointer to an initialized lexer
 * @return          Pointer to initialized Parser, or NULL on failure
 */
Parser* initParser(Lexer* lexer);

/**
 * parse - Parse the source code and build an AST
 * 
 * @param parser    Pointer to the parser
 * @return          Root AST node (AST_PROGRAM), or NULL on error
 */
ASTNode* parse(Parser* parser);

/**
 * hasError - Check if parser encountered any errors
 * 
 * @param parser    Pointer to the parser
 * @return          true if errors occurred, false otherwise
 */
bool hasError(Parser* parser);

/**
 * freeParser - Free parser memory
 * 
 * @param parser    Pointer to the parser
 */
void freeParser(Parser* parser);

#endif // EAC_PARSER_H