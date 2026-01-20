#ifndef EAC_AST_H
#define EAC_AST_H

#include "../common/token.h"
#include <stdio.h>

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct ASTNodeList ASTNodeList;

// ===== AST Node Types =====
typedef enum {
    // Program & Statements
    AST_PROGRAM,
    AST_STATEMENT_LIST,
    
    // Declarations
    AST_VAR_DECL,           
    AST_FUNCTION_DECL,      
    AST_IMPORT_STMT,        
    
    // Statements
    AST_ASSIGNMENT,         
    AST_COMPOUND_ASSIGN,    
    AST_OUTPUT_STMT,        
    AST_INPUT_STMT,         
    AST_IF_STMT,            
    AST_WHILE_STMT,         
    AST_FOR_STMT,           
    AST_RETURN_STMT,        
    AST_BREAK_STMT,         
    AST_CONTINUE_STMT,      
    AST_EXPR_STMT,          
    
    // Expressions
    AST_BINARY_OP,          
    AST_UNARY_OP,           
    AST_CALL_EXPR,          
    AST_INDEX_EXPR,         
    AST_LITERAL,            
    AST_IDENTIFIER,         
    AST_LIST_LITERAL,       
    AST_CAST_EXPR,          
    AST_INPUT_EXPR,         
    
    // Type hints
    AST_TYPE_HINT,          
    
    // Parameters & Arguments
    AST_PARAM_LIST,         
    AST_ARG_LIST,           
} ASTNodeType;

// ===== Value Union for Literals =====
typedef union {
    long long intValue;
    double floatValue;
    char* stringValue;
    char charValue;
    bool boolValue;
} LiteralValue;

// ===== AST Node List =====
struct ASTNodeList {
    ASTNode** nodes;
    int count;
    int capacity;
};

// ===== Main AST Node Structure =====
struct ASTNode {
    ASTNodeType type;
    int line;
    
    union {
        // Program
        struct {
            ASTNodeList* statements;
        } program;
        
        // Variable Declaration
        struct {
            bool isMutable;        
            char* name;
            ASTNode* typeHint;      
            ASTNode* initializer;   
        } varDecl;
        
        // Function Declaration
        struct {
            char* name;
            ASTNode* params;        
            ASTNode* returnType;    
            ASTNodeList* body;
        } funcDecl;
        
        // Import Statement
        struct {
            char* moduleName;
            char* fromModule;       
        } importStmt;
        
        // Assignment
        struct {
            char* varName;
            ASTNode* value;
        } assignment;
        
        // Compound Assignment
        struct {
            char* varName;
            TokenType op;          
            ASTNode* value;
        } compoundAssign;
        
        // Output Statement
        struct {
            ASTNodeList* expressions;
        } outputStmt;
        
        // Input Statement
        struct {
            char* varName;
            char* prompt;           
        } inputStmt;
        
        // If Statement
        struct {
            ASTNode* condition;
            ASTNodeList* thenBranch;
            ASTNodeList* elseBranch;    
        } ifStmt;
        
        // While Loop
        struct {
            ASTNode* condition;
            ASTNodeList* body;
        } whileStmt;
        
        // For Loop
        struct {
            char* iterVar;
            ASTNode* iterable;
            ASTNodeList* body;
        } forStmt;
        
        // Return Statement
        struct {
            ASTNode* value;         
        } returnStmt;
        
        // Expression Statement
        struct {
            ASTNode* expression;
        } exprStmt;
        
        // Binary Operation
        struct {
            TokenType op;
            ASTNode* left;
            ASTNode* right;
        } binaryOp;
        
        // Unary Operation
        struct {
            TokenType op;
            ASTNode* operand;
        } unaryOp;
        
        // Function Call
        struct {
            char* funcName;
            ASTNode* args;          
        } callExpr;
        
        // Index Expression
        struct {
            char* varName;
            ASTNode* index;
        } indexExpr;
        
        // Literal
        struct {
            TokenType literalType;
            LiteralValue value;
        } literal;
        
        // Identifier
        struct {
            char* name;
        } identifier;
        
        // List Literal
        struct {
            ASTNodeList* elements;
        } listLiteral;
        
        // Cast Expression
        struct {
            ASTNode* expression;
            TokenType targetType;
        } castExpr;
        
        // Input Expression
        struct {
            char* prompt;
        } inputExpr;
        
        // Type Hint
        struct {
            TokenType hintType;     
        } typeHint;
        
        // Parameter/Argument List
        struct {
            ASTNodeList* items;
        } list;
    } data;
};

// ===== AST Construction Functions =====

// Node list management
ASTNodeList* createNodeList(void);
void addNode(ASTNodeList* list, ASTNode* node);
void freeNodeList(ASTNodeList* list);

// Program
ASTNode* createProgram(ASTNodeList* statements);

// Declarations
ASTNode* createVarDecl(bool isMutable, char* name, ASTNode* typeHint, ASTNode* initializer, int line);
ASTNode* createFuncDecl(char* name, ASTNode* params, ASTNode* returnType, ASTNodeList* body, int line);
ASTNode* createImportStmt(char* moduleName, char* fromModule, int line);

// Statements
ASTNode* createAssignment(char* varName, ASTNode* value, int line);
ASTNode* createCompoundAssign(char* varName, TokenType op, ASTNode* value, int line);
ASTNode* createOutputStmt(ASTNodeList* expressions, int line);
ASTNode* createInputStmt(char* varName, char* prompt, int line);
ASTNode* createIfStmt(ASTNode* condition, ASTNodeList* thenBranch, ASTNodeList* elseBranch, int line);
ASTNode* createWhileStmt(ASTNode* condition, ASTNodeList* body, int line);
ASTNode* createForStmt(char* iterVar, ASTNode* iterable, ASTNodeList* body, int line);
ASTNode* createReturnStmt(ASTNode* value, int line);
ASTNode* createBreakStmt(int line);
ASTNode* createContinueStmt(int line);
ASTNode* createExprStmt(ASTNode* expression, int line);

// Expressions
ASTNode* createBinaryOp(TokenType op, ASTNode* left, ASTNode* right, int line);
ASTNode* createUnaryOp(TokenType op, ASTNode* operand, int line);
ASTNode* createCallExpr(char* funcName, ASTNode* args, int line);
ASTNode* createIndexExpr(char* varName, ASTNode* index, int line);
ASTNode* createIntLiteral(long long value, int line);
ASTNode* createFloatLiteral(double value, int line);
ASTNode* createStringLiteral(char* value, int line);
ASTNode* createCharLiteral(char value, int line);
ASTNode* createBoolLiteral(bool value, int line);
ASTNode* createIdentifier(char* name, int line);
ASTNode* createListLiteral(ASTNodeList* elements, int line);
ASTNode* createCastExpr(ASTNode* expr, TokenType targetType, int line);
ASTNode* createInputExpr(char* prompt, int line); 

// Type hints
ASTNode* createTypeHint(TokenType hintType, int line);

// Lists
ASTNode* createParamList(ASTNodeList* params, int line);
ASTNode* createArgList(ASTNodeList* args, int line);

// Memory management
void freeAST(ASTNode* node);

// AST Visualization
void printAST(ASTNode* node, int indent);
void printASTToFile(ASTNode* node, int indent, FILE* file);

#endif