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
            
        case AST_PARAM_LIST:
        case AST_ARG_LIST:
            freeNodeList(node->data.list.items);
            break;
            
        default:
            break;
    }
    
    free(node);
}

// ===== AST Visualization (for debugging) =====

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
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
            printf("OP: %d\n", node->data.binaryOp.op);
            printAST(node->data.binaryOp.right, indent + 1);
            break;
            
        case AST_LITERAL:
            printf("LITERAL ");
            if (node->data.literal.literalType == TOKEN_INTEGER) {
                printf("%lld\n", node->data.literal.value.intValue);
            } else if (node->data.literal.literalType == TOKEN_FLOAT) {
                printf("%f\n", node->data.literal.value.floatValue);
            } else if (node->data.literal.literalType == TOKEN_STRING) {
                printf("\"%s\"\n", node->data.literal.value.stringValue);
            }
            break;
            
        case AST_IDENTIFIER:
            printf("IDENTIFIER %s\n", node->data.identifier.name);
            break;
            
        default:
            printf("NODE_TYPE_%d\n", node->type);
            break;
    }
}