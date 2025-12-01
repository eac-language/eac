#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

// Parser structure
struct Parser {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    int statementCount;
};

// ===== FORWARD DECLARATIONS =====

static void advance(Parser* parser);
static bool check(Parser* parser, TokenType type);
static bool match(Parser* parser, TokenType type);
static void consume(Parser* parser, TokenType type, const char* message);
static void consumeStatementTerminator(Parser* parser, const char* message);
static void error(Parser* parser, const char* message);
static void errorAtCurrent(Parser* parser, const char* message);
static void synchronize(Parser* parser);
static void skipNewlines(Parser* parser);
static bool isAtEnd(Parser* parser);

// Statement parsing
static void parseDeclaration(Parser* parser);
static void parseAssignment(Parser* parser, Token identifier);
static void parseInput(Parser* parser, Token identifier);
static void parseOutput(Parser* parser);
static void parseConditional(Parser* parser);
static void parseWhileLoop(Parser* parser);
static void parseForLoop(Parser* parser);
static void parseStatement(Parser* parser);
static void parseBlock(Parser* parser);

// Expression parsing
static void parseExpression(Parser* parser);
static void parseLogicalOr(Parser* parser);
static void parseLogicalAnd(Parser* parser);
static void parseEquality(Parser* parser);
static void parseRelational(Parser* parser);
static void parseTerm(Parser* parser);
static void parseFactor(Parser* parser);
static void parseUnary(Parser* parser);
static void parsePower(Parser* parser);
static void parsePrimary(Parser* parser);

// ===== UTILITY FUNCTIONS =====

Parser* initParser(Lexer* lexer) {
    if (lexer == NULL) return NULL;
    
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (parser == NULL) return NULL;
    
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
    parser->statementCount = 0;
    
    // Prime the parser
    advance(parser);
    
    return parser;
}

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = getNextToken(parser->lexer);
        
        if (parser->current.type != TOKEN_ERROR) break;
        
        errorAtCurrent(parser, parser->current.lexeme);
    }
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    
    errorAtCurrent(parser, message);
}

static void consumeStatementTerminator(Parser* parser, const char* message) {
    // Accept newline, EOF, or DEDENT (for end of block)
    if (match(parser, TOKEN_NEWLINE)) {
        return;
    }
    
    if (check(parser, TOKEN_EOF) || check(parser, TOKEN_DEDENT)) {
        return;
    }
    
    errorAtCurrent(parser, message);
}

// Updated error reporting to match documentation format
static void error(Parser* parser, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
    parser->hadError = true;
    
    // Format: "Error at line X: message"
    fprintf(stderr, "Error at line %d", parser->previous.line);
    
    if (parser->previous.type == TOKEN_EOF) {
        fprintf(stderr, ": Unexpected end of file");
    } else if (parser->previous.type != TOKEN_ERROR) {
        fprintf(stderr, ": %s", message);
        if (parser->previous.length > 0) {
            fprintf(stderr, " '%.*s'", parser->previous.length, parser->previous.lexeme);
        }
    } else {
        fprintf(stderr, ": %s", message);
    }
    
    fprintf(stderr, "\n");
}

static void errorAtCurrent(Parser* parser, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
    parser->hadError = true;
    
    // Format: "Error at line X: message"
    fprintf(stderr, "Error at line %d", parser->current.line);
    
    if (parser->current.type == TOKEN_EOF) {
        fprintf(stderr, ": Unexpected end of file");
    } else if (parser->current.type != TOKEN_ERROR) {
        fprintf(stderr, ": %s", message);
        if (parser->current.length > 0) {
            fprintf(stderr, " - got '%.*s'", parser->current.length, parser->current.lexeme);
        }
    } else {
        fprintf(stderr, ": %s", message);
    }
    
    fprintf(stderr, "\n");
}

static void synchronize(Parser* parser) {
    parser->panicMode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_NEWLINE) return;
        
        switch (parser->current.type) {
            case TOKEN_FLEX:
            case TOKEN_FIXED:
            case TOKEN_WHEN:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_OUTPUT:
            case TOKEN_INPUT:
            case TOKEN_RETURN:
            case TOKEN_BREAK:
            case TOKEN_CONTINUE:
            case TOKEN_FUNCTION:
                return;
            default:
                ;
        }
        
        advance(parser);
    }
}

static void skipNewlines(Parser* parser) {
    while (match(parser, TOKEN_NEWLINE)) {
        // Skip
    }
}

static bool isAtEnd(Parser* parser) {
    return parser->current.type == TOKEN_EOF;
}

// ===== EXPRESSION PARSING =====

static void parseExpression(Parser* parser) {
    parseLogicalOr(parser);
}

static void parseLogicalOr(Parser* parser) {
    parseLogicalAnd(parser);
    
    while (match(parser, TOKEN_OR)) {
        parseLogicalAnd(parser);
    }
}

static void parseLogicalAnd(Parser* parser) {
    parseEquality(parser);
    
    while (match(parser, TOKEN_AND)) {
        parseEquality(parser);
    }
}

static void parseEquality(Parser* parser) {
    parseRelational(parser);
    
    while (match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL)) {
        parseRelational(parser);
    }
}

static void parseRelational(Parser* parser) {
    parseTerm(parser);
    
    while (match(parser, TOKEN_LESS) || match(parser, TOKEN_GREATER) ||
           match(parser, TOKEN_LESS_EQUAL) || match(parser, TOKEN_GREATER_EQUAL)) {
        parseTerm(parser);
    }
}

static void parseTerm(Parser* parser) {
    parseFactor(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        parseFactor(parser);
    }
}

static void parseFactor(Parser* parser) {
    parseUnary(parser);
    
    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) ||
           match(parser, TOKEN_PERCENT) || match(parser, TOKEN_FLOOR_DIV)) {
        parseUnary(parser);
    }
}

static void parseUnary(Parser* parser) {
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_NOT)) {
        parseUnary(parser);
        return;
    }
    
    parsePower(parser);
}

static void parsePower(Parser* parser) {
    parsePrimary(parser);
    
    if (match(parser, TOKEN_CARET)) {
        parsePower(parser); // Right associative
    }
}

static void parsePrimary(Parser* parser) {
    // Boolean literals
    if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE)) {
        return;
    }
    
    // Numeric literals
    if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT)) {
        return;
    }
    
    // String literal
    if (match(parser, TOKEN_STRING)) {
        return;
    }
    
    // Char literal
    if (match(parser, TOKEN_CHAR)) {
        return;
    }
    
    // Identifier or function call
    if (match(parser, TOKEN_IDENTIFIER)) {
        // Function call
        if (match(parser, TOKEN_LPAREN)) {
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    parseExpression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expected ')' after function arguments");
            return;
        }
        
        // Array/list indexing
        if (match(parser, TOKEN_LBRACKET)) {
            parseExpression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after index");
            return;
        }
        
        // Simple identifier
        return;
    }
    
    // Parenthesized expression
    if (match(parser, TOKEN_LPAREN)) {
        parseExpression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return;
    }
    
    // List literal
    if (match(parser, TOKEN_LBRACKET)) {
        if (!check(parser, TOKEN_RBRACKET)) {
            do {
                parseExpression(parser);
            } while (match(parser, TOKEN_COMMA));
        }
        
        consume(parser, TOKEN_RBRACKET, "Expected ']' after list elements");
        return;
    }
    
    // Absolute value
    if (match(parser, TOKEN_VBAR)) {
        parseExpression(parser);
        consume(parser, TOKEN_VBAR, "Expected '|' after absolute value expression");
        return;
    }
    
    errorAtCurrent(parser, "Expected expression");
}

// ===== STATEMENT PARSING =====

static void parseDeclaration(Parser* parser) {
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier after variable type");
    
    // Optional type hint
    if (match(parser, TOKEN_COLON)) {
        if (!match(parser, TOKEN_HINT_INT) && !match(parser, TOKEN_HINT_FLOAT) &&
            !match(parser, TOKEN_HINT_STR) && !match(parser, TOKEN_HINT_BOOL) &&
            !match(parser, TOKEN_HINT_CHAR)) {
            errorAtCurrent(parser, "Invalid type hint - use 'int', 'float', 'str', 'bool', or 'char'");
        }
    }
    
    // Optional initializer (with 'to' noise word or '=')
    if (match(parser, TOKEN_TO) || match(parser, TOKEN_EQUAL)) {
        parseExpression(parser);
    }
    
    consumeStatementTerminator(parser, "Expected newline after declaration - statements end with newline");
}

static void parseAssignment(Parser* parser, Token identifier) {
    // Consume assignment operator (+=, -=, etc.)
    advance(parser);
    
    // Optional 'as' noise word for type casting (e.g., result as float = x / y)
    if (match(parser, TOKEN_AS)) {
        // Expect type hint after 'as'
        if (!match(parser, TOKEN_HINT_INT) && !match(parser, TOKEN_HINT_FLOAT) &&
            !match(parser, TOKEN_HINT_STR) && !match(parser, TOKEN_HINT_BOOL) &&
            !match(parser, TOKEN_HINT_CHAR)) {
            errorAtCurrent(parser, "Expected type hint after 'as' keyword");
        }
        // Now expect the actual assignment operator
        if (!match(parser, TOKEN_EQUAL)) {
            errorAtCurrent(parser, "Expected '=' after type hint in assignment");
        }
    }
    
    parseExpression(parser);
    
    consumeStatementTerminator(parser, "Expected newline after assignment - statements end with newline");
}

static void parseInput(Parser* parser, Token identifier) {
    // Should already be at 'input' keyword
    consume(parser, TOKEN_INPUT, "Expected 'input' function");
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'input'");
    
    // Optional prompt
    if (match(parser, TOKEN_STRING)) {
        // Prompt consumed
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after input arguments");
    consumeStatementTerminator(parser, "Expected newline after input statement");
}

static void parseOutput(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'output'");
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            parseExpression(parser);
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after output arguments");
    consumeStatementTerminator(parser, "Expected newline after output statement");
}

static void parseBlock(Parser* parser) {
    // Expect INDENT
    if (!match(parser, TOKEN_INDENT)) {
        errorAtCurrent(parser, "Indentation error - expected indent after statement");
        return;
    }
    
    // Parse statements until DEDENT or EOF
    while (!check(parser, TOKEN_DEDENT) && !isAtEnd(parser)) {
        parseStatement(parser);
    }
    
    // Handle DEDENT or EOF
    if (isAtEnd(parser)) {
        if (!check(parser, TOKEN_DEDENT)) {
            return;
        }
    }
    
    // Consume DEDENT token
    if (!match(parser, TOKEN_DEDENT)) {
        errorAtCurrent(parser, "Expected dedent after block");
    }
}

static void parseConditional(Parser* parser) {
    // Optional 'then' noise word
    match(parser, TOKEN_THEN);
    
    parseExpression(parser);
    
    consume(parser, TOKEN_COLON, "Expected ':' after condition in 'when' statement");
    consumeStatementTerminator(parser, "Expected newline after ':'");
    
    parseBlock(parser);
    
    skipNewlines(parser);
    
    // Handle else-when and else
    if (match(parser, TOKEN_ELSE)) {
        if (match(parser, TOKEN_WHEN)) {
            // Recursive else-when (elif chain)
            parseConditional(parser);
        } else {
            // Final else
            consume(parser, TOKEN_COLON, "Expected ':' after 'else'");
            consumeStatementTerminator(parser, "Expected newline after ':'");
            parseBlock(parser);
        }
    }
}

static void parseWhileLoop(Parser* parser) {
    parseExpression(parser);
    
    consume(parser, TOKEN_COLON, "Expected ':' after while condition");
    consumeStatementTerminator(parser, "Expected newline after ':'");
    
    parseBlock(parser);
}

static void parseForLoop(Parser* parser) {
    // Optional 'each' noise word
    match(parser, TOKEN_EACH);
    
    consume(parser, TOKEN_IDENTIFIER, "Expected iterator variable in for loop");
    
    consume(parser, TOKEN_IN, "Expected 'in' in for loop");
    
    parseExpression(parser);
    
    consume(parser, TOKEN_COLON, "Expected ':' after for loop header");
    consumeStatementTerminator(parser, "Expected newline after ':'");
    
    parseBlock(parser);
}

static void parseStatement(Parser* parser) {
    skipNewlines(parser);
    
    if (isAtEnd(parser)) return;
    
    // Skip comments
    if (match(parser, TOKEN_COMMENT_LINE) || match(parser, TOKEN_COMMENT_BLOCK)) {
        return;
    }
    
    // Declaration
    if (match(parser, TOKEN_FLEX) || match(parser, TOKEN_FIXED)) {
        parseDeclaration(parser);
        parser->statementCount++;
        return;
    }
    
    // Output
    if (match(parser, TOKEN_OUTPUT)) {
        parseOutput(parser);
        parser->statementCount++;
        return;
    }
    
    // Conditional
    if (match(parser, TOKEN_WHEN)) {
        parseConditional(parser);
        parser->statementCount++;
        return;
    }
    
    // While loop
    if (match(parser, TOKEN_WHILE)) {
        parseWhileLoop(parser);
        parser->statementCount++;
        return;
    }
    
    // For loop
    if (match(parser, TOKEN_FOR)) {
        parseForLoop(parser);
        parser->statementCount++;
        return;
    }
    
    // Break
    if (match(parser, TOKEN_BREAK)) {
        consumeStatementTerminator(parser, "Expected newline after 'break'");
        parser->statementCount++;
        return;
    }
    
    // Continue
    if (match(parser, TOKEN_CONTINUE)) {
        consumeStatementTerminator(parser, "Expected newline after 'continue'");
        parser->statementCount++;
        return;
    }
    
    // Return
    if (match(parser, TOKEN_RETURN)) {
        if (!check(parser, TOKEN_NEWLINE) && !check(parser, TOKEN_EOF) && !check(parser, TOKEN_DEDENT)) {
            parseExpression(parser);
        }
        
        consumeStatementTerminator(parser, "Expected newline after return");
        parser->statementCount++;
        return;
    }
    
    // Identifier (assignment or input)
    if (match(parser, TOKEN_IDENTIFIER)) {
        Token idToken = parser->previous;
        
        // Check for compound assignment operators
        if (check(parser, TOKEN_PLUS_EQUAL) || check(parser, TOKEN_MINUS_EQUAL) ||
            check(parser, TOKEN_STAR_EQUAL) || check(parser, TOKEN_SLASH_EQUAL) ||
            check(parser, TOKEN_PERCENT_EQUAL)) {
            parseAssignment(parser, idToken);
            parser->statementCount++;
            return;
        }
        
        // Check for regular assignment
        if (check(parser, TOKEN_EQUAL)) {
            advance(parser);  // Consume the '='
            
            // Check for input statement
            if (check(parser, TOKEN_INPUT)) {
                parseInput(parser, idToken);
                parser->statementCount++;
                return;
            }
            
            // Regular assignment - but first check for 'as' type cast
            if (check(parser, TOKEN_AS)) {
                // Backtrack: this is "id = as type = expr" pattern
                // We need to handle this in parseAssignment
                // For now, just parse the expression
            }
            
            parseExpression(parser);
            consumeStatementTerminator(parser, "Expected newline after assignment");
            parser->statementCount++;
            return;
        }
        
        // Check for 'as' type cast assignment (id as type = expr)
        if (check(parser, TOKEN_AS)) {
            advance(parser);  // Consume 'as'
            
            // Expect type hint
            if (!match(parser, TOKEN_HINT_INT) && !match(parser, TOKEN_HINT_FLOAT) &&
                !match(parser, TOKEN_HINT_STR) && !match(parser, TOKEN_HINT_BOOL) &&
                !match(parser, TOKEN_HINT_CHAR)) {
                errorAtCurrent(parser, "Expected type hint after 'as' keyword");
            }
            
            // Now expect '='
            consume(parser, TOKEN_EQUAL, "Expected '=' after type hint in assignment");
            
            parseExpression(parser);
            consumeStatementTerminator(parser, "Expected newline after assignment");
            parser->statementCount++;
            return;
        }
        
        // If we get here, it's an invalid statement
        errorAtCurrent(parser, "Expected assignment operator or statement after identifier");
        synchronize(parser);
        return;
    }
    
    errorAtCurrent(parser, "Expected statement");
    synchronize(parser);
}

// ===== PUBLIC API =====

bool parse(Parser* parser) {
    if (parser == NULL) return false;
    
    printf("\n=== Starting EaC Syntax Analysis ===\n");
    printf("Algorithm: Recursive Descent Parsing\n");
    printf("Method: Panic Mode Error Recovery with Synchronization Tokens\n");
    printf("Validating syntax without AST construction...\n\n");
    
    while (!isAtEnd(parser)) {
        parseStatement(parser);
    }
    
    consume(parser, TOKEN_EOF, "Expected end of file");
    
    printf("\n=== Syntax Analysis Complete ===\n");
    if (parser->hadError) {
        printf("Status: FAILED - Syntax errors found\n");
        printf("Error Recovery: Panic mode with synchronization\n");
    } else {
        printf("Status: SUCCESS - No syntax errors\n");
        printf("Total statements validated: %d\n", parser->statementCount);
    }
    
    return !parser->hadError;
}

void freeParser(Parser* parser) {
    if (parser != NULL) {
        free(parser);
    }
}