#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

struct Parser {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

// ===== Helper Functions =====

static void errorAt(Parser* parser, Token* token, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
    parser->hadError = true;
    
    fprintf(stderr, "Error at line %d", token->line);
    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, ", column %d", token->length);
    }
    
    fprintf(stderr, ": %s\n", message);
    
    // Show the problematic line context
    if (token->type != TOKEN_EOF && token->type != TOKEN_ERROR) {
        fprintf(stderr, "  %.*s\n", token->length, token->lexeme);
        fprintf(stderr, "  ");
        for (int i = 0; i < token->length && i < 5; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
    }
}

static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = getNextToken(parser->lexer);
        
        // Skip noise words and comments (as per spec)
        if (parser->current.type == TOKEN_AS ||
            parser->current.type == TOKEN_EACH ||
            parser->current.type == TOKEN_OF ||
            parser->current.type == TOKEN_TO ||
            parser->current.type == TOKEN_THEN ||
            parser->current.type == TOKEN_COMMENT_LINE ||
            parser->current.type == TOKEN_COMMENT_BLOCK) {
            continue;
        }
        
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

static void synchronize(Parser* parser) {
    parser->panicMode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        // Synchronization tokens as per spec
        if (parser->previous.type == TOKEN_NEWLINE) return;
        if (parser->previous.type == TOKEN_DEDENT) return;
        
        switch (parser->current.type) {
            case TOKEN_FLEX:
            case TOKEN_FIXED:
            case TOKEN_WHEN:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_OUTPUT:
            case TOKEN_ELSE:
                return;
            default:
                ; // Do nothing
        }
        
        advance(parser);
    }
}

static void skipNewlines(Parser* parser) {
    while (match(parser, TOKEN_NEWLINE)) {
        // Skip
    }
}

static char* tokenToString(Token token) {
    char* str = (char*)malloc(token.length + 1);
    if (str) {
        memcpy(str, token.lexeme, token.length);
        str[token.length] = '\0';
    }
    return str;
}

static long long parseInteger(Token token) {
    char* str = tokenToString(token);
    long long value = strtoll(str, NULL, 10);
    free(str);
    return value;
}

static double parseFloat(Token token) {
    char* str = tokenToString(token);
    double value = strtod(str, NULL);
    free(str);
    return value;
}

static char* parseString(Token token) {
    if (token.length < 2) return strdup("");
    char* str = (char*)malloc(token.length - 1);
    if (str) {
        memcpy(str, token.lexeme + 1, token.length - 2);
        str[token.length - 2] = '\0';
    }
    return str;
}

static char parseChar(Token token) {
    if (token.length < 3) return '\0';
    if (token.lexeme[1] == '\\' && token.length >= 4) {
        switch (token.lexeme[2]) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '\\': return '\\';
            case '\'': return '\'';
            default: return token.lexeme[2];
        }
    }
    return token.lexeme[1];
}

// ===== Forward Declarations =====
static ASTNode* expression(Parser* parser);
static ASTNode* statement(Parser* parser);
static ASTNodeList* statements(Parser* parser);
static ASTNode* declaration(Parser* parser);

// ===== Expression Parsing (Following Grammar) =====

// <PRIMARY> → <LITERAL> | <id> | ( <ARITH_EXPR> ) | | <ARITH_EXPR> |
static ASTNode* primary(Parser* parser) {
    if (match(parser, TOKEN_TRUE)) {
        return createBoolLiteral(true, parser->previous.line);
    }
    
    if (match(parser, TOKEN_FALSE)) {
        return createBoolLiteral(false, parser->previous.line);
    }
    
    if (match(parser, TOKEN_INTEGER)) {
        long long value = parseInteger(parser->previous);
        return createIntLiteral(value, parser->previous.line);
    }
    
    if (match(parser, TOKEN_FLOAT)) {
        double value = parseFloat(parser->previous);
        return createFloatLiteral(value, parser->previous.line);
    }
    
    if (match(parser, TOKEN_STRING)) {
        char* value = parseString(parser->previous);
        ASTNode* node = createStringLiteral(value, parser->previous.line);
        free(value);
        return node;
    }
    
    if (match(parser, TOKEN_CHAR)) {
        char value = parseChar(parser->previous);
        return createCharLiteral(value, parser->previous.line);
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = tokenToString(parser->previous);
        int line = parser->previous.line;
        
        // Check for function call
        if (match(parser, TOKEN_LPAREN)) {
            ASTNodeList* args = createNodeList();
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    addNode(args, expression(parser));
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
            ASTNode* argList = createArgList(args, line);
            ASTNode* call = createCallExpr(name, argList, line);
            free(name);
            return call;
        }
        
        // Check for array indexing
        if (match(parser, TOKEN_LBRACKET)) {
            ASTNode* index = expression(parser);
            consume(parser, TOKEN_RBRACKET, "Expected ']' after index");
            ASTNode* indexExpr = createIndexExpr(name, index, line);
            free(name);
            return indexExpr;
        }
        
        ASTNode* id = createIdentifier(name, line);
        free(name);
        return id;
    }
    
    // ( <ARITH_EXPR> )
    if (match(parser, TOKEN_LPAREN)) {
        ASTNode* expr = expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    // List literal: [ <LIST_ITEMS> ]
    if (match(parser, TOKEN_LBRACKET)) {
        ASTNodeList* elements = createNodeList();
        
        if (!check(parser, TOKEN_RBRACKET)) {
            do {
                addNode(elements, expression(parser));
            } while (match(parser, TOKEN_COMMA));
        }
        
        consume(parser, TOKEN_RBRACKET, "Expected ']' after list elements");
        return createListLiteral(elements, parser->previous.line);
    }
    
    // Absolute value: | <ARITH_EXPR> |
    if (match(parser, TOKEN_VBAR)) {
        ASTNode* expr = expression(parser);
        consume(parser, TOKEN_VBAR, "Expected '|' after expression");
        return createUnaryOp(TOKEN_VBAR, expr, parser->previous.line);
    }
    
    errorAtCurrent(parser, "Expected expression");
    return NULL;
}

// <POWER> → <PRIMARY> | <PRIMARY> ^ <POWER>
static ASTNode* power(Parser* parser) {
    ASTNode* expr = primary(parser);
    
    if (match(parser, TOKEN_CARET)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = power(parser);  // Right-associative
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <FACTOR> → <POWER> | - <FACTOR>
static ASTNode* factor(Parser* parser) {
    if (match(parser, TOKEN_MINUS)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = factor(parser);
        return createUnaryOp(op, right, line);
    }
    
    return power(parser);
}

// <TERM> → <FACTOR> | <TERM> * <FACTOR> | <TERM> / <FACTOR> | <TERM> % <FACTOR> | <TERM> // <FACTOR>
static ASTNode* term(Parser* parser) {
    ASTNode* expr = factor(parser);
    
    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) ||
           match(parser, TOKEN_FLOOR_DIV) || match(parser, TOKEN_PERCENT)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = factor(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <ARITH_EXPR> → <TERM> | <ARITH_EXPR> + <TERM> | <ARITH_EXPR> - <TERM>
static ASTNode* arithmeticExpr(Parser* parser) {
    ASTNode* expr = term(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = term(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <RELATIONAL_EXPR> → <ARITH_EXPR> <REL_OP> <ARITH_EXPR>
static ASTNode* relationalExpr(Parser* parser) {
    ASTNode* expr = arithmeticExpr(parser);
    
    if (match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL) ||
        match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL) ||
        match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = arithmeticExpr(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <BOOL_FACTOR> → <RELATIONAL_EXPR> | not <BOOL_FACTOR> | ( <LOGICAL_EXPR> ) | true | false
static ASTNode* boolFactor(Parser* parser);
static ASTNode* logicalExpr(Parser* parser);

static ASTNode* boolFactor(Parser* parser) {
    // not <BOOL_FACTOR>
    if (match(parser, TOKEN_NOT)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = boolFactor(parser);
        return createUnaryOp(op, right, line);
    }
    
    // ( <LOGICAL_EXPR> )
    if (check(parser, TOKEN_LPAREN)) {
        int savedLine = parser->current.line;
        advance(parser);
        ASTNode* expr = logicalExpr(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after logical expression");
        return expr;
    }
    
    // true | false | <RELATIONAL_EXPR>
    return relationalExpr(parser);
}

// <BOOL_TERM> → <BOOL_FACTOR> | <BOOL_TERM> and <BOOL_FACTOR>
static ASTNode* boolTerm(Parser* parser) {
    ASTNode* expr = boolFactor(parser);
    
    while (match(parser, TOKEN_AND)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = boolFactor(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <LOGICAL_EXPR> → <BOOL_TERM> | <LOGICAL_EXPR> or <BOOL_TERM>
static ASTNode* logicalExpr(Parser* parser) {
    ASTNode* expr = boolTerm(parser);
    
    while (match(parser, TOKEN_OR)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = boolTerm(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <CONDITION> → <BOOL_EXPR> | ( <BOOL_EXPR> )
static ASTNode* condition(Parser* parser) {
    return logicalExpr(parser);
}

// <expr> → <ARITH_EXPR> | <BOOL_EXPR> | <LITERAL> | <id>
static ASTNode* expression(Parser* parser) {
    return logicalExpr(parser);
}

// ===== Statement Parsing =====

// <OUTPUT_STMT> → output ( <OUTPUT_ARGS> )
static ASTNode* outputStatement(Parser* parser) {
    int line = parser->previous.line;
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'output'");
    
    ASTNodeList* expressions = createNodeList();
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            addNode(expressions, expression(parser));
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after output arguments");
    
    return createOutputStmt(expressions, line);
}

// <INPUT_STMT> → <id> = input ( ) | <id> = input ( <STRING_LITERAL> )
static ASTNode* inputStatement(Parser* parser, char* varName, int line) {
    consume(parser, TOKEN_EQUAL, "Expected '=' in input statement");
    consume(parser, TOKEN_INPUT, "Expected 'input' keyword");
    consume(parser, TOKEN_LPAREN, "Expected '(' after 'input'");
    
    char* prompt = NULL;
    if (match(parser, TOKEN_STRING)) {
        prompt = parseString(parser->previous);
    }
    
    consume(parser, TOKEN_RPAREN, "Expected ')' after input arguments");
    
    ASTNode* node = createInputStmt(varName, prompt, line);
    free(prompt);
    return node;
}

// <WHEN_STMT> → when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
// <WHEN_ELSE_STMT> → ... else : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* conditionalStatement(Parser* parser) {
    int line = parser->previous.line;
    
    ASTNode* cond = condition(parser);
    consume(parser, TOKEN_COLON, "Expected ':' after condition in 'when' statement");
    
    if (!match(parser, TOKEN_NEWLINE)) {
        errorAtCurrent(parser, "Expected newline after ':' in 'when' statement");
    }
    
    // Note: INDENT/DEDENT tokens should come from lexer for proper indentation handling
    skipNewlines(parser);
    
    ASTNodeList* thenBranch = statements(parser);
    ASTNodeList* elseBranch = NULL;
    
    skipNewlines(parser);
    
    // Handle else when (elwhen) chain
    while (match(parser, TOKEN_ELSE)) {
        if (match(parser, TOKEN_WHEN)) {
            // else when case - create nested if statement
            int elwhenLine = parser->previous.line;
            ASTNode* elwhenCond = condition(parser);
            consume(parser, TOKEN_COLON, "Expected ':' after condition in 'else when' statement");
            
            if (!match(parser, TOKEN_NEWLINE)) {
                errorAtCurrent(parser, "Expected newline after ':' in 'else when' statement");
            }
            
            skipNewlines(parser);
            ASTNodeList* elwhenBody = statements(parser);
            skipNewlines(parser);
            
            // Create nested if for else when
            ASTNode* elwhenStmt = createIfStmt(elwhenCond, elwhenBody, NULL, elwhenLine);
            
            if (elseBranch == NULL) {
                elseBranch = createNodeList();
            }
            addNode(elseBranch, elwhenStmt);
        } else {
            // Final else case
            consume(parser, TOKEN_COLON, "Expected ':' after 'else'");
            
            if (!match(parser, TOKEN_NEWLINE)) {
                errorAtCurrent(parser, "Expected newline after ':' in 'else' statement");
            }
            
            skipNewlines(parser);
            elseBranch = statements(parser);
            break;
        }
    }
    
    return createIfStmt(cond, thenBranch, elseBranch, line);
}

// <WHILE_STMT> → while <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* whileStatement(Parser* parser) {
    int line = parser->previous.line;
    
    ASTNode* cond = condition(parser);
    consume(parser, TOKEN_COLON, "Expected ':' after condition in 'while' statement");
    
    if (!match(parser, TOKEN_NEWLINE)) {
        errorAtCurrent(parser, "Expected newline after ':'");
    }
    
    skipNewlines(parser);
    ASTNodeList* body = statements(parser);
    
    return createWhileStmt(cond, body, line);
}

// <FOR_STMT> → for <id> in <ITERABLE> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* forStatement(Parser* parser) {
    int line = parser->previous.line;
    
    if (!match(parser, TOKEN_IDENTIFIER)) {
        errorAtCurrent(parser, "Expected variable name after 'for'");
        return NULL;
    }
    
    char* iterVar = tokenToString(parser->previous);
    
    consume(parser, TOKEN_IN, "Expected 'in' after loop variable");
    
    // <ITERABLE> → <id> | range ( <expr> ) | [ <LIST_ITEMS> ]
    ASTNode* iterable = NULL;
    
    if (check(parser, TOKEN_IDENTIFIER)) {
        Token nameToken = parser->current;
        advance(parser);
        char* name = tokenToString(nameToken);
        
        // Check for range(expr)
        if (strcmp(name, "range") == 0 && match(parser, TOKEN_LPAREN)) {
            ASTNode* rangeArg = expression(parser);
            consume(parser, TOKEN_RPAREN, "Expected ')' after range argument");
            
            // Create a call expression for range
            ASTNodeList* args = createNodeList();
            addNode(args, rangeArg);
            ASTNode* argList = createArgList(args, nameToken.line);
            iterable = createCallExpr("range", argList, nameToken.line);
        } else {
            iterable = createIdentifier(name, nameToken.line);
        }
        free(name);
    } else if (match(parser, TOKEN_LBRACKET)) {
        // List literal
        ASTNodeList* elements = createNodeList();
        
        if (!check(parser, TOKEN_RBRACKET)) {
            do {
                addNode(elements, expression(parser));
            } while (match(parser, TOKEN_COMMA));
        }
        
        consume(parser, TOKEN_RBRACKET, "Expected ']' after list elements");
        iterable = createListLiteral(elements, parser->previous.line);
    } else {
        errorAtCurrent(parser, "Expected iterable after 'in'");
        free(iterVar);
        return NULL;
    }
    
    consume(parser, TOKEN_COLON, "Expected ':' after iterable");
    
    if (!match(parser, TOKEN_NEWLINE)) {
        errorAtCurrent(parser, "Expected newline after ':'");
    }
    
    skipNewlines(parser);
    ASTNodeList* body = statements(parser);
    
    ASTNode* node = createForStmt(iterVar, iterable, body, line);
    free(iterVar);
    return node;
}

// <ASS_STMT> → <id> <ASSIGN_OP> <expr>
static ASTNode* assignmentStatement(Parser* parser, char* varName, int line) {
    // Check for compound assignment operators
    if (match(parser, TOKEN_PLUS_EQUAL) || match(parser, TOKEN_MINUS_EQUAL) ||
        match(parser, TOKEN_STAR_EQUAL) || match(parser, TOKEN_SLASH_EQUAL) ||
        match(parser, TOKEN_PERCENT_EQUAL)) {
        TokenType op = parser->previous.type;
        ASTNode* value = expression(parser);
        return createCompoundAssign(varName, op, value, line);
    }
    
    // Regular assignment
    if (match(parser, TOKEN_EQUAL)) {
        ASTNode* value = expression(parser);
        return createAssignment(varName, value, line);
    }
    
    errorAtCurrent(parser, "Expected assignment operator");
    return NULL;
}

// <DECL_STMT> → <VAR_TYPE> <id> [<TYPE_HINT>] [<ASSIGN> <expr>]
static ASTNode* declarationStatement(Parser* parser) {
    bool isMutable = parser->previous.type == TOKEN_FLEX;
    int line = parser->previous.line;
    
    if (!match(parser, TOKEN_IDENTIFIER)) {
        errorAtCurrent(parser, "Expected variable name after variable type");
        return NULL;
    }
    
    char* name = tokenToString(parser->previous);
    ASTNode* typeHint = NULL;
    ASTNode* initializer = NULL;
    
    // Optional type hint: : <DATA_TYPE>
    if (match(parser, TOKEN_COLON)) {
        if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
            check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL) ||
            check(parser, TOKEN_HINT_CHAR)) {
            TokenType hintType = parser->current.type;
            advance(parser);
            typeHint = createTypeHint(hintType, parser->previous.line);
        } else {
            errorAtCurrent(parser, "Invalid type hint - expected 'int', 'float', 'str', 'bool', or 'char'");
        }
    }
    
    // Optional initializer: <ASSIGN> <expr>
    if (match(parser, TOKEN_EQUAL)) {
        initializer = expression(parser);
    }
    
    ASTNode* node = createVarDecl(isMutable, name, typeHint, initializer, line);
    free(name);
    return node;
}

// <STATEMENT> → <DECL_STMT> | <ASS_STMT> | <INPUT_STMT> | <OUTPUT_STMT> | <COND_STMT> | <ITER_STMT>
static ASTNode* statement(Parser* parser) {
    skipNewlines(parser);
    
    // <DECL_STMT>
    if (match(parser, TOKEN_FLEX) || match(parser, TOKEN_FIXED)) {
        return declarationStatement(parser);
    }
    
    // <OUTPUT_STMT>
    if (match(parser, TOKEN_OUTPUT)) {
        return outputStatement(parser);
    }
    
    // <COND_STMT>
    if (match(parser, TOKEN_WHEN)) {
        return conditionalStatement(parser);
    }
    
    // <ITER_STMT>
    if (match(parser, TOKEN_WHILE)) {
        return whileStatement(parser);
    }
    
    if (match(parser, TOKEN_FOR)) {
        return forStatement(parser);
    }
    
    // <ASS_STMT> or <INPUT_STMT> (both start with identifier)
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = tokenToString(parser->previous);
        int line = parser->previous.line;
        
        // Check for input statement: <id> = input(...)
        if (check(parser, TOKEN_EQUAL)) {
            Token nextToken = parser->current;
            advance(parser); // consume =
            
            if (check(parser, TOKEN_INPUT)) {
                advance(parser); // consume input
                consume(parser, TOKEN_LPAREN, "Expected '(' after 'input'");
                
                char* prompt = NULL;
                if (match(parser, TOKEN_STRING)) {
                    prompt = parseString(parser->previous);
                }
                
                consume(parser, TOKEN_RPAREN, "Expected ')' after input arguments");
                
                ASTNode* node = createInputStmt(name, prompt, line);
                free(name);
                free(prompt);
                return node;
            } else {
                // Regular assignment
                ASTNode* value = expression(parser);
                ASTNode* node = createAssignment(name, value, line);
                free(name);
                return node;
            }
        }
        
        // Compound assignment
        if (check(parser, TOKEN_PLUS_EQUAL) || check(parser, TOKEN_MINUS_EQUAL) ||
            check(parser, TOKEN_STAR_EQUAL) || check(parser, TOKEN_SLASH_EQUAL) ||
            check(parser, TOKEN_PERCENT_EQUAL)) {
            TokenType op = parser->current.type;
            advance(parser);
            ASTNode* value = expression(parser);
            ASTNode* node = createCompoundAssign(name, op, value, line);
            free(name);
            return node;
        }
        
        // Expression statement (like function call)
        if (match(parser, TOKEN_LPAREN)) {
            ASTNodeList* args = createNodeList();
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    addNode(args, expression(parser));
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expected ')' after arguments");
            ASTNode* argList = createArgList(args, line);
            ASTNode* call = createCallExpr(name, argList, line);
            free(name);
            return createExprStmt(call, line);
        }
        
        errorAtCurrent(parser, "Expected assignment or function call");
        free(name);
        return NULL;
    }
    
    errorAtCurrent(parser, "Expected statement");
    return NULL;
}

// <STATEMENTS> → <STATEMENT> | <STATEMENT> <NEWLINE> <STATEMENTS>
static ASTNodeList* statements(Parser* parser) {
    ASTNodeList* stmtList = createNodeList();
    
    while (!check(parser, TOKEN_EOF) && !check(parser, TOKEN_ELSE)) {
        skipNewlines(parser);
        
        if (check(parser, TOKEN_EOF) || check(parser, TOKEN_ELSE)) {
            break;
        }
        
        ASTNode* stmt = statement(parser);
        if (stmt) {
            addNode(stmtList, stmt);
        }
        
        if (parser->panicMode) {
            synchronize(parser);
        }
        
        // Expect newline or EOF after statement
        if (!check(parser, TOKEN_EOF) && !check(parser, TOKEN_ELSE)) {
            if (!match(parser, TOKEN_NEWLINE)) {
                // Some statements like when/while/for end with their block
                if (parser->previous.type != TOKEN_COLON) {
                    break;
                }
            }
        }
    }
    
    return stmtList;
}

// ===== Public API =====

Parser* initParser(Lexer* lexer) {
    if (!lexer) return NULL;
    
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
    
    advance(parser);
    
    return parser;
}

// <PROGRAM> → <STATEMENTS>
ASTNode* parse(Parser* parser) {
    if (!parser) return NULL;
    
    ASTNodeList* stmts = statements(parser);
    
    if (!match(parser, TOKEN_EOF)) {
        errorAtCurrent(parser, "Expected end of file");
    }
    
    if (parser->hadError) {
        freeNodeList(stmts);
        return NULL;
    }
    
    return createProgram(stmts);
}

bool hasError(Parser* parser) {
    return parser && parser->hadError;
}

void freeParser(Parser* parser) {
    if (parser) {
        free(parser);
    }
}