#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ast.h"

#define INITIAL_LIST_CAPACITY 8

// ===== Node List Management =====

ASTNodeList* createNodeList(void) {
    ASTNodeList* list = (ASTNodeList*)malloc(sizeof(ASTNodeList));
    if (!list) return NULL;
    
    list->nodes = (ASTNode**)malloc(sizeof(ASTNode*) * INITIAL_LIST_CAPACITY);
    if (!list->nodes) {
        free(list);
        return NULL;
    }
    
    list->count = 0;
    list->capacity = INITIAL_LIST_CAPACITY;
    return list;
}

void addNode(ASTNodeList* list, ASTNode* node) {
    if (!list || !node) return;
    
    if (list->count >= list->capacity) {
        int newCapacity = list->capacity * 2;
        ASTNode** newNodes = (ASTNode**)realloc(list->nodes, sizeof(ASTNode*) * newCapacity);
        if (!newNodes) return;
        list->nodes = newNodes;
        list->capacity = newCapacity;
    }
    
    list->nodes[list->count++] = node;
}

void freeNodeList(ASTNodeList* list) {
    if (!list) return;
    
    for (int i = 0; i < list->count; i++) {
        freeAST(list->nodes[i]);
    }
    free(list->nodes);
    free(list);
}

// ===== Helper for String Duplication =====

static char* dupString(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = (char*)malloc(len + 1);
    if (copy) strcpy(copy, str);
    return copy;
}

// ===== Helper to convert token type to symbol string =====
const char* getOpSymbol(TokenType op) {
    switch (op) {
        
        case TOKEN_PLUS:        return "+";
        case TOKEN_MINUS:       return "-";
        case TOKEN_STAR:        return "*";
        case TOKEN_SLASH:       return "/";
        case TOKEN_PERCENT:     return "%";
        case TOKEN_CARET:       return "^";
        case TOKEN_FLOOR_DIV:   return "//";
        
        
        case TOKEN_EQUAL_EQUAL: return "==";
        case TOKEN_BANG_EQUAL:  return "!=";
        case TOKEN_LESS:        return "<";
        case TOKEN_LESS_EQUAL:  return "<=";
        case TOKEN_GREATER:     return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        
        
        case TOKEN_AND:         return "and";
        case TOKEN_OR:          return "or";
        case TOKEN_NOT:         return "not";
        case TOKEN_IN:          return "in";  
        
        
        case TOKEN_EQUAL:       return "=";
        case TOKEN_PLUS_EQUAL:  return "+=";
        case TOKEN_MINUS_EQUAL: return "-=";
        
        default:                return "undefined";
    }
}

const char* getTypeHintName(TokenType type) {
    switch (type) {
        case TOKEN_HINT_INT:   return "int";
        case TOKEN_HINT_FLOAT: return "float";
        case TOKEN_HINT_STR:   return "str";
        case TOKEN_HINT_BOOL:  return "bool";
        case TOKEN_HINT_CHAR:  return "char";
        default:               return "unknown";
    }
}

// ===== AST Node Constructors =====

ASTNode* createProgram(ASTNodeList* statements) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_PROGRAM;
    node->line = 1;
    node->data.program.statements = statements;
    return node;
}

ASTNode* createVarDecl(bool isMutable, char* name, ASTNode* typeHint, ASTNode* initializer, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_VAR_DECL;
    node->line = line;
    node->data.varDecl.isMutable = isMutable;
    node->data.varDecl.name = dupString(name);
    node->data.varDecl.typeHint = typeHint;
    node->data.varDecl.initializer = initializer;
    return node;
}

ASTNode* createFuncDecl(char* name, ASTNode* params, ASTNode* returnType, ASTNodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_FUNCTION_DECL;
    node->line = line;
    node->data.funcDecl.name = dupString(name);
    node->data.funcDecl.params = params;
    node->data.funcDecl.returnType = returnType;
    node->data.funcDecl.body = body;
    return node;
}

ASTNode* createImportStmt(char* moduleName, char* fromModule, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_IMPORT_STMT;
    node->line = line;
    node->data.importStmt.moduleName = dupString(moduleName);
    node->data.importStmt.fromModule = fromModule ? dupString(fromModule) : NULL;
    return node;
}

ASTNode* createAssignment(char* varName, ASTNode* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_ASSIGNMENT;
    node->line = line;
    node->data.assignment.varName = dupString(varName);
    node->data.assignment.value = value;
    return node;
}

ASTNode* createCompoundAssign(char* varName, TokenType op, ASTNode* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_COMPOUND_ASSIGN;
    node->line = line;
    node->data.compoundAssign.varName = dupString(varName);
    node->data.compoundAssign.op = op;
    node->data.compoundAssign.value = value;
    return node;
}

ASTNode* createOutputStmt(ASTNodeList* expressions, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_OUTPUT_STMT;
    node->line = line;
    node->data.outputStmt.expressions = expressions;
    return node;
}

ASTNode* createInputStmt(char* varName, char* prompt, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_INPUT_STMT;
    node->line = line;
    node->data.inputStmt.varName = dupString(varName);
    node->data.inputStmt.prompt = prompt ? dupString(prompt) : NULL;
    return node;
}

ASTNode* createIfStmt(ASTNode* condition, ASTNodeList* thenBranch, ASTNodeList* elseBranch, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_IF_STMT;
    node->line = line;
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = thenBranch;
    node->data.ifStmt.elseBranch = elseBranch;
    return node;
}

ASTNode* createWhileStmt(ASTNode* condition, ASTNodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_WHILE_STMT;
    node->line = line;
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}

ASTNode* createForStmt(char* iterVar, ASTNode* iterable, ASTNodeList* body, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_FOR_STMT;
    node->line = line;
    node->data.forStmt.iterVar = dupString(iterVar);
    node->data.forStmt.iterable = iterable;
    node->data.forStmt.body = body;
    return node;
}

ASTNode* createReturnStmt(ASTNode* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_RETURN_STMT;
    node->line = line;
    node->data.returnStmt.value = value;
    return node;
}

ASTNode* createBreakStmt(int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_BREAK_STMT;
    node->line = line;
    return node;
}

ASTNode* createContinueStmt(int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_CONTINUE_STMT;
    node->line = line;
    return node;
}

ASTNode* createExprStmt(ASTNode* expression, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_EXPR_STMT;
    node->line = line;
    node->data.exprStmt.expression = expression;
    return node;
}

ASTNode* createBinaryOp(TokenType op, ASTNode* left, ASTNode* right, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_BINARY_OP;
    node->line = line;
    node->data.binaryOp.op = op;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    return node;
}

ASTNode* createUnaryOp(TokenType op, ASTNode* operand, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_UNARY_OP;
    node->line = line;
    node->data.unaryOp.op = op;
    node->data.unaryOp.operand = operand;
    return node;
}

ASTNode* createCallExpr(char* funcName, ASTNode* args, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_CALL_EXPR;
    node->line = line;
    node->data.callExpr.funcName = dupString(funcName);
    node->data.callExpr.args = args;
    return node;
}

ASTNode* createIndexExpr(char* varName, ASTNode* index, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_INDEX_EXPR;
    node->line = line;
    node->data.indexExpr.varName = dupString(varName);
    node->data.indexExpr.index = index;
    return node;
}

ASTNode* createIntLiteral(long long value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LITERAL;
    node->line = line;
    node->data.literal.literalType = TOKEN_INTEGER;
    node->data.literal.value.intValue = value;
    return node;
}

ASTNode* createFloatLiteral(double value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LITERAL;
    node->line = line;
    node->data.literal.literalType = TOKEN_FLOAT;
    node->data.literal.value.floatValue = value;
    return node;
}

ASTNode* createStringLiteral(char* value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LITERAL;
    node->line = line;
    node->data.literal.literalType = TOKEN_STRING;
    node->data.literal.value.stringValue = dupString(value);
    return node;
}

ASTNode* createCharLiteral(char value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LITERAL;
    node->line = line;
    node->data.literal.literalType = TOKEN_CHAR;
    node->data.literal.value.charValue = value;
    return node;
}

ASTNode* createBoolLiteral(bool value, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LITERAL;
    node->line = line;
    node->data.literal.literalType = value ? TOKEN_TRUE : TOKEN_FALSE;
    node->data.literal.value.boolValue = value;
    return node;
}

ASTNode* createIdentifier(char* name, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_IDENTIFIER;
    node->line = line;
    node->data.identifier.name = dupString(name);
    return node;
}

ASTNode* createListLiteral(ASTNodeList* elements, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_LIST_LITERAL;
    node->line = line;
    node->data.listLiteral.elements = elements;
    return node;
}

ASTNode* createCastExpr(ASTNode* expr, TokenType targetType, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_CAST_EXPR;
    node->line = line;
    node->data.castExpr.expression = expr;
    node->data.castExpr.targetType = targetType;
    return node;
}

ASTNode* createInputExpr(char* prompt, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_INPUT_EXPR;
    node->line = line;
    node->data.inputExpr.prompt = prompt ? dupString(prompt) : NULL;
    return node;
}

ASTNode* createTypeHint(TokenType hintType, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_TYPE_HINT;
    node->line = line;
    node->data.typeHint.hintType = hintType;
    return node;
}

ASTNode* createParamList(ASTNodeList* params, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_PARAM_LIST;
    node->line = line;
    node->data.list.items = params;
    return node;
}

ASTNode* createArgList(ASTNodeList* args, int line) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = AST_ARG_LIST;
    node->line = line;
    node->data.list.items = args;
    return node;
}

// ===== Memory Management =====

void freeAST(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            freeNodeList(node->data.program.statements);
            break;
            
        case AST_VAR_DECL:
            free(node->data.varDecl.name);
            freeAST(node->data.varDecl.typeHint);
            freeAST(node->data.varDecl.initializer);
            break;
            
        case AST_FUNCTION_DECL:
            free(node->data.funcDecl.name);
            freeAST(node->data.funcDecl.params);
            freeAST(node->data.funcDecl.returnType);
            freeNodeList(node->data.funcDecl.body);
            break;
            
        case AST_IMPORT_STMT:
            free(node->data.importStmt.moduleName);
            free(node->data.importStmt.fromModule);
            break;
            
        case AST_ASSIGNMENT:
            free(node->data.assignment.varName);
            freeAST(node->data.assignment.value);
            break;
            
        case AST_COMPOUND_ASSIGN:
            free(node->data.compoundAssign.varName);
            freeAST(node->data.compoundAssign.value);
            break;
            
        case AST_OUTPUT_STMT:
            freeNodeList(node->data.outputStmt.expressions);
            break;
            
        case AST_INPUT_STMT:
            free(node->data.inputStmt.varName);
            free(node->data.inputStmt.prompt);
            break;
            
        case AST_IF_STMT:
            freeAST(node->data.ifStmt.condition);
            freeNodeList(node->data.ifStmt.thenBranch);
            freeNodeList(node->data.ifStmt.elseBranch);
            break;
            
        case AST_WHILE_STMT:
            freeAST(node->data.whileStmt.condition);
            freeNodeList(node->data.whileStmt.body);
            break;
            
        case AST_FOR_STMT:
            free(node->data.forStmt.iterVar);
            freeAST(node->data.forStmt.iterable);
            freeNodeList(node->data.forStmt.body);
            break;
            
        case AST_RETURN_STMT:
            freeAST(node->data.returnStmt.value);
            break;
            
        case AST_EXPR_STMT:
            freeAST(node->data.exprStmt.expression);
            break;
            
        case AST_BINARY_OP:
            freeAST(node->data.binaryOp.left);
            freeAST(node->data.binaryOp.right);
            break;
            
        case AST_UNARY_OP:
            freeAST(node->data.unaryOp.operand);
            break;
            
        case AST_CALL_EXPR:
            free(node->data.callExpr.funcName);
            freeAST(node->data.callExpr.args);
            break;
            
        case AST_INDEX_EXPR:
            free(node->data.indexExpr.varName);
            freeAST(node->data.indexExpr.index);
            break;
            
        case AST_LITERAL:
            if (node->data.literal.literalType == TOKEN_STRING) {
                free(node->data.literal.value.stringValue);
            }
            break;
            
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_LIST_LITERAL:
            freeNodeList(node->data.listLiteral.elements);
            break;
            
        case AST_CAST_EXPR:
            freeAST(node->data.castExpr.expression);
            break;
            
        case AST_INPUT_EXPR:
            free(node->data.inputExpr.prompt);
            break;
            
        case AST_PARAM_LIST:
        case AST_ARG_LIST:
            freeNodeList(node->data.list.items);
            break;
            
        default:
            break;
    }
    
    free(node);
}

// ===== AST Visualization =====

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static void printIndentToFile(int indent, FILE* file) {
    for (int i = 0; i < indent; i++) {
        fprintf(file, "  ");
    }
}

void printAST(ASTNode* node, int indent) {
    if (!node) {
        printIndent(indent);
        printf("(null)\n");
        return;
    }
    
    printIndent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            if (node->data.program.statements) {
                for (int i = 0; i < node->data.program.statements->count; i++) {
                    printAST(node->data.program.statements->nodes[i], indent + 1);
                }
            }
            break;

        case AST_CONTINUE_STMT:
            printf("CONTINUE\n");
            break;

        case AST_IMPORT_STMT:
            if (node->data.importStmt.fromModule) {
                printf("IMPORT %s FROM %s\n", 
                       node->data.importStmt.moduleName, 
                       node->data.importStmt.fromModule);
            } else {
                printf("IMPORT %s\n", node->data.importStmt.moduleName);
            }
            break;
        
        case AST_BREAK_STMT:
            printf("BREAK\n");
            break;

        case AST_VAR_DECL:
            printf("VAR_DECL (%s) %s\n", 
                   node->data.varDecl.isMutable ? "flex" : "fixed",
                   node->data.varDecl.name);
            if (node->data.varDecl.typeHint) {
                printAST(node->data.varDecl.typeHint, indent + 1);
            }
            if (node->data.varDecl.initializer) {
                printAST(node->data.varDecl.initializer, indent + 1);
            }
            break;
            
        case AST_ASSIGNMENT:
            printf("ASSIGNMENT %s =\n", node->data.assignment.varName);
            printAST(node->data.assignment.value, indent + 1);
            break;
            
        case AST_BINARY_OP:
            printf("BINARY_OP\n");
            printAST(node->data.binaryOp.left, indent + 1);
            printIndent(indent + 1);
            printf("OP: %s\n", getOpSymbol(node->data.binaryOp.op));
            printAST(node->data.binaryOp.right, indent + 1);
            break;
            
        case AST_LITERAL:
            printf("LITERAL ");
            if (node->data.literal.literalType == TOKEN_INTEGER) {
                printf("(int) %lld\n", node->data.literal.value.intValue);
            } else if (node->data.literal.literalType == TOKEN_FLOAT) {
                printf("(float) %f\n", node->data.literal.value.floatValue);
            } else if (node->data.literal.literalType == TOKEN_STRING) {
                printf("(string) \"%s\"\n", node->data.literal.value.stringValue);
            } else if (node->data.literal.literalType == TOKEN_CHAR) {
                printf("(char) '%c'\n", node->data.literal.value.charValue);
            } else if (node->data.literal.literalType == TOKEN_TRUE || 
                       node->data.literal.literalType == TOKEN_FALSE) {
                printf("(bool) %s\n", node->data.literal.value.boolValue ? "true" : "false");
            }
            break;
            
        case AST_IDENTIFIER:
            printf("IDENTIFIER %s\n", node->data.identifier.name);
            break;

        case AST_FUNCTION_DECL:
            printf("FUNCTION_DECL %s\n", node->data.funcDecl.name);
            
            if (node->data.funcDecl.params) {
                printIndent(indent + 1);
                printf("PARAMS\n");
                printAST(node->data.funcDecl.params, indent + 2);
            }
            
            if (node->data.funcDecl.returnType) {
                printIndent(indent + 1);
                printf("RETURN TYPE\n");
                printAST(node->data.funcDecl.returnType, indent + 2);
            }
            
            if (node->data.funcDecl.body) {
                printIndent(indent + 1);
                printf("BODY\n");
                for (int i = 0; i < node->data.funcDecl.body->count; i++) {
                    printAST(node->data.funcDecl.body->nodes[i], indent + 2);
                }
            }
            break;

        case AST_PARAM_LIST:
            if (node->data.list.items) {
                for (int i = 0; i < node->data.list.items->count; i++) {
                    printAST(node->data.list.items->nodes[i], indent);
                }
            }
            break;

        case AST_RETURN_STMT:
            printf("RETURN\n");
            if (node->data.returnStmt.value) {
                printAST(node->data.returnStmt.value, indent + 1);
            }
            break;

        case AST_TYPE_HINT:
            printf("TYPE_HINT %s\n", getTypeHintName(node->data.typeHint.hintType));
            break;

        case AST_FOR_STMT:
            printf("FOR LOOP (Iterator: %s)\n", node->data.forStmt.iterVar);
            
            printIndent(indent + 1);
            printf("ITERABLE\n");
            printAST(node->data.forStmt.iterable, indent + 2);
            
            printIndent(indent + 1);
            printf("BODY\n");
            if (node->data.forStmt.body) {
                for (int i = 0; i < node->data.forStmt.body->count; i++) {
                    printAST(node->data.forStmt.body->nodes[i], indent + 2);
                }
            }
            break;

        case AST_LIST_LITERAL:
            printf("LIST LITERAL\n");
            if (node->data.listLiteral.elements) {
                for (int i = 0; i < node->data.listLiteral.elements->count; i++) {
                    printAST(node->data.listLiteral.elements->nodes[i], indent + 1);
                }
            }
            break;

        case AST_OUTPUT_STMT:
            printf("OUTPUT\n");
            if (node->data.outputStmt.expressions) {
                for (int i = 0; i < node->data.outputStmt.expressions->count; i++) {
                    printAST(node->data.outputStmt.expressions->nodes[i], indent + 1);
                }
            }
            break;

        case AST_IF_STMT:
            printf("WHEN STMT\n");
            printIndent(indent + 1);
            printf("CONDITION\n");
            printAST(node->data.ifStmt.condition, indent + 2);
            
            printIndent(indent + 1);
            printf("THEN\n");
            if (node->data.ifStmt.thenBranch) {
                for (int i = 0; i < node->data.ifStmt.thenBranch->count; i++) {
                    printAST(node->data.ifStmt.thenBranch->nodes[i], indent + 2);
                }
            }
            if (node->data.ifStmt.elseBranch) {
                printIndent(indent + 1);
                printf("ELSE\n");
                for (int i = 0; i < node->data.ifStmt.elseBranch->count; i++) {
                    printAST(node->data.ifStmt.elseBranch->nodes[i], indent + 2);
                }
            }
            break;

        case AST_COMPOUND_ASSIGN:
            printf("COMPOUND_ASSIGN %s %s\n", 
                   node->data.compoundAssign.varName,
                   getOpSymbol(node->data.compoundAssign.op));
            printAST(node->data.compoundAssign.value, indent + 1);
            break;

        case AST_INPUT_STMT:
            printf("INPUT %s\n", node->data.inputStmt.varName);
            if (node->data.inputStmt.prompt) {
                printIndent(indent + 1);
                printf("PROMPT: \"%s\"\n", node->data.inputStmt.prompt);
            }
            break;

        case AST_WHILE_STMT:
            printf("WHILE LOOP\n");
            printIndent(indent + 1);
            printf("CONDITION\n");
            printAST(node->data.whileStmt.condition, indent + 2);
            printIndent(indent + 1);
            printf("BODY\n");
            if (node->data.whileStmt.body) {
                for (int i = 0; i < node->data.whileStmt.body->count; i++) {
                    printAST(node->data.whileStmt.body->nodes[i], indent + 2);
                }
            }
            break;

        case AST_EXPR_STMT:
            printAST(node->data.exprStmt.expression, indent);
            break;

        case AST_UNARY_OP:
            printf("UNARY_OP %s\n", getOpSymbol(node->data.unaryOp.op));
            printAST(node->data.unaryOp.operand, indent + 1);
            break;

        case AST_CALL_EXPR:
            printf("CALL %s\n", node->data.callExpr.funcName);
            printAST(node->data.callExpr.args, indent + 1);
            break;

        case AST_INDEX_EXPR:
            printf("INDEX_EXPR %s\n", node->data.indexExpr.varName);
            printIndent(indent + 1);
            printf("INDEX\n");
            printAST(node->data.indexExpr.index, indent + 2);
            break;

        case AST_ARG_LIST:
            if (node->data.list.items) {
                for (int i = 0; i < node->data.list.items->count; i++) {
                    printAST(node->data.list.items->nodes[i], indent);
                }
            }
            break;

        case AST_CAST_EXPR:
            printf("CAST_EXPR to %s\n", getTypeHintName(node->data.castExpr.targetType));
            printAST(node->data.castExpr.expression, indent + 1);
            break;

        case AST_INPUT_EXPR:
            printf("INPUT_EXPR");
            if (node->data.inputExpr.prompt) {
                printf(" (prompt: \"%s\")", node->data.inputExpr.prompt);
            }
            printf("\n");
            break;

        default:
            printf("NODE_TYPE_%d\n", node->type);
            break;
    }
}

void printASTToFile(ASTNode* node, int indent, FILE* file) {
    if (!file) return;
    
    if (!node) {
        printIndentToFile(indent, file);
        fprintf(file, "(null)\n");
        return;
    }
    
    printIndentToFile(indent, file);
    
    switch (node->type) {
        case AST_PROGRAM:
            fprintf(file, "PROGRAM\n");
            if (node->data.program.statements) {
                for (int i = 0; i < node->data.program.statements->count; i++) {
                    printASTToFile(node->data.program.statements->nodes[i], indent + 1, file);
                }
            }
            break;

        case AST_CONTINUE_STMT:
            fprintf(file, "CONTINUE\n");
            break;

        case AST_IMPORT_STMT:
            if (node->data.importStmt.fromModule) {
                fprintf(file, "IMPORT %s FROM %s\n", 
                       node->data.importStmt.moduleName, 
                       node->data.importStmt.fromModule);
            } else {
                fprintf(file, "IMPORT %s\n", node->data.importStmt.moduleName);
            }
            break;
        
        case AST_BREAK_STMT:
            fprintf(file, "BREAK\n");
            break;

        case AST_VAR_DECL:
            fprintf(file, "VAR_DECL (%s) %s\n", 
                   node->data.varDecl.isMutable ? "flex" : "fixed",
                   node->data.varDecl.name);
            if (node->data.varDecl.typeHint) {
                printASTToFile(node->data.varDecl.typeHint, indent + 1, file);
            }
            if (node->data.varDecl.initializer) {
                printASTToFile(node->data.varDecl.initializer, indent + 1, file);
            }
            break;
            
        case AST_ASSIGNMENT:
            fprintf(file, "ASSIGNMENT %s =\n", node->data.assignment.varName);
            printASTToFile(node->data.assignment.value, indent + 1, file);
            break;
            
        case AST_BINARY_OP:
            fprintf(file, "BINARY_OP\n");
            printASTToFile(node->data.binaryOp.left, indent + 1, file);
            printIndentToFile(indent + 1, file);
            fprintf(file, "OP: %s\n", getOpSymbol(node->data.binaryOp.op));
            printASTToFile(node->data.binaryOp.right, indent + 1, file);
            break;
            
        case AST_LITERAL:
            fprintf(file, "LITERAL ");
            if (node->data.literal.literalType == TOKEN_INTEGER) {
                fprintf(file, "(int) %lld\n", node->data.literal.value.intValue);
            } else if (node->data.literal.literalType == TOKEN_FLOAT) {
                fprintf(file, "(float) %f\n", node->data.literal.value.floatValue);
            } else if (node->data.literal.literalType == TOKEN_STRING) {
                fprintf(file, "(string) \"%s\"\n", node->data.literal.value.stringValue);
            } else if (node->data.literal.literalType == TOKEN_CHAR) {
                fprintf(file, "(char) '%c'\n", node->data.literal.value.charValue);
            } else if (node->data.literal.literalType == TOKEN_TRUE || 
                       node->data.literal.literalType == TOKEN_FALSE) {
                fprintf(file, "(bool) %s\n", node->data.literal.value.boolValue ? "true" : "false");
            }
            break;
            
        case AST_IDENTIFIER:
            fprintf(file, "IDENTIFIER %s\n", node->data.identifier.name);
            break;

        case AST_FUNCTION_DECL:
            fprintf(file, "FUNCTION_DECL %s\n", node->data.funcDecl.name);
            
            if (node->data.funcDecl.params) {
                printIndentToFile(indent + 1, file);
                fprintf(file, "PARAMS\n");
                printASTToFile(node->data.funcDecl.params, indent + 2, file);
            }
            
            if (node->data.funcDecl.returnType) {
                printIndentToFile(indent + 1, file);
                fprintf(file, "RETURN TYPE\n");
                printASTToFile(node->data.funcDecl.returnType, indent + 2, file);
            }
            
            if (node->data.funcDecl.body) {
                printIndentToFile(indent + 1, file);
                fprintf(file, "BODY\n");
                for (int i = 0; i < node->data.funcDecl.body->count; i++) {
                    printASTToFile(node->data.funcDecl.body->nodes[i], indent + 2, file);
                }
            }
            break;

        case AST_PARAM_LIST:
            if (node->data.list.items) {
                for (int i = 0; i < node->data.list.items->count; i++) {
                    printASTToFile(node->data.list.items->nodes[i], indent, file);
                }
            }
            break;

        case AST_RETURN_STMT:
            fprintf(file, "RETURN\n");
            if (node->data.returnStmt.value) {
                printASTToFile(node->data.returnStmt.value, indent + 1, file);
            }
            break;

        case AST_TYPE_HINT:
            fprintf(file, "TYPE_HINT %s\n", getTypeHintName(node->data.typeHint.hintType));
            break;

        case AST_FOR_STMT:
            fprintf(file, "FOR LOOP (Iterator: %s)\n", node->data.forStmt.iterVar);
            
            printIndentToFile(indent + 1, file);
            fprintf(file, "ITERABLE\n");
            printASTToFile(node->data.forStmt.iterable, indent + 2, file);
            
            printIndentToFile(indent + 1, file);
            fprintf(file, "BODY\n");
            if (node->data.forStmt.body) {
                for (int i = 0; i < node->data.forStmt.body->count; i++) {
                    printASTToFile(node->data.forStmt.body->nodes[i], indent + 2, file);
                }
            }
            break;

        case AST_LIST_LITERAL:
            fprintf(file, "LIST LITERAL\n");
            if (node->data.listLiteral.elements) {
                for (int i = 0; i < node->data.listLiteral.elements->count; i++) {
                    printASTToFile(node->data.listLiteral.elements->nodes[i], indent + 1, file);
                }
            }
            break;

        case AST_OUTPUT_STMT:
            fprintf(file, "OUTPUT\n");
            if (node->data.outputStmt.expressions) {
                for (int i = 0; i < node->data.outputStmt.expressions->count; i++) {
                    printASTToFile(node->data.outputStmt.expressions->nodes[i], indent + 1, file);
                }
            }
            break;

        case AST_IF_STMT:
            fprintf(file, "WHEN STMT\n");
            printIndentToFile(indent + 1, file);
            fprintf(file, "CONDITION\n");
            printASTToFile(node->data.ifStmt.condition, indent + 2, file);
            
            printIndentToFile(indent + 1, file);
            fprintf(file, "THEN\n");
            if (node->data.ifStmt.thenBranch) {
                for (int i = 0; i < node->data.ifStmt.thenBranch->count; i++) {
                    printASTToFile(node->data.ifStmt.thenBranch->nodes[i], indent + 2, file);
                }
            }
            if (node->data.ifStmt.elseBranch) {
                printIndentToFile(indent + 1, file);
                fprintf(file, "ELSE\n");
                for (int i = 0; i < node->data.ifStmt.elseBranch->count; i++) {
                    printASTToFile(node->data.ifStmt.elseBranch->nodes[i], indent + 2, file);
                }
            }
            break;

        case AST_COMPOUND_ASSIGN:
            fprintf(file, "COMPOUND_ASSIGN %s %s\n", 
                   node->data.compoundAssign.varName,
                   getOpSymbol(node->data.compoundAssign.op));
            printASTToFile(node->data.compoundAssign.value, indent + 1, file);
            break;

        case AST_INPUT_STMT:
            fprintf(file, "INPUT %s\n", node->data.inputStmt.varName);
            if (node->data.inputStmt.prompt) {
                printIndentToFile(indent + 1, file);
                fprintf(file, "PROMPT: \"%s\"\n", node->data.inputStmt.prompt);
            }
            break;

        case AST_WHILE_STMT:
            fprintf(file, "WHILE LOOP\n");
            printIndentToFile(indent + 1, file);
            fprintf(file, "CONDITION\n");
            printASTToFile(node->data.whileStmt.condition, indent + 2, file);
            printIndentToFile(indent + 1, file);
            fprintf(file, "BODY\n");
            if (node->data.whileStmt.body) {
                for (int i = 0; i < node->data.whileStmt.body->count; i++) {
                    printASTToFile(node->data.whileStmt.body->nodes[i], indent + 2, file);
                }
            }
            break;

        case AST_EXPR_STMT:
            printASTToFile(node->data.exprStmt.expression, indent, file);
            break;

        case AST_UNARY_OP:
            fprintf(file, "UNARY_OP %s\n", getOpSymbol(node->data.unaryOp.op));
            printASTToFile(node->data.unaryOp.operand, indent + 1, file);
            break;

        case AST_CALL_EXPR:
            fprintf(file, "CALL %s\n", node->data.callExpr.funcName);
            printASTToFile(node->data.callExpr.args, indent + 1, file);
            break;

        case AST_INDEX_EXPR:
            fprintf(file, "INDEX_EXPR %s\n", node->data.indexExpr.varName);
            printIndentToFile(indent + 1, file);
            fprintf(file, "INDEX\n");
            printASTToFile(node->data.indexExpr.index, indent + 2, file);
            break;

        case AST_ARG_LIST:
            if (node->data.list.items) {
                for (int i = 0; i < node->data.list.items->count; i++) {
                    printASTToFile(node->data.list.items->nodes[i], indent, file);
                }
            }
            break;

        case AST_CAST_EXPR:
            fprintf(file, "CAST_EXPR to %s\n", getTypeHintName(node->data.castExpr.targetType));
            printASTToFile(node->data.castExpr.expression, indent + 1, file);
            break;

        case AST_INPUT_EXPR:
            fprintf(file, "INPUT_EXPR");
            if (node->data.inputExpr.prompt) {
                fprintf(file, " (prompt: \"%s\")", node->data.inputExpr.prompt);
            }
            fprintf(file, "\n");
            break;

        default:
            fprintf(file, "NODE_TYPE_%d\n", node->type);
            break;
    }
}