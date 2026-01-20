# EaC Parser Walkthrough

This document provides a detailed, step-by-step walkthrough of how the EaC parser processes tokens, builds the Abstract Syntax Tree (AST), and handles errors.

---

## Table of Contents

1. [Overview](#overview)
2. [Parser Architecture](#parser-architecture)
3. [Step-by-Step Walkthrough](#step-by-step-walkthrough)
4. [Grammar Rules](#grammar-rules)
5. [Error Handling](#error-handling)
6. [Complete Examples](#complete-examples)

---

## Overview

The EaC parser is a **recursive descent parser** that transforms a stream of tokens (from the lexer) into an **Abstract Syntax Tree (AST)**. It follows the EaC grammar to validate syntax and build a structured representation of the program.

### Key Files

| File | Purpose |
|------|---------|
| `src/parser/parser.c` | Main parser implementation |
| `src/parser/parser.h` | Parser interface definitions |
| `src/parser/ast.c` | AST node creation functions |
| `src/parser/ast.h` | AST node type definitions |

---

## Parser Architecture

### Data Structures

#### Parser Structure (`src/parser/parser.c`, lines 13-19)

```c
struct Parser {
    Lexer* lexer;      // Pointer to the lexer
    Token current;     // Current token being examined
    Token previous;    // Previously consumed token
    bool hadError;     // Has any error occurred?
    bool panicMode;    // In panic mode (error recovery)?
};
```

### Core Helper Functions

#### `advance()` - Move to Next Token (lines 88-103)

```c
static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = getNextToken(parser->lexer);
        
        // Skip comments automatically
        if (parser->current.type == TOKEN_COMMENT_LINE ||
            parser->current.type == TOKEN_COMMENT_BLOCK) {
            continue;
        }
        
        if (parser->current.type != TOKEN_ERROR) break;
        
        errorAtCurrent(parser, parser->current.lexeme);
    }
}
```

#### `check()` - Check Current Token Type (lines 105-107)

```c
static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}
```

#### `match()` - Check and Consume Token (lines 109-113)

```c
static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}
```

#### `consume()` - Require Specific Token (lines 115-121)

```c
static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    errorAtCurrent(parser, message);
}
```

---

## Step-by-Step Walkthrough

Let's trace through how the parser processes input.

### Example Input

```eac
flex x = 42
```

### Step 1: Parser Initialization (`initParser`, lines 964-977)

**Code:**
```c
Parser* initParser(Lexer* lexer) {
    if (!lexer) return NULL;
    
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
    
    advance(parser);  // Load first token
    
    return parser;
}
```

**State After Initialization:**
```
current: Token { type: TOKEN_FLEX, lexeme: "flex", line: 1 }
previous: (uninitialized/empty)
hadError: false
panicMode: false
```

### Step 2: Begin Parsing (`parse`, lines 980-993)

**Code:**
```c
ASTNode* parse(Parser* parser) {
    if (!parser) return NULL;
    
    ASTNodeList* stmts = statements(parser);  // Parse all statements
    
    while (match(parser, TOKEN_DEDENT)) {
        // Clean up any remaining DEDENTs
    }
    
    if (!match(parser, TOKEN_EOF)) {
        errorAtCurrent(parser, "Expected end of file");
    }
    
    return createProgram(stmts);  // Wrap in PROGRAM node
}
```

### Step 3: Parse Statements List (`statements`, lines 932-962)

**Code:**
```c
static ASTNodeList* statements(Parser* parser) {
    ASTNodeList* stmtList = createNodeList();
    
    while (!check(parser, TOKEN_EOF) && 
           !check(parser, TOKEN_ELSE) && 
           !check(parser, TOKEN_DEDENT)) {
        
        // Skip any newlines
        while (match(parser, TOKEN_NEWLINE) || check(parser, TOKEN_INDENT)) {
            if (check(parser, TOKEN_INDENT)) {
                advance(parser);
            }
        }
        
        // Check termination conditions
        if (check(parser, TOKEN_EOF) || 
            check(parser, TOKEN_ELSE) || 
            check(parser, TOKEN_DEDENT)) {
            break;
        }
        
        ASTNode* stmt = statement(parser);  // Parse single statement
        if (stmt && !parser->panicMode) {
            addNode(stmtList, stmt);
        }
        
        if (parser->panicMode) {
            synchronize(parser);  // Error recovery
        }
    }
    
    return stmtList;
}
```

### Step 4: Parse Single Statement (`statement`, lines 810-929)

**Code Entry Point:**
```c
static ASTNode* statement(Parser* parser) {
    skipNewlines(parser);
    
    // ... check various statement types ...
    
    if (match(parser, TOKEN_FLEX) || match(parser, TOKEN_FIXED)) {
        return declarationStatement(parser);  // <-- Our case!
    }
    
    // ... more statement types ...
}
```

**Current token is `TOKEN_FLEX`, so `match(parser, TOKEN_FLEX)` returns `true`:**

1. `check(parser, TOKEN_FLEX)` returns `true`
2. `advance(parser)` is called, moving to next token
3. State becomes:
   ```
   current: Token { type: TOKEN_IDENTIFIER, lexeme: "x", line: 1 }
   previous: Token { type: TOKEN_FLEX, lexeme: "flex", line: 1 }
   ```
4. `declarationStatement(parser)` is called

### Step 5: Parse Declaration Statement (`declarationStatement`, lines 681-728)

**Code:**
```c
static ASTNode* declarationStatement(Parser* parser) {
    bool isMutable = parser->previous.type == TOKEN_FLEX;  // true
    int line = parser->previous.line;  // 1
    
    // Expect identifier
    if (!match(parser, TOKEN_IDENTIFIER)) {
        errorAtCurrent(parser, "Expected variable name after variable type");
        return NULL;
    }
    
    char* name = tokenToString(parser->previous);  // "x"
    ASTNode* typeHint = NULL;
    ASTNode* initializer = NULL;
    
    // Check for type hint (optional)
    if (match(parser, TOKEN_OF) || match(parser, TOKEN_AS)) {
        // ... parse type hint ...
    }
    else if (match(parser, TOKEN_COLON)) {
        // ... parse type hint with : ...
    }
    
    // Check for initializer
    if (match(parser, TOKEN_TO)) {
        initializer = expression(parser);
    }
    else if (match(parser, TOKEN_EQUAL)) {  // <-- This matches!
        initializer = expression(parser);
    }
    
    ASTNode* node = createVarDecl(isMutable, name, typeHint, initializer, line);
    free(name);
    checkStatementEnd(parser);
    return node;
}
```

**Execution Trace:**

| Step | Action | Current Token | Previous Token |
|------|--------|---------------|----------------|
| 1 | `isMutable = true` | TOKEN_IDENTIFIER "x" | TOKEN_FLEX "flex" |
| 2 | `match(TOKEN_IDENTIFIER)` succeeds | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 3 | `name = "x"` | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 4 | `match(TOKEN_OF)` fails | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 5 | `match(TOKEN_AS)` fails | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 6 | `match(TOKEN_COLON)` fails | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 7 | `match(TOKEN_TO)` fails | TOKEN_EQUAL "=" | TOKEN_IDENTIFIER "x" |
| 8 | `match(TOKEN_EQUAL)` succeeds | TOKEN_INTEGER "42" | TOKEN_EQUAL "=" |
| 9 | `expression(parser)` called | TOKEN_INTEGER "42" | TOKEN_EQUAL "=" |

### Step 6: Parse Expression (`expression`, lines 476-478)

**Code:**
```c
static ASTNode* expression(Parser* parser) {
    return logicalExpr(parser);  // Start at lowest precedence
}
```

The parser uses **precedence climbing** through mutual recursion:

```
expression()
  └── logicalExpr()     [or]
        └── boolTerm()  [and]
              └── boolFactor()  [not]
                    └── relationalExpr()  [<, >, ==, !=, <=, >=]
                          └── membershipExpr()  [in]
                                └── castExpr()  [as]
                                      └── arithmeticExpr()  [+, -]
                                            └── term()  [*, /, %, //]
                                                  └── factor()  [-unary]
                                                        └── power()  [^]
                                                              └── primary()  [literals, identifiers]
```

For our input `42`, this cascades down to `primary()`:

### Step 7: Parse Primary (`primary`, lines 208-317)

**Code:**
```c
static ASTNode* primary(Parser* parser) {
    if (match(parser, TOKEN_TRUE)) {
        return createBoolLiteral(true, parser->previous.line);
    }
    
    if (match(parser, TOKEN_FALSE)) {
        return createBoolLiteral(false, parser->previous.line);
    }
    
    if (match(parser, TOKEN_INTEGER)) {  // <-- This matches!
        long long value = parseInteger(parser->previous);  // 42
        return createIntLiteral(value, parser->previous.line);
    }
    
    // ... more cases ...
}
```

**Execution:**

1. `match(parser, TOKEN_INTEGER)` succeeds
2. `parser->previous` is now `{ type: TOKEN_INTEGER, lexeme: "42" }`
3. `parseInteger(parser->previous)` returns `42`
4. `createIntLiteral(42, 1)` creates AST node

**Created AST Node:**
```
IntLiteral {
    type: AST_INT_LITERAL
    value: 42
    line: 1
}
```

### Step 8: Return Up the Call Stack

The `IntLiteral` node bubbles back up through:
- `power()` → returns `IntLiteral`
- `factor()` → returns `IntLiteral`
- `term()` → returns `IntLiteral`
- `arithmeticExpr()` → returns `IntLiteral`
- `castExpr()` → returns `IntLiteral`
- `membershipExpr()` → returns `IntLiteral`
- `relationalExpr()` → returns `IntLiteral`
- `boolFactor()` → returns `IntLiteral`
- `boolTerm()` → returns `IntLiteral`
- `logicalExpr()` → returns `IntLiteral`
- `expression()` → returns `IntLiteral`

### Step 9: Complete Declaration Node

Back in `declarationStatement()`:

```c
ASTNode* node = createVarDecl(
    isMutable,      // true (flex)
    name,           // "x"
    typeHint,       // NULL
    initializer,    // IntLiteral(42)
    line            // 1
);
```

**Created AST Node:**
```
VarDecl {
    type: AST_VAR_DECL
    isMutable: true
    name: "x"
    typeHint: NULL
    initializer: IntLiteral { value: 42 }
    line: 1
}
```

### Step 10: Check Statement End (`checkStatementEnd`, lines 26-31)

```c
static void checkStatementEnd(Parser* parser) {
    if (check(parser, TOKEN_NEWLINE) || check(parser, TOKEN_EOF) || check(parser, TOKEN_DEDENT)) {
        return;  // Valid statement ending
    }
    errorAtCurrent(parser, "Expected newline or end of statement");
}
```

Current token is `TOKEN_EOF`, so this passes.

### Step 11: Create Program Node

Back in `parse()`:

```c
return createProgram(stmts);
```

**Final AST:**
```
Program {
    type: AST_PROGRAM
    statements: [
        VarDecl {
            type: AST_VAR_DECL
            isMutable: true
            name: "x"
            typeHint: NULL
            initializer: IntLiteral { value: 42 }
            line: 1
        }
    ]
}
```

---

## Grammar Rules

The parser implements the following grammar rules (excerpted from key productions):

### Program Structure

```
<PROGRAM> → <STATEMENTS>
<STATEMENTS> → <STATEMENT> | <STATEMENT> <NEWLINE> <STATEMENTS>
```

### Statements

```
<STATEMENT> → <FUNCTION_STMT> | <DECL_STMT> | <ASS_STMT> | <INPUT_STMT> 
            | <OUTPUT_STMT> | <COND_STMT> | <BREAK_STMT> | <RETURN_STMT> 
            | <ITER_STMT> | <IMPORT_STMT>

<DECL_STMT> → <VAR_TYPE> <id> [<TYPE_HINT>] [<ASSIGN> <expr>]
<VAR_TYPE> → flex | fixed

<ASS_STMT> → <id> <ASSIGN_OP> <expr>
<ASSIGN_OP> → = | += | -= | *= | /= | %=

<OUTPUT_STMT> → output ( <OUTPUT_ARGS> )
<INPUT_STMT> → <id> = input () | <id> = input ( <STRING_LITERAL> )
```

### Control Flow

```
<WHEN_STMT> → when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
<WHEN_ELSE_STMT> → when <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
                   else : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>

<WHILE_STMT> → while <CONDITION> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>

<FOR_STMT> → for <id> in <ITERABLE> : <NEWLINE> <INDENT> <STATEMENTS> <DEDENT>
<ITERABLE> → <id> | range ( <expr> ) | [ <LIST_ITEMS> ]
```

### Expressions

```
<expr> → <LOGICAL_EXPR>
<LOGICAL_EXPR> → <BOOL_TERM> | <LOGICAL_EXPR> or <BOOL_TERM>
<BOOL_TERM> → <BOOL_FACTOR> | <BOOL_TERM> and <BOOL_FACTOR>
<BOOL_FACTOR> → <RELATIONAL_EXPR> | not <BOOL_FACTOR>
<RELATIONAL_EXPR> → <MEMBERSHIP_EXPR> <REL_OP> <MEMBERSHIP_EXPR>
<MEMBERSHIP_EXPR> → <CAST_EXPR> | <CAST_EXPR> in <CAST_EXPR>
<CAST_EXPR> → <ARITH_EXPR> | <ARITH_EXPR> as <TYPE>
<ARITH_EXPR> → <TERM> | <ARITH_EXPR> + <TERM> | <ARITH_EXPR> - <TERM>
<TERM> → <FACTOR> | <TERM> * <FACTOR> | <TERM> / <FACTOR>
<FACTOR> → <POWER> | - <FACTOR>
<POWER> → <PRIMARY> | <PRIMARY> ^ <POWER>
<PRIMARY> → <LITERAL> | <id> | ( <expr> ) | | <expr> |
```

---

## Error Handling

The parser uses two key mechanisms for error handling: **error reporting** and **panic mode recovery**.

### Error Reporting (`errorAt`, lines 33-82)

```c
static void errorAt(Parser* parser, Token* token, const char* message) {
    if (parser->panicMode) return;  // Don't report cascading errors
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
    
    // Show the problematic token context
    if (token->type != TOKEN_EOF && token->type != TOKEN_ERROR) {
        fprintf(stderr, "  %.*s\n", token->length, token->lexeme);
        fprintf(stderr, "  ");
        for (int i = 0; i < token->length && i < 5; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
    }
}
```

**Example Error Output:**
```
Error at line 3, column 5: Expected ':' after condition in 'when' statement
  then
  ^^^^
```

### Panic Mode Recovery (`synchronize`, lines 123-145)

When an error occurs, the parser enters **panic mode** and skips tokens until it finds a synchronization point:

```c
static void synchronize(Parser* parser) {
    parser->panicMode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        // Synchronize at end of statement
        if (parser->previous.type == TOKEN_NEWLINE) return;
        if (parser->previous.type == TOKEN_DEDENT) return;
        
        // Synchronize at start of new statement
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
```

### Error Examples

#### Example 1: Missing Colon After Condition

**Input:**
```eac
when x > 0
    output("positive")
```

**Error Flow:**

1. Parser matches `TOKEN_WHEN`
2. `conditionalStatement()` is called
3. `condition()` parses `x > 0`
4. `match(parser, TOKEN_THEN)` fails (no optional `then`)
5. `consume(parser, TOKEN_COLON, ...)` is called:

```c
consume(parser, TOKEN_COLON, "Expected ':' after condition in 'when' statement");
```

6. Current token is `TOKEN_NEWLINE`, not `TOKEN_COLON`
7. `errorAtCurrent()` is called:

**Output:**
```
Error at line 1, column 11: Expected ':' after condition in 'when' statement
```

**Code Location (lines 540):**
```c
consume(parser, TOKEN_COLON, "Expected ':' after condition in 'when' statement");
```

#### Example 2: Missing Indentation

**Input:**
```eac
when x > 0:
output("positive")
```

**Error Flow:**

1. Parser matches `TOKEN_WHEN`
2. Condition and colon are parsed correctly
3. `consume(parser, TOKEN_NEWLINE, ...)` succeeds
4. `consume(parser, TOKEN_INDENT, ...)` is called:

```c
consume(parser, TOKEN_INDENT, "Expected indentation after 'when:'");
```

5. Current token is `TOKEN_OUTPUT`, not `TOKEN_INDENT`

**Output:**
```
Error at line 2, column 1: Expected indentation after 'when:'
  output
  ^^^^^
```

**Code Location (lines 542):**
```c
consume(parser, TOKEN_INDENT, "Expected indentation after 'when:'");
```

#### Example 3: Unexpected Token in Expression

**Input:**
```eac
flex x = 10 + * 5
```

**Error Flow:**

1. `declarationStatement()` parses `flex x =`
2. `expression()` is called
3. `arithmeticExpr()` parses `10`
4. Matches `+`, continues with right operand
5. `term()` → `factor()` → `power()` → `primary()`
6. In `primary()`, current token is `*` (TOKEN_STAR)
7. No match case handles `*` at expression start

**Code (lines 315-316):**
```c
errorAtCurrent(parser, "Expected expression");
return NULL;
```

**Output:**
```
Error at line 1, column 14: Expected expression
  *
  ^
```

#### Example 4: Invalid Type Hint

**Input:**
```eac
flex x : invalid = 10
```

**Error Flow:**

1. `declarationStatement()` parses `flex x`
2. Matches `TOKEN_COLON`
3. Checks for valid type hint tokens:

```c
if (check(parser, TOKEN_HINT_INT) || check(parser, TOKEN_HINT_FLOAT) ||
    check(parser, TOKEN_HINT_STR) || check(parser, TOKEN_HINT_BOOL) ||
    check(parser, TOKEN_HINT_CHAR)) {
    // ... handle type hint ...
} else {
    errorAtCurrent(parser, "Invalid type hint - expected 'int', 'float', 'str', 'bool', or 'char'");
}
```

4. `invalid` is `TOKEN_IDENTIFIER`, not a type hint

**Output:**
```
Error at line 1, column 10: Invalid type hint - expected 'int', 'float', 'str', 'bool', or 'char'
  invalid
  ^^^^^
```

**Code Location (lines 713):**
```c
errorAtCurrent(parser, "Invalid type hint - expected 'int', 'float', 'str', 'bool', or 'char'");
```

#### Example 5: Missing Closing Parenthesis

**Input:**
```eac
output("hello"
```

**Error Flow:**

1. `statement()` matches `TOKEN_OUTPUT`
2. `outputStatement()` is called
3. `consume(TOKEN_LPAREN, ...)` succeeds
4. Expression `"hello"` is parsed
5. `consume(TOKEN_RPAREN, ...)` is called:

```c
consume(parser, TOKEN_RPAREN, "Expected ')' after output arguments");
```

6. Current token is `TOKEN_EOF` or `TOKEN_NEWLINE`

**Output:**
```
Error at line 1, column 15: Expected ')' after output arguments
```

**Code Location (lines 524):**
```c
consume(parser, TOKEN_RPAREN, "Expected ')' after output arguments");
```

#### Example 6: Missing Function Name

**Input:**
```eac
function ():
    return 0
```

**Error Flow:**

1. `statement()` matches `TOKEN_FUNCTION`
2. `functionDeclaration()` is called
3. `consume(TOKEN_IDENTIFIER, ...)` is called:

```c
consume(parser, TOKEN_IDENTIFIER, "Expected function name.");
```

4. Current token is `TOKEN_LPAREN`, not identifier

**Output:**
```
Error at line 1, column 10: Expected function name.
  (
  ^
```

**Code Location (lines 734):**
```c
consume(parser, TOKEN_IDENTIFIER, "Expected function name.");
```

---

## Complete Examples

### Example 1: Simple Program

**Input:**
```eac
flex x = 10
flex y = 20
output(x + y)
```

**Token Stream (from lexer):**
```
TOKEN_FLEX, TOKEN_IDENTIFIER("x"), TOKEN_EQUAL, TOKEN_INTEGER(10), TOKEN_NEWLINE
TOKEN_FLEX, TOKEN_IDENTIFIER("y"), TOKEN_EQUAL, TOKEN_INTEGER(20), TOKEN_NEWLINE
TOKEN_OUTPUT, TOKEN_LPAREN, TOKEN_IDENTIFIER("x"), TOKEN_PLUS, TOKEN_IDENTIFIER("y"), TOKEN_RPAREN, TOKEN_EOF
```

**Parse Trace:**

```
parse() called
  └── statements() called
        ├── statement() - matches TOKEN_FLEX
        │     └── declarationStatement()
        │           ├── match(TOKEN_IDENTIFIER) → "x"
        │           ├── match(TOKEN_EQUAL) → true
        │           └── expression() → IntLiteral(10)
        │
        ├── statement() - matches TOKEN_FLEX
        │     └── declarationStatement()
        │           ├── match(TOKEN_IDENTIFIER) → "y"
        │           ├── match(TOKEN_EQUAL) → true
        │           └── expression() → IntLiteral(20)
        │
        └── statement() - matches TOKEN_OUTPUT
              └── outputStatement()
                    ├── consume(TOKEN_LPAREN)
                    ├── expression() → BinaryOp(+, Identifier("x"), Identifier("y"))
                    └── consume(TOKEN_RPAREN)
```

**Final AST:**

```
Program
├── VarDecl { name: "x", initializer: IntLiteral(10) }
├── VarDecl { name: "y", initializer: IntLiteral(20) }
└── OutputStmt
    └── expressions: [
          BinaryOp {
            op: TOKEN_PLUS,
            left: Identifier("x"),
            right: Identifier("y")
          }
        ]
```

### Example 2: Conditional Statement

**Input:**
```eac
when x > 0:
    output("positive")
else:
    output("non-positive")
```

**Parse Trace:**

```
parse() called
  └── statements() called
        └── statement() - matches TOKEN_WHEN
              └── conditionalStatement()
                    ├── condition() → BinaryOp(>, Identifier("x"), IntLiteral(0))
                    ├── consume(TOKEN_COLON)
                    ├── consume(TOKEN_NEWLINE)
                    ├── consume(TOKEN_INDENT)
                    ├── statements() - parse "then" block
                    │     └── OutputStmt("positive")
                    ├── consume(TOKEN_DEDENT)
                    ├── match(TOKEN_ELSE) → true
                    ├── consume(TOKEN_COLON)
                    ├── consume(TOKEN_NEWLINE)
                    ├── consume(TOKEN_INDENT)
                    ├── statements() - parse "else" block
                    │     └── OutputStmt("non-positive")
                    └── consume(TOKEN_DEDENT)
```

**Code Excerpt (`conditionalStatement`, lines 532-590):**
```c
static ASTNode* conditionalStatement(Parser* parser) {
    int line = parser->previous.line;
    
    ASTNode* cond = condition(parser);
    
    match(parser, TOKEN_THEN);  // Optional noise word
    
    consume(parser, TOKEN_COLON, "Expected ':' after condition");
    consume(parser, TOKEN_NEWLINE, "Expected newline after ':'");
    consume(parser, TOKEN_INDENT, "Expected indentation after 'when:'");
    
    ASTNodeList* thenBranch = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after 'when' block");
    
    ASTNodeList* elseBranch = NULL;
    
    skipNewlines(parser);
    
    if (match(parser, TOKEN_ELSE)) {
        // Handle else or else-when
        consume(parser, TOKEN_COLON, "Expected ':' after 'else'");
        consume(parser, TOKEN_NEWLINE, "Expected newline after ':'");
        consume(parser, TOKEN_INDENT, "Expected indentation after 'else:'");
        
        elseBranch = statements(parser);
        
        consume(parser, TOKEN_DEDENT, "Expected dedent after 'else' block");
    }
    
    return createIfStmt(cond, thenBranch, elseBranch, line);
}
```

**Final AST:**

```
Program
└── IfStmt
    ├── condition: BinaryOp(>, Identifier("x"), IntLiteral(0))
    ├── thenBranch: [
    │     OutputStmt { expressions: [StringLiteral("positive")] }
    │   ]
    └── elseBranch: [
          OutputStmt { expressions: [StringLiteral("non-positive")] }
        ]
```

### Example 3: Function with Loop

**Input:**
```eac
function sum_range(n: int): int:
    flex total = 0
    for i in range(n):
        total += i
    return total
```

**Parse Trace:**

```
parse() called
  └── statements() called
        └── statement() - matches TOKEN_FUNCTION
              └── functionDeclaration()
                    ├── consume(TOKEN_IDENTIFIER) → "sum_range"
                    ├── consume(TOKEN_LPAREN)
                    ├── parse parameters:
                    │     └── VarDecl { name: "n", typeHint: int }
                    ├── consume(TOKEN_RPAREN)
                    ├── consume(TOKEN_COLON)
                    ├── check return type → TypeHint(int)
                    ├── consume(TOKEN_COLON)
                    ├── consume(TOKEN_NEWLINE)
                    ├── consume(TOKEN_INDENT)
                    ├── statements() - parse function body:
                    │     ├── VarDecl { name: "total", initializer: 0 }
                    │     ├── ForStmt {
                    │     │     iterVar: "i",
                    │     │     iterable: CallExpr("range", [Identifier("n")]),
                    │     │     body: [
                    │     │       CompoundAssign(total, +=, Identifier("i"))
                    │     │     ]
                    │     │   }
                    │     └── ReturnStmt { value: Identifier("total") }
                    └── consume(TOKEN_DEDENT)
```

**Code Excerpt (`functionDeclaration`, lines 731-795):**
```c
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
                // Parse parameter type hint
                // ...
            }
            
            ASTNode* param = createVarDecl(false, paramName, paramType, NULL, paramLine);
            addNode(paramsList, param);
            free(paramName);
            
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
    
    // Parse optional return type
    ASTNode* returnType = NULL;
    if (check(parser, TOKEN_COLON)) {
        consume(parser, TOKEN_COLON, "Expected ':'");
        // Check for type hint
        // ...
    }
    
    consume(parser, TOKEN_NEWLINE, "Expected newline before function body.");
    consume(parser, TOKEN_INDENT, "Expected indentation for function body.");
    
    ASTNodeList* body = statements(parser);
    
    consume(parser, TOKEN_DEDENT, "Expected dedent after function body.");
    
    ASTNode* funcNode = createFuncDecl(name, paramsNode, returnType, body, line);
    free(name);
    return funcNode;
}
```

**Final AST:**

```
Program
└── FuncDecl
    ├── name: "sum_range"
    ├── params: [
    │     VarDecl { name: "n", typeHint: int }
    │   ]
    ├── returnType: TypeHint(int)
    └── body: [
          VarDecl { name: "total", initializer: IntLiteral(0) },
          ForStmt {
            iterVar: "i",
            iterable: CallExpr {
              name: "range",
              args: [Identifier("n")]
            },
            body: [
              CompoundAssign {
                name: "total",
                op: TOKEN_PLUS_EQUAL,
                value: Identifier("i")
              }
            ]
          },
          ReturnStmt { value: Identifier("total") }
        ]
```

---

## Summary

The EaC parser:

1. **Initializes** with a lexer and loads the first token (`initParser`)
2. **Parses statements** by checking the current token type and dispatching to specific parsers
3. **Parses expressions** using precedence climbing through recursive function calls
4. **Builds AST nodes** for each recognized construct
5. **Handles errors** through panic mode and synchronization
6. **Returns** a complete AST with a Program node as the root

### Key Parser Functions

| Function | Purpose |
|----------|---------|
| `initParser()` | Initialize parser with lexer |
| `parse()` | Main entry point, returns AST |
| `statements()` | Parse list of statements |
| `statement()` | Dispatch to specific statement parser |
| `expression()` | Parse expression (starts with `logicalExpr`) |
| `primary()` | Parse literals, identifiers, grouped expressions |
| `consume()` | Require specific token or report error |
| `match()` | Check and optionally consume token |
| `synchronize()` | Recover from errors to continue parsing |

### Error Recovery Strategy

1. **Report** the first error with location and context
2. **Enter** panic mode to suppress cascading errors
3. **Synchronize** by skipping tokens until a statement boundary
4. **Continue** parsing to find additional errors
5. **Check** `hasError()` to determine if compilation should proceed
