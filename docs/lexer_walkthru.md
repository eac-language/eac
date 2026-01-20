# EaC Lexer Walkthrough

This document provides a detailed, step-by-step walkthrough of how the EaC lexer processes source code, including input/output examples and error handling mechanisms.

---

## Table of Contents

1. [Overview](#overview)
2. [Lexer Architecture](#lexer-architecture)
3. [Step-by-Step Walkthrough](#step-by-step-walkthrough)
4. [Token Types](#token-types)
5. [Error Handling](#error-handling)
6. [Indentation Handling](#indentation-handling)
7. [Complete Examples](#complete-examples)

---

## Overview

The EaC lexer is a **finite state machine (FSM)** that converts source code text into a stream of tokens. It uses a **maximal munch** strategy, meaning it always matches the longest possible valid token.

### Key Files

| File | Purpose |
|------|---------|
| `src/lexer/lexer.c` | Main lexer implementation |
| `src/lexer/lexer.h` | Lexer interface definitions |
| `src/common/token.h` | Token type definitions |

---

## Lexer Architecture

### Data Structures

#### Lexer Structure (`src/lexer/lexer.c`, lines 69-97)

```c
struct Lexer {
    const char* source;        // Pointer to source code
    const char* start;         // Start of current token
    const char* current;       // Current position in source
    int line;                  // Current line number
    
    // Indentation tracking
    int indentStack[MAX_INDENT_LEVELS];  // Stack of indentation levels
    int indentCount;                      // Number of indent levels
    
    // Token queue for INDENT/DEDENT tokens
    Token tokenQueue[MAX_TOKEN_QUEUE];
    int queueStart;
    int queueEnd;
    int queueSize;
    
    // State flags
    bool atLineStart;          // At beginning of line?
    bool seenEOF;              // Seen end of file?
    bool justEmittedNewline;   // Just produced a newline token?
    
    // Column tracking
    int column;
    int tokenStartColumn;
};
```

#### Token Structure (`src/common/token.h`, lines 104-110)

```c
typedef struct {
    TokenType type;        // Type of the token (e.g., TOKEN_INTEGER)
    const char* lexeme;    // Pointer to start of token in source
    int length;            // Length of the token
    int line;              // Line number
    int column;            // Column number
} Token;
```

### State Machine

The lexer uses an enumeration of states (`State`) to track its position within token recognition. Key states include:

| State | Description |
|-------|-------------|
| `Q_START` | Initial state, waiting for first character |
| `Q_IDENTIFIER` | Recognizing an identifier |
| `Q_NUMBER_INT` | Recognizing an integer |
| `Q_NUMBER_FLOAT` | Recognizing a floating-point number |
| `Q_STRING_BODY` | Inside a string literal |
| `Q_ERROR` | Error state - invalid input |

---

## Step-by-Step Walkthrough

Let's trace through how the lexer processes a simple input.

### Example Input

```eac
flex x = 42
```

### Step 1: Initialization (`initLexer`, lines 1154-1185)

**Code:**
```c
Lexer* initLexer(const char* source) {
    if (source == NULL) return NULL;
    
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) return NULL;
    
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->tokenStartColumn = 1;
    
    // Initialize indentation tracking
    lexer->indentCount = 0;
    lexer->indentStack[0] = 0;
    
    // Initialize token queue
    initQueue(lexer);
    
    // Track state
    lexer->atLineStart = true;
    lexer->seenEOF = false;
    lexer->justEmittedNewline = false;
    
    return lexer;
}
```

**State After Initialization:**
```
source: "flex x = 42"
         ^
         current (position 0)
line: 1
column: 1
indentStack: [0]
```

### Step 2: Get First Token - "flex" (`getNextToken`, lines 1187-1260)

**Entry Point:**
```c
Token getNextToken(Lexer* lexer) {
    if (lexer == NULL) {
        // Return error token
    }
    
    // Check token queue first (for INDENT/DEDENT)
    if (!queueIsEmpty(lexer)) {
        return dequeueToken(lexer);
    }
    
    // ... continues to scanToken
}
```

#### Step 2a: Skip Whitespace (`scanToken`, lines 1016-1026)

```c
static Token scanToken(Lexer* lexer) {
    // Skip whitespace (spaces, tabs, carriage returns - but NOT newlines)
    while (*lexer->current == ' ' || *lexer->current == '\r' || *lexer->current == '\t') {
        if (*lexer->current == '\t') {
            lexer->column += 4;
        } else if (*lexer->current == ' ') {
            lexer->column++;
        }
        lexer->current++;
    }
    
    lexer->start = lexer->current;
    lexer->tokenStartColumn = lexer->column;
```

**No whitespace at start, so:**
```
source: "flex x = 42"
         ^
         start, current (position 0)
```

#### Step 2b: State Machine Transitions (`transition` function, lines 103-748)

The lexer enters the main scanning loop:

```c
State state = Q_START;
State lastAccept = Q_ERROR;
const char* lastAcceptPos = lexer->start;

while (*lexer->current != '\0') {
    char c = *lexer->current;
    State next = transition(state, c);
    // ...
}
```

**Transition Trace for "flex":**

| Step | Current Char | Current State | Next State | Is Accepting? |
|------|--------------|---------------|------------|---------------|
| 1 | 'f' | Q_START | Q_F | Yes |
| 2 | 'l' | Q_F | Q_FL | No |
| 3 | 'e' | Q_FL | Q_FLE | No |
| 4 | 'x' | Q_FLE | Q_FLEX | Yes |
| 5 | ' ' | Q_FLEX | Q_ERROR | - |

**Key Code - Transition for 'f' from Q_START (lines 105-117):**
```c
case Q_START:
    if (c == 'a') return Q_A;
    if (c == 'b') return Q_B;
    if (c == 'c') return Q_C;
    if (c == 'e') return Q_E;
    if (c == 'f') return Q_F;  // <-- This matches!
    // ...
```

**Key Code - Transition for 'l' from Q_F (lines 287-295):**
```c
case Q_F:
    if (c == 'l') return Q_FL;  // <-- This matches!
    if (c == 'i') return Q_FI;
    if (c == 'o') return Q_FO;
    if (c == 'r') return Q_FR;
    if (c == 'u') return Q_FU;
    if (c == 'a') return Q_FA;
    if (IS_ALNUM(c)) return Q_IDENTIFIER;
    return Q_ERROR;
```

#### Step 2c: Token Creation (`makeToken`, lines 835-843)

When we hit Q_ERROR (on space), we backtrack to last accepting state (Q_FLEX):

```c
static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = lexer->start;
    token.length = (int)(lexer->current - lexer->start);  // 4 chars
    token.line = lexer->line;                              // 1
    token.column = lexer->tokenStartColumn;                // 1
    return token;
}
```

**Token Type Mapping (`getTokenType`, lines 762-833):**
```c
static TokenType getTokenType(State state) {
    switch (state) {
        // ...
        case Q_FLEX: return TOKEN_FLEX;
        // ...
    }
}
```

**Output Token:**
```
Token {
    type: TOKEN_FLEX
    lexeme: "flex"
    length: 4
    line: 1
    column: 1
}
```

### Step 3: Get Second Token - "x"

**After first token:**
```
source: "flex x = 42"
             ^
             current (position 4, pointing to space)
```

#### Step 3a: Skip Whitespace

The space is skipped:
```
source: "flex x = 42"
              ^
              start, current (position 5)
```

#### Step 3b: Transition Trace for "x"

| Step | Current Char | Current State | Next State | Is Accepting? |
|------|--------------|---------------|------------|---------------|
| 1 | 'x' | Q_START | Q_IDENTIFIER | Yes |
| 2 | ' ' | Q_IDENTIFIER | Q_ERROR | - |

**Key Code (lines 118):**
```c
case Q_START:
    // ... keyword checks ...
    if (IS_ALPHA(c)) return Q_IDENTIFIER;  // <-- 'x' matches this
```

**Output Token:**
```
Token {
    type: TOKEN_IDENTIFIER
    lexeme: "x"
    length: 1
    line: 1
    column: 6
}
```

### Step 4: Get Third Token - "="

**Transition Trace:**

| Step | Current Char | Current State | Next State | Is Accepting? |
|------|--------------|---------------|------------|---------------|
| 1 | '=' | Q_START | Q_EQUAL | Yes |
| 2 | ' ' | Q_EQUAL | Q_ERROR | - |

**Key Code (line 128):**
```c
case Q_START:
    // ...
    if (c == '=') return Q_EQUAL;
```

**Note:** If the next character was `=`, it would transition to `Q_EQUAL_EQUAL` for the `==` operator.

**Output Token:**
```
Token {
    type: TOKEN_EQUAL
    lexeme: "="
    length: 1
    line: 1
    column: 8
}
```

### Step 5: Get Fourth Token - "42"

**Transition Trace:**

| Step | Current Char | Current State | Next State | Is Accepting? |
|------|--------------|---------------|------------|---------------|
| 1 | '4' | Q_START | Q_NUMBER_INT | Yes |
| 2 | '2' | Q_NUMBER_INT | Q_NUMBER_INT | Yes |
| 3 | '\0' (EOF) | Q_NUMBER_INT | Loop exits | - |

**Key Code (line 119):**
```c
case Q_START:
    // ...
    if (IS_DIGIT(c)) return Q_NUMBER_INT;
```

**Output Token:**
```
Token {
    type: TOKEN_INTEGER
    lexeme: "42"
    length: 2
    line: 1
    column: 10
}
```

### Step 6: Get Fifth Token - EOF

When `*lexer->current == '\0'`:

```c
if (*lexer->current == '\0') {
    return makeToken(lexer, TOKEN_EOF);
}
```

**Output Token:**
```
Token {
    type: TOKEN_EOF
    lexeme: ""
    length: 0
    line: 1
    column: 12
}
```

---

## Token Types

The lexer recognizes the following token categories (from `src/common/token.h`):

### Lifecycle Tokens
| Token | Description |
|-------|-------------|
| `TOKEN_EOF` | End of file |
| `TOKEN_ERROR` | Error token with message |

### Whitespace & Structural
| Token | Description |
|-------|-------------|
| `TOKEN_NEWLINE` | Line terminator |
| `TOKEN_INDENT` | Indentation increase |
| `TOKEN_DEDENT` | Indentation decrease |

### Literals
| Token | Example |
|-------|---------|
| `TOKEN_IDENTIFIER` | `myVar`, `count` |
| `TOKEN_INTEGER` | `42`, `0`, `999` |
| `TOKEN_FLOAT` | `3.14`, `0.5` |
| `TOKEN_CHAR` | `'a'`, `'Z'` |
| `TOKEN_STRING` | `"hello"` |

### Keywords
| Token | Keyword |
|-------|---------|
| `TOKEN_FLEX` | `flex` |
| `TOKEN_FIXED` | `fixed` |
| `TOKEN_WHEN` | `when` |
| `TOKEN_ELSE` | `else` |
| `TOKEN_WHILE` | `while` |
| `TOKEN_FOR` | `for` |
| `TOKEN_FUNCTION` | `function` |
| `TOKEN_RETURN` | `return` |
| `TOKEN_INPUT` | `input` |
| `TOKEN_OUTPUT` | `output` |
| `TOKEN_TRUE` | `true` |
| `TOKEN_FALSE` | `false` |

---

## Error Handling

The lexer handles errors through the `errorToken` function and specific error detection in the state machine.

### Error Token Creation (`errorToken`, lines 845-853)

```c
static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = message;           // Error message, not source
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->tokenStartColumn;
    return token;
}
```

### Error Cases (`scanToken`, lines 1071-1084)

```c
// Error handling
if (state == Q_START) {
    lexer->current++;
    return errorToken(lexer, "Unexpected character.");
}
if (state == Q_STRING_BODY) 
    return errorToken(lexer, "Unterminated string literal.");
if (state >= Q_CHAR_START && state <= Q_CHAR_ESCAPE_DONE) 
    return errorToken(lexer, "Unterminated char literal.");
if (state == Q_COMMENT_BLOCK || state == Q_COMMENT_BLOCK_STAR)
    return errorToken(lexer, "Unterminated block comment.");
if (state == Q_BANG) 
    return errorToken(lexer, "Expected '=' after '!'.");
if (state == Q_NUMBER_DOT) 
    return errorToken(lexer, "Expected digit after decimal point.");

return errorToken(lexer, "Invalid token.");
```

### Error Examples

#### Example 1: Unexpected Character

**Input:**
```eac
flex x = @42
```

**Error Flow:**

1. Lexer processes `flex`, `x`, `=` normally
2. At `@`, transition from Q_START returns Q_ERROR (no match)
3. Since `state == Q_START`, the error is caught:

```c
if (state == Q_START) {
    lexer->current++;
    return errorToken(lexer, "Unexpected character.");
}
```

**Output Token:**
```
Token {
    type: TOKEN_ERROR
    lexeme: "Unexpected character."
    length: 21
    line: 1
    column: 10
}
```

#### Example 2: Unterminated String

**Input:**
```eac
flex msg = "hello
```

**Error Flow:**

1. Lexer processes `flex`, `msg`, `=` normally
2. At `"`, transitions to Q_STRING_BODY
3. Processes `h`, `e`, `l`, `l`, `o` staying in Q_STRING_BODY
4. At `\n` (newline), transition returns Q_ERROR:

```c
case Q_STRING_BODY:
    if (c == '"') return Q_STRING_END;
    if (c == '\0' || c == '\n') return Q_ERROR;  // <-- Newline triggers error
    return Q_STRING_BODY;
```

**Output Token:**
```
Token {
    type: TOKEN_ERROR
    lexeme: "Unterminated string literal."
    length: 28
    line: 1
    column: 12
}
```

#### Example 3: Invalid Number Format

**Input:**
```eac
flex x = 3.
```

**Error Flow:**

1. At `3`, transitions to Q_NUMBER_INT
2. At `.`, transitions to Q_NUMBER_DOT
3. At EOF (or non-digit), Q_NUMBER_DOT is not accepting, error triggered:

```c
if (state == Q_NUMBER_DOT) 
    return errorToken(lexer, "Expected digit after decimal point.");
```

**Output Token:**
```
Token {
    type: TOKEN_ERROR
    lexeme: "Expected digit after decimal point."
    length: 36
    line: 1
    column: 10
}
```

#### Example 4: Standalone Exclamation Mark

**Input:**
```eac
flex x = !
```

**Error Flow:**

1. At `!`, transitions to Q_BANG
2. At `\n` or EOF, Q_BANG expects `=` but doesn't get it:

```c
case Q_BANG:
    if (c == '=') return Q_BANG_EQUAL;
    return Q_ERROR;
```

**Output Token:**
```
Token {
    type: TOKEN_ERROR
    lexeme: "Expected '=' after '!'."
    length: 23
    line: 1
    column: 10
}
```

---

## Indentation Handling

EaC uses Python-like significant indentation. The lexer tracks indentation levels and emits `TOKEN_INDENT` and `TOKEN_DEDENT` tokens.

### Indentation Stack

```c
int indentStack[MAX_INDENT_LEVELS];  // Stores indentation levels
int indentCount;                      // Current stack depth
```

### Handling Logic (`handleIndentation`, lines 952-1012)

```c
static void handleIndentation(Lexer* lexer) {
    // Count leading spaces/tabs
    const char* lineStart = lexer->current;
    int spaces = countLeadingSpaces(lineStart);
    
    // Skip the whitespace
    while (*lexer->current == ' ' || *lexer->current == '\t') {
        // ... advance current ...
    }
    
    int currentLevel = currentIndent(lexer);
    
    if (spaces > currentLevel) {
        // INDENT - push new level
        pushIndent(lexer, spaces);
        Token indent = makeIndentToken(lexer);
        enqueueToken(lexer, indent);
    } else if (spaces < currentLevel) {
        // DEDENT - pop levels until we match
        while (lexer->indentCount > 0 && currentIndent(lexer) > spaces) {
            popIndent(lexer);
            Token dedent = makeDedentToken(lexer);
            enqueueToken(lexer, dedent);
        }
        
        // Check for indentation error
        if (currentIndent(lexer) != spaces) {
            Token error;
            error.type = TOKEN_ERROR;
            error.lexeme = "Indentation error";
            enqueueToken(lexer, error);
        }
    }
}
```

### Example: Nested Blocks

**Input:**
```eac
when x > 0:
    output("positive")
    when y > 0:
        output("both positive")
    output("back to first level")
output("base level")
```

**Token Stream:**

```
TOKEN_WHEN          "when"
TOKEN_IDENTIFIER    "x"
TOKEN_GREATER       ">"
TOKEN_INTEGER       "0"
TOKEN_COLON         ":"
TOKEN_NEWLINE       "\n"
TOKEN_INDENT        <INDENT>     ← Indent to 4 spaces
TOKEN_OUTPUT        "output"
TOKEN_LPAREN        "("
TOKEN_STRING        "positive"
TOKEN_RPAREN        ")"
TOKEN_NEWLINE       "\n"
TOKEN_WHEN          "when"
TOKEN_IDENTIFIER    "y"
TOKEN_GREATER       ">"
TOKEN_INTEGER       "0"
TOKEN_COLON         ":"
TOKEN_NEWLINE       "\n"
TOKEN_INDENT        <INDENT>     ← Indent to 8 spaces
TOKEN_OUTPUT        "output"
TOKEN_LPAREN        "("
TOKEN_STRING        "both positive"
TOKEN_RPAREN        ")"
TOKEN_NEWLINE       "\n"
TOKEN_DEDENT        <DEDENT>     ← Dedent from 8 to 4 spaces
TOKEN_OUTPUT        "output"
...
TOKEN_NEWLINE       "\n"
TOKEN_DEDENT        <DEDENT>     ← Dedent from 4 to 0 spaces
TOKEN_OUTPUT        "output"
...
TOKEN_EOF
```

### Indentation Error Example

**Input:**
```eac
when x > 0:
    output("level 1")
  output("invalid")    # Only 2 spaces, doesn't match any level!
```

**Error Flow:**

1. First indent pushes 4 onto stack: `[0, 4]`
2. At `output("invalid")`, spaces = 2
3. 2 < 4, so we try to dedent
4. Pop 4, stack becomes `[0]`
5. `currentIndent(lexer) == 0`, but `spaces == 2`
6. `0 != 2`, so indentation error is enqueued

---

## Complete Examples

### Example 1: Simple Variable Declaration

**Input:**
```eac
flex count = 0
```

**Complete Token Stream:**
```
| # | Type             | Lexeme  | Line | Column |
|---|------------------|---------|------|--------|
| 1 | TOKEN_FLEX       | flex    | 1    | 1      |
| 2 | TOKEN_IDENTIFIER | count   | 1    | 6      |
| 3 | TOKEN_EQUAL      | =       | 1    | 12     |
| 4 | TOKEN_INTEGER    | 0       | 1    | 14     |
| 5 | TOKEN_EOF        |         | 1    | 15     |
```

### Example 2: Function with Control Flow

**Input:**
```eac
function test():
    when true:
        return 1
    return 0
```

**Complete Token Stream:**
```
| #  | Type             | Lexeme   | Line | Column |
|----|------------------|----------|------|--------|
| 1  | TOKEN_FUNCTION   | function | 1    | 1      |
| 2  | TOKEN_IDENTIFIER | test     | 1    | 10     |
| 3  | TOKEN_LPAREN     | (        | 1    | 14     |
| 4  | TOKEN_RPAREN     | )        | 1    | 15     |
| 5  | TOKEN_COLON      | :        | 1    | 16     |
| 6  | TOKEN_NEWLINE    | \n       | 1    | 17     |
| 7  | TOKEN_INDENT     | <INDENT> | 2    | 1      |
| 8  | TOKEN_WHEN       | when     | 2    | 5      |
| 9  | TOKEN_TRUE       | true     | 2    | 10     |
| 10 | TOKEN_COLON      | :        | 2    | 14     |
| 11 | TOKEN_NEWLINE    | \n       | 2    | 15     |
| 12 | TOKEN_INDENT     | <INDENT> | 3    | 1      |
| 13 | TOKEN_RETURN     | return   | 3    | 9      |
| 14 | TOKEN_INTEGER    | 1        | 3    | 16     |
| 15 | TOKEN_NEWLINE    | \n       | 3    | 17     |
| 16 | TOKEN_DEDENT     | <DEDENT> | 4    | 1      |
| 17 | TOKEN_RETURN     | return   | 4    | 5      |
| 18 | TOKEN_INTEGER    | 0        | 4    | 12     |
| 19 | TOKEN_NEWLINE    | \n       | 4    | 13     |
| 20 | TOKEN_DEDENT     | <DEDENT> | 5    | 1      |
| 21 | TOKEN_EOF        |          | 5    | 1      |
```

### Example 3: Complex Expression with Operators

**Input:**
```eac
flex result = (10 + 5) * 2 >= 15 and x != 0
```

**Complete Token Stream:**
```
| #  | Type              | Lexeme   | Line | Column |
|----|-------------------|----------|------|--------|
| 1  | TOKEN_FLEX        | flex     | 1    | 1      |
| 2  | TOKEN_IDENTIFIER  | result   | 1    | 6      |
| 3  | TOKEN_EQUAL       | =        | 1    | 13     |
| 4  | TOKEN_LPAREN      | (        | 1    | 15     |
| 5  | TOKEN_INTEGER     | 10       | 1    | 16     |
| 6  | TOKEN_PLUS        | +        | 1    | 19     |
| 7  | TOKEN_INTEGER     | 5        | 1    | 21     |
| 8  | TOKEN_RPAREN      | )        | 1    | 22     |
| 9  | TOKEN_STAR        | *        | 1    | 24     |
| 10 | TOKEN_INTEGER     | 2        | 1    | 26     |
| 11 | TOKEN_GREATER_EQ  | >=       | 1    | 28     |
| 12 | TOKEN_INTEGER     | 15       | 1    | 31     |
| 13 | TOKEN_AND         | and      | 1    | 34     |
| 14 | TOKEN_IDENTIFIER  | x        | 1    | 38     |
| 15 | TOKEN_BANG_EQUAL  | !=       | 1    | 40     |
| 16 | TOKEN_INTEGER     | 0        | 1    | 43     |
| 17 | TOKEN_EOF         |          | 1    | 44     |
```

---

## Summary

The EaC lexer:

1. **Initializes** with source code pointer and tracking variables
2. **Scans** character-by-character using a finite state machine
3. **Recognizes** tokens using maximal munch (longest match)
4. **Handles indentation** by tracking a stack of indent levels
5. **Reports errors** with descriptive messages and location information
6. **Produces** a stream of tokens for the parser to consume

The key functions in the lexer pipeline are:
- `initLexer()` - Create and initialize the lexer
- `getNextToken()` - Main entry point for token retrieval
- `scanToken()` - Core scanning logic with FSM
- `transition()` - State machine transition function
- `handleIndentation()` - Indentation processing
- `errorToken()` - Error token generation
