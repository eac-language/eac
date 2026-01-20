#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

FILE* g_parserErrorLog = NULL;

void setParserErrorLog(FILE* logFile) {
    g_parserErrorLog = logFile;
}

struct Parser {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

// ===== Helper Functions =====

static bool check(Parser* parser, TokenType type);
static void errorAtCurrent(Parser* parser, const char* message);

static void checkStatementEnd(Parser* parser) {
    if (check(parser, TOKEN_NEWLINE) || check(parser, TOKEN_EOF) || check(parser, TOKEN_DEDENT)) {
        return;
    }
    errorAtCurrent(parser, "Expected newline or end of statement");
}

static void errorAt(Parser* parser, Token* token, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
    parser->hadError = true;
    
    char errorMsg[512];
    int offset = 0;
    
    offset += snprintf(errorMsg + offset, sizeof(errorMsg) - offset, 
                      "Error at line %d", token->line);
    
    if (token->type == TOKEN_EOF) {
        offset += snprintf(errorMsg + offset, sizeof(errorMsg) - offset, " at end");
    } else {
        offset += snprintf(errorMsg + offset, sizeof(errorMsg) - offset, 
                          ", column %d", token->column);
    }
    
    offset += snprintf(errorMsg + offset, sizeof(errorMsg) - offset, 
                      ": %s\n", message);
    
    fprintf(stderr, "%s", errorMsg);
    
    if (g_parserErrorLog) {
        fprintf(g_parserErrorLog, "%s", errorMsg);
        fflush(g_parserErrorLog);
    }
    
    // Show the problematic line context
    if (token->type != TOKEN_EOF && token->type != TOKEN_ERROR) {
        char contextMsg[256];
        offset = 0;
        
        offset += snprintf(contextMsg + offset, sizeof(contextMsg) - offset,
                          "  %.*s\n", token->length, token->lexeme);
        offset += snprintf(contextMsg + offset, sizeof(contextMsg) - offset, "  ");
        
        for (int i = 0; i < token->length && i < 5; i++) {
            offset += snprintf(contextMsg + offset, sizeof(contextMsg) - offset, "^");
        }
        offset += snprintf(contextMsg + offset, sizeof(contextMsg) - offset, "\n");
        
        fprintf(stderr, "%s", contextMsg);
        
        if (g_parserErrorLog) {
            fprintf(g_parserErrorLog, "%s", contextMsg);
            fflush(g_parserErrorLog);
        }
    }
}

static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = getNextToken(parser->lexer);
        
        if (parser->current.type == TOKEN_COMMENT_LINE ||
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
                ; 
        }
        
        advance(parser);
    }
}

static void skipNewlines(Parser* parser) {
    while (match(parser, TOKEN_NEWLINE)) {
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
    
    if (match(parser, TOKEN_INPUT)) {
        int line = parser->previous.line;
        consume(parser, TOKEN_LPAREN, "Expected '(' after 'input'");
        
        char* prompt = NULL;
        if (match(parser, TOKEN_STRING)) {
            prompt = parseString(parser->previous);
        }
        
        consume(parser, TOKEN_RPAREN, "Expected ')' after input arguments");
        ASTNode* node = createInputExpr(prompt, line);
        if (prompt) free(prompt);
        return node;
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = tokenToString(parser->previous);
        int line = parser->previous.line;
        
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
        ASTNode* right = power(parser);  
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

// <CAST_EXPR> → <ARITH_EXPR> | <ARITH_EXPR> as <TYPE>
static ASTNode* castExpr(Parser* parser) {
    ASTNode* expr = arithmeticExpr(parser);
    
    if (match(parser, TOKEN_AS)) {
        int line = parser->previous.line;
        
        if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
            check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL) ||
            check(parser, TOKEN_HINT_CHAR)) {
            
            TokenType targetType = parser->current.type;
            advance(parser);
            
            expr = createCastExpr(expr, targetType, line);
        } else {
            errorAtCurrent(parser, "Expected type after 'as'");
        }
    }
    
    return expr;
}

// <MEMBERSHIP_EXPR> → <CAST_EXPR> | <CAST_EXPR> in <CAST_EXPR>
static ASTNode* membershipExpr(Parser* parser) {
    ASTNode* expr = castExpr(parser);
    
    if (match(parser, TOKEN_IN)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = castExpr(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <RELATIONAL_EXPR> → <MEMBERSHIP_EXPR> <REL_OP> <MEMBERSHIP_EXPR>
static ASTNode* relationalExpr(Parser* parser) {
    ASTNode* expr = membershipExpr(parser);
    
    if (match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL) ||
        match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL) ||
        match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = membershipExpr(parser);
        expr = createBinaryOp(op, expr, right, line);
    }
    
    return expr;
}

// <BOOL_FACTOR> → <RELATIONAL_EXPR> | not <BOOL_FACTOR> | ( <LOGICAL_EXPR> ) | true | false
static ASTNode* boolFactor(Parser* parser);
static ASTNode* logicalExpr(Parser* parser);

static ASTNode* boolFactor(Parser* parser) {
    if (match(parser, TOKEN_NOT)) {
        TokenType op = parser->previous.type;
        int line = parser->previous.line;
        ASTNode* right = boolFactor(parser);
        return createUnaryOp(op, right, line);
    }
    
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

// <IMPORT_STMT> → import <id> | from <id> import <id>
static ASTNode* importStatement(Parser* parser) {
    int line = parser->previous.line;
    
    // import moduleName
    if (parser->previous.type == TOKEN_IMPORT) {
        consume(parser, TOKEN_IDENTIFIER, "Expected module name after 'import'");
        char* moduleName = tokenToString(parser->previous);
        ASTNode* node = createImportStmt(moduleName, NULL, line);
        free(moduleName);
        return node;
    } 
    
    // from moduleName import target
    consume(parser, TOKEN_IDENTIFIER, "Expected module name after 'from'");
    char* fromModule = tokenToString(parser->previous);
    
    consume(parser, TOKEN_IMPORT, "Expected 'import' after module name");
    consume(parser, TOKEN_IDENTIFIER, "Expected identifier to import");
    char* moduleName = tokenToString(parser->previous);
    
    ASTNode* node = createImportStmt(moduleName, fromModule, line);
    free(fromModule);
    free(moduleName);
    checkStatementEnd(parser);
    return node;
}

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
    
    checkStatementEnd(parser);
    return createOutputStmt(expressions, line);
}

// <WHEN_STMT> → when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
// <WHEN_ELSE_STMT> → ... else : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* conditionalStatement(Parser* parser) {
    int line = parser->previous.line;
    
    // Rule 44: when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
    ASTNode* cond = condition(parser);
    
    match(parser, TOKEN_THEN); 
    
    consume(parser, TOKEN_COLON, "Expected ':' after condition in 'when' statement");
    consume(parser, TOKEN_NEWLINE, "Expected newline after ':' in 'when' statement");
    consume(parser, TOKEN_INDENT, "Expected indentation after 'when:'");
    
    ASTNodeList* thenBranch = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after 'when' block");
    
    ASTNodeList* elseBranch = NULL;
    
    skipNewlines(parser);
    
    while (match(parser, TOKEN_ELSE)) {
        if (match(parser, TOKEN_WHEN)) {
            // Rule 50-51: else when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
            int elwhenLine = parser->previous.line;
            ASTNode* elwhenCond = condition(parser);
            
            match(parser, TOKEN_THEN); 
            
            consume(parser, TOKEN_COLON, "Expected ':' after condition in 'else when' statement");
            consume(parser, TOKEN_NEWLINE, "Expected newline after ':' in 'else when' statement");
            consume(parser, TOKEN_INDENT, "Expected indentation after 'else when:'");
            
            ASTNodeList* elwhenBody = statements(parser);
            
            consume(parser, TOKEN_DEDENT, "Expected dedent after 'else when' block");
            
            ASTNode* elwhenStmt = createIfStmt(elwhenCond, elwhenBody, NULL, elwhenLine);
            
            if (elseBranch == NULL) {
                elseBranch = createNodeList();
            }
            addNode(elseBranch, elwhenStmt);
            
            skipNewlines(parser);
        } else {
            // Rule 46: else : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
            consume(parser, TOKEN_COLON, "Expected ':' after 'else'");
            consume(parser, TOKEN_NEWLINE, "Expected newline after ':' in 'else' statement");
            consume(parser, TOKEN_INDENT, "Expected indentation after 'else:'");
            
            elseBranch = statements(parser);
            
            consume(parser, TOKEN_DEDENT, "Expected dedent after 'else' block");
            break;
        }
    }
    
    return createIfStmt(cond, thenBranch, elseBranch, line);
}

// <WHILE_STMT> → while <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* whileStatement(Parser* parser) {
    int line = parser->previous.line;
    
    // Rule 60: while <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
    ASTNode* cond = condition(parser);
    consume(parser, TOKEN_COLON, "Expected ':' after condition in 'while' statement");
    consume(parser, TOKEN_NEWLINE, "Expected newline after ':' in 'while' statement");
    consume(parser, TOKEN_INDENT, "Expected indentation after 'while:'");
    
    ASTNodeList* body = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after 'while' block");
    
    return createWhileStmt(cond, body, line);
}

// <FOR_STMT> → for <id> in <ITERABLE> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
static ASTNode* forStatement(Parser* parser) {
    int line = parser->previous.line;
    
    match(parser, TOKEN_EACH); 
    
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
        
        if (strcmp(name, "range") == 0 && match(parser, TOKEN_LPAREN)) {
            ASTNodeList* args = createNodeList();
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    addNode(args, expression(parser));
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expected ')' after range arguments");
            
            ASTNode* argList = createArgList(args, nameToken.line);
            iterable = createCallExpr("range", argList, nameToken.line);
        } else {
            iterable = createIdentifier(name, nameToken.line);
        }
        free(name);
    } else if (match(parser, TOKEN_LBRACKET)) {
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
    
    // Rule 61-62: for <id> in <ITERABLE> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
    consume(parser, TOKEN_COLON, "Expected ':' after iterable");
    consume(parser, TOKEN_NEWLINE, "Expected newline after ':' in 'for' statement");
    consume(parser, TOKEN_INDENT, "Expected indentation after 'for:'");
    
    ASTNodeList* body = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after 'for' block");
    
    ASTNode* node = createForStmt(iterVar, iterable, body, line);
    free(iterVar);
    return node;
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
    
    if (match(parser, TOKEN_OF) || match(parser, TOKEN_AS)) {
        if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
            check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL) ||
            check(parser, TOKEN_HINT_CHAR)) {
            TokenType hintType = parser->current.type;
            advance(parser);
            typeHint = createTypeHint(hintType, parser->previous.line);
        } else {
            errorAtCurrent(parser, "Expected type hint after 'of' or 'as'");
        }
    }
    else if (match(parser, TOKEN_COLON)) {
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
    
    if (match(parser, TOKEN_TO)) {
        initializer = expression(parser);
    }
    else if (match(parser, TOKEN_EQUAL)) {
        initializer = expression(parser);
    }
    
    ASTNode* node = createVarDecl(isMutable, name, typeHint, initializer, line);
    free(name);
    checkStatementEnd(parser);
    return node;
}

// Function Declaration
static ASTNode* functionDeclaration(Parser* parser) {
    int line = parser->previous.line;
    
    consume(parser, TOKEN_IDENTIFIER, "Expected function name.");
    char* name = tokenToString(parser->previous);
    
    consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
    ASTNodeList* paramsList = createNodeList();
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.");
            char* paramName = tokenToString(parser->previous);
            int paramLine = parser->previous.line;
            
            ASTNode* paramType = NULL;
            if (match(parser, TOKEN_COLON)) {
                 if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
                     check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL)) {
                    TokenType type = parser->current.type;
                    advance(parser);
                    paramType = createTypeHint(type, paramLine);
                } else {
                    errorAtCurrent(parser, "Expected type hint for parameter.");
                }
            }
            
            ASTNode* param = createVarDecl(false, paramName, paramType, NULL, paramLine);
            addNode(paramsList, param);
            free(paramName);
            
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
    ASTNode* paramsNode = createParamList(paramsList, line);

    ASTNode* returnType = NULL;
    if (check(parser, TOKEN_COLON)) {
        consume(parser, TOKEN_COLON, "Expected ':'");
        
        if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
            check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL) || 
            check(parser, TOKEN_HINT_CHAR)) {
            
            TokenType type = parser->current.type;
            advance(parser);
            returnType = createTypeHint(type, line);
            
            consume(parser, TOKEN_COLON, "Expected ':' before function body.");
        } 
    } else {
        consume(parser, TOKEN_COLON, "Expected ':' before function body.");
    }

    consume(parser, TOKEN_NEWLINE, "Expected newline before function body.");
    consume(parser, TOKEN_INDENT, "Expected indentation for function body.");
    
    ASTNodeList* body = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after function body.");
    
    ASTNode* funcNode = createFuncDecl(name, paramsNode, returnType, body, line);
    free(name);
    return funcNode;
}

static ASTNode* returnStatement(Parser* parser) {
    int line = parser->previous.line;
    ASTNode* value = NULL;

    if (!check(parser, TOKEN_NEWLINE) && !check(parser, TOKEN_DEDENT) && !check(parser, TOKEN_EOF)) {
        value = expression(parser);
    }
    
    checkStatementEnd(parser);
    return createReturnStmt(value, line);
}

// <STATEMENT> → <FUNCTION_STMT> | <DECL_STMT> | <ASS_STMT> | <INPUT_STMT> | <OUTPUT_STMT> | <COND_STMT> | <BREAK_STMT> | <RETURN_STMT> | <ITER_STMT>
static ASTNode* statement(Parser* parser) {
    skipNewlines(parser);

    if (match(parser, TOKEN_IMPORT) || match(parser, TOKEN_FROM)) {
        return importStatement(parser);
    }
    
    if (match(parser, TOKEN_FUNCTION)) {
        return functionDeclaration(parser);
    }

    if (match(parser, TOKEN_RETURN)) {
        return returnStatement(parser);
    }
    
    if (match(parser, TOKEN_FLEX) || match(parser, TOKEN_FIXED)) {
        return declarationStatement(parser);
    }
    
    if (match(parser, TOKEN_OUTPUT)) {
        return outputStatement(parser);
    }
    
    if (match(parser, TOKEN_WHEN)) {
        return conditionalStatement(parser);
    }

    if (match(parser, TOKEN_BREAK)) {
        ASTNode* node = createBreakStmt(parser->previous.line);
        checkStatementEnd(parser);
        return node;
    }

    if (match(parser, TOKEN_CONTINUE)) {
        ASTNode* node = createContinueStmt(parser->previous.line);
        checkStatementEnd(parser);
        return node;
    }
    
    if (match(parser, TOKEN_WHILE)) {
        return whileStatement(parser);
    }
    
    if (match(parser, TOKEN_FOR)) {
        return forStatement(parser);
    }
    
    // <ASS_STMT> → <id> <ASSIGN_OP> <expr>
    // <ASSIGN_OP> → = | += | -= | *= | /= | %=
    // <INPUT_STMT> → <id> = input () | <id> = input ( <STRING_LITERAL> )
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = tokenToString(parser->previous);
        int line = parser->previous.line;
        
        if (check(parser, TOKEN_EQUAL)) {
            advance(parser);
            
            if (check(parser, TOKEN_INPUT)) {
                advance(parser);
                consume(parser, TOKEN_LPAREN, "Expected '(' after 'input'");
                
                char* prompt = NULL;
                if (match(parser, TOKEN_STRING)) {
                    prompt = parseString(parser->previous);
                }
                
                consume(parser, TOKEN_RPAREN, "Expected ')' after input arguments");
                
                ASTNode* node = createInputStmt(name, prompt, line);
                free(name);
                if (prompt) free(prompt);
                checkStatementEnd(parser);
                return node;
            } else {
                
                ASTNode* value = expression(parser);
                ASTNode* node = createAssignment(name, value, line);
                free(name);
                checkStatementEnd(parser);
                return node;
            }
        }
        
        if (check(parser, TOKEN_PLUS_EQUAL) || check(parser, TOKEN_MINUS_EQUAL) ||
            check(parser, TOKEN_STAR_EQUAL) || check(parser, TOKEN_SLASH_EQUAL) ||
            check(parser, TOKEN_PERCENT_EQUAL)) {
            TokenType op = parser->current.type;
            advance(parser);
            ASTNode* value = expression(parser);
            ASTNode* node = createCompoundAssign(name, op, value, line);
            free(name);
            checkStatementEnd(parser);
            return node;
        }
        
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
            checkStatementEnd(parser);
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
    
    while (!check(parser, TOKEN_EOF) && 
           !check(parser, TOKEN_ELSE) && 
           !check(parser, TOKEN_DEDENT)) {
        
        while (match(parser, TOKEN_NEWLINE) || check(parser, TOKEN_INDENT)) {
            if (check(parser, TOKEN_INDENT)) {
                advance(parser);
            }
        }
        
        if (check(parser, TOKEN_EOF) || 
            check(parser, TOKEN_ELSE) || 
            check(parser, TOKEN_DEDENT)) {
            break;
        }
        
        ASTNode* stmt = statement(parser);
        if (stmt && !parser->panicMode) {
            addNode(stmtList, stmt);
        }
        
        if (parser->panicMode) {
            synchronize(parser);
        }
    }
    
    return stmtList;
}

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
    
    while (match(parser, TOKEN_DEDENT)) {
    }
    
    if (!match(parser, TOKEN_EOF)) {
        errorAtCurrent(parser, "Expected end of file");
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