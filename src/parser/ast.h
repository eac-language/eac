#ifndef EAC_AST_H
#define EAC_AST_H

#include "../common/token.h"

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct ASTNodeList ASTNodeList;

// ===== AST Node Types =====
typedef enum {
    // Program & Statements
    AST_PROGRAM,
    AST_STATEMENT_LIST,
    
    // Declarations
    AST_VAR_DECL,           // flex/fixed variable declaration
    AST_FUNCTION_DECL,      // function declaration
    AST_IMPORT_STMT,        // import statement
    
    // Statements
    AST_ASSIGNMENT,         // variable assignment
    AST_COMPOUND_ASSIGN,    // +=, -=, etc.
    AST_OUTPUT_STMT,        // output statement
    AST_INPUT_STMT,         // input statement
    AST_IF_STMT,            // when/else statement
    AST_WHILE_STMT,         // while loop
    AST_FOR_STMT,           // for loop
    AST_RETURN_STMT,        // return statement
    AST_BREAK_STMT,         // break statement
    AST_CONTINUE_STMT,      // continue statement
    AST_EXPR_STMT,          // expression statement
    
    // Expressions
    AST_BINARY_OP,          // binary operations
    AST_UNARY_OP,           // unary operations
    AST_CALL_EXPR,          // function call
    AST_INDEX_EXPR,         // array indexing
    AST_LITERAL,            // literals (int, float, string, char, bool)
    AST_IDENTIFIER,         // variable reference
    AST_LIST_LITERAL,       // list literal [1, 2, 3]
    
    // Type hints
    AST_TYPE_HINT,          // type annotation
    
    // Parameters & Arguments
    AST_PARAM_LIST,         // function parameters
    AST_ARG_LIST,           // function arguments
} ASTNodeType;

// ===== Value Union for Literals =====
typedef union {
    long long intValue;
    double floatValue;
    char* stringValue;
    char charValue;
    bool boolValue;
} LiteralValue;

// ===== AST Node List (for managing multiple nodes) =====
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
            bool isMutable;         // flex=true, fixed=false
            char* name;
            ASTNode* typeHint;      // optional
            ASTNode* initializer;   // optional
        } varDecl;
        
        // Function Declaration
        struct {
            char* name;
            ASTNode* params;        // AST_PARAM_LIST
            ASTNode* returnType;    // optional type hint
            ASTNodeList* body;
        } funcDecl;
        
        // Import Statement
        struct {
            char* moduleName;
            char* fromModule;       // optional (for "from X import Y")
        } importStmt;
        
        // Assignment
        struct {
            char* varName;
            ASTNode* value;
        } assignment;
        
        // Compound Assignment
        struct {
            char* varName;
            TokenType op;           // +=, -=, *=, /=, %=
            ASTNode* value;
        } compoundAssign;
        
        // Output Statement
        struct {
            ASTNodeList* expressions;
        } outputStmt;
        
        // Input Statement
        struct {
            char* varName;
            char* prompt;           // optional prompt string
        } inputStmt;
        
        // If Statement
        struct {
            ASTNode* condition;
            ASTNodeList* thenBranch;
            ASTNodeList* elseBranch;    // optional
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
            ASTNode* value;         // optional
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
            ASTNode* args;          // AST_ARG_LIST
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
        
        // Type Hint
        struct {
            TokenType hintType;     // TOKEN_HINT_INT, etc.
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

// Type hints
ASTNode* createTypeHint(TokenType hintType, int line);

// Lists
ASTNode* createParamList(ASTNodeList* params, int line);
ASTNode* createArgList(ASTNodeList* args, int line);

// Memory management
void freeAST(ASTNode* node);

// AST Visualization (for debugging)
void printAST(ASTNode* node, int indent);

#endif // EAC_AST_H