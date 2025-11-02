# EaC Lexer Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────┐
│                    EaC Compiler Pipeline                │
│                                                          │
│  Source Code → [LEXER] → Tokens → Parser → AST → ...   │
│                   ↑                                      │
│                 YOU ARE HERE                             │
└─────────────────────────────────────────────────────────┘
```

## Lexer Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         CLIENT CODE                         │
│                        (main.c, Parser)                     │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       │ Public API
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                      LEXER INTERFACE                        │
│                       (lexer.h)                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │  Lexer* initLexer(const char* source)             │    │
│  │  Token getNextToken(Lexer* lexer)                 │    │
│  │  void freeLexer(Lexer* lexer)                     │    │
│  └────────────────────────────────────────────────────┘    │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       │ Implementation
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   LEXER IMPLEMENTATION                      │
│                       (lexer.c)                             │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │         Lexer State Structure          │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • source, start, current         │  │                │
│  │  │ • line number                    │  │                │
│  │  │ • indentStack[256]               │  │                │
│  │  │ • indentCount, pendingDedents    │  │                │
│  │  │ • atLineStart, needsNewline      │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │       Core Scanning Functions          │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • scanToken()     (main loop)    │  │                │
│  │  │ • scanString()                   │  │                │
│  │  │ • scanNumber()                   │  │                │
│  │  │ • scanIdentifier()               │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │      Indentation Management            │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • handleIndentation()            │  │                │
│  │  │ • measureIndentation()           │  │                │
│  │  │ • Indent stack push/pop          │  │                │
│  │  │ • INDENT/DEDENT emission         │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │       Keyword Recognition              │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • keywords[] lookup table        │  │                │
│  │  │ • identifierType()               │  │                │
│  │  │ • Linear search (~25 keywords)   │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │        Comment Handling                │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • skipLineComment()      (#)     │  │                │
│  │  │ • skipBlockComment()     (/* */) │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
│                                                              │
│  ┌────────────────────────────────────────┐                │
│  │          Helper Functions              │                │
│  │  ┌──────────────────────────────────┐  │                │
│  │  │ • peek(), peekNext()             │  │                │
│  │  │ • advance(), match()             │  │                │
│  │  │ • makeToken(), errorToken()      │  │                │
│  │  │ • isAlpha(), isDigit()           │  │                │
│  │  └──────────────────────────────────┘  │                │
│  └────────────────────────────────────────┘                │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       │ Uses
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                     TOKEN DEFINITIONS                       │
│                       (token.h)                             │
│  ┌────────────────────────────────────────────────────┐    │
│  │  TokenType enum (89 types)                         │    │
│  │    • Lifecycle (EOF, ERROR)                        │    │
│  │    • Structural (NEWLINE, INDENT, DEDENT)          │    │
│  │    • Literals (INTEGER, FLOAT, STRING, ID)         │    │
│  │    • Keywords (FLEX, FIXED, WHEN, ...)             │    │
│  │    • Operators (+, -, *, /, ==, !=, ...)           │    │
│  │    • Delimiters ((, ), [, ], :, ...)               │    │
│  └────────────────────────────────────────────────────┘    │
│  ┌────────────────────────────────────────────────────┐    │
│  │  Token struct                                       │    │
│  │    • TokenType type                                 │    │
│  │    • const char* lexeme (pointer to source)        │    │
│  │    • int length                                     │    │
│  │    • int line                                       │    │
│  └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow

```
┌──────────────┐
│ Source Code  │  "when x > 5:\n    output('yes')"
└──────┬───────┘
       │
       ▼
┌──────────────────────────────┐
│   initLexer(source)          │
│   • Allocate Lexer struct    │
│   • Initialize state         │
│   • Setup indent stack [0]   │
└──────┬───────────────────────┘
       │
       ▼
┌──────────────────────────────────────┐
│      getNextToken(lexer)             │
│                                       │
│  ┌────────────────────────────────┐  │
│  │ Check pending dedents?         │  │
│  └──────┬─────────────────────────┘  │
│         │ No                          │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ At line start?                 │  │
│  └──────┬─────────────────────────┘  │
│         │ Yes                         │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ Handle indentation             │  │
│  │ • Measure spaces/tabs          │  │
│  │ • Compare with stack           │  │
│  │ • Return INDENT/DEDENT if != │  │
│  └──────┬─────────────────────────┘  │
│         │                             │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ Skip whitespace                │  │
│  └──────┬─────────────────────────┘  │
│         │                             │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ Peek at current character      │  │
│  └──────┬─────────────────────────┘  │
│         │                             │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ Classify character:            │  │
│  │ • Alpha? → scanIdentifier()    │  │
│  │ • Digit? → scanNumber()        │  │
│  │ • Quote? → scanString()        │  │
│  │ • Symbol? → Handle operator    │  │
│  │ • #? → skipLineComment()       │  │
│  │ • /? → Check for /*            │  │
│  └──────┬─────────────────────────┘  │
│         │                             │
│         ▼                             │
│  ┌────────────────────────────────┐  │
│  │ Create Token                   │  │
│  │ • Set type                     │  │
│  │ • Set lexeme pointer           │  │
│  │ • Set length                   │  │
│  │ • Set line number              │  │
│  └──────┬─────────────────────────┘  │
└─────────┼─────────────────────────────┘
          │
          ▼
    ┌──────────┐
    │  Token   │  {WHEN, "when", 4, 1}
    └──────────┘
```

## Indentation Stack Example

```
Source:
when x > 5:
    output("a")
    when y > 10:
        output("b")
    output("c")
output("d")

Stack Evolution:
                                        
Line 1: "when x > 5:"
  Stack: [0]              No change

Line 2: "    output(a)"  (indent=4)
  Stack: [0, 4]           Emit INDENT, push 4

Line 3: "    when y > 10:" (indent=4)
  Stack: [0, 4]           No change

Line 4: "        output(b)" (indent=8)
  Stack: [0, 4, 8]        Emit INDENT, push 8

Line 5: "    output(c)"  (indent=4)
  Stack: [0, 4]           Emit DEDENT, pop 8

Line 6: "output(d)"      (indent=0)
  Stack: [0]              Emit DEDENT, pop 4
```

## Token State Machine Example

### Scanning "when"

```
Input: "when x > 5"
       ^
       start

State 0: Peek 'w' → isAlpha? Yes → scanIdentifier()
         ↓
State 1: Consume 'w', 'h', 'e', 'n'
         ↓
State 2: Peek ' ' → !isAlphaNumeric → Stop
         ↓
State 3: Look up "when" in keyword table → Found → TOKEN_WHEN
         ↓
State 4: Return token {WHEN, "when", 4, 1}
```

### Scanning "=="

```
Input: "x == 5"
          ^
          
State 0: Peek '=' → match case
         ↓
State 1: Consume '=' → Check next
         ↓
State 2: Peek '=' → match('=')? Yes
         ↓
State 3: Consume '=' → Return TOKEN_EQUAL_EQUAL
```

## Memory Layout

```
Source Code Buffer:
┌─────────────────────────────────────────┐
│ f l e x   x   =   1 0 \n w h e n ... \0 │
└─────────────────────────────────────────┘
  ↑       ↑   ↑
  │       │   │
  │       │   current (scanning '=')
  │       start (current lexeme)
  source (original)

Token (zero-copy):
┌──────────────────────────┐
│ type:   TOKEN_EQUAL      │
│ lexeme: ptr to '=' ─────┼──→ Source buffer
│ length: 1                │
│ line:   1                │
└──────────────────────────┘
```

## Component Responsibilities

### token.h (Contract)
- ✓ Define all token types
- ✓ Define Token structure
- ✓ Shared by lexer, parser, all components
- ✗ No implementation code

### lexer.h (Public Interface)
- ✓ Declare Lexer type (opaque)
- ✓ Declare public functions
- ✓ Document API
- ✗ No implementation details

### lexer.c (Implementation)
- ✓ Define Lexer structure (internal)
- ✓ Implement scanning logic
- ✓ Implement indentation tracking
- ✓ Implement keyword lookup
- ✓ Implement error handling

### main.c (Test Harness)
- ✓ Read source files
- ✓ Initialize lexer
- ✓ Loop through tokens
- ✓ Print formatted output
- ✓ Cleanup resources

## Design Patterns Used

### 1. Opaque Pointer Pattern
```c
// lexer.h (public)
typedef struct Lexer Lexer;  // Opaque

// lexer.c (private)
struct Lexer {               // Implementation hidden
    const char* source;
    // ... internal details
};
```

### 2. Factory Pattern
```c
Lexer* lexer = initLexer(source);  // Factory function
// Use lexer
freeLexer(lexer);                   // Cleanup
```

### 3. State Machine Pattern
- Each token type has scanning logic
- State transitions based on characters
- Lookahead with peek/peekNext

### 4. Table-Driven Programming
```c
static const KeywordEntry keywords[] = {
    {"flex", TOKEN_FLEX},
    // ... data-driven keyword recognition
};
```

### 5. Zero-Copy Design
- Tokens point directly into source
- No string duplication
- Fast and memory-efficient

## Performance Characteristics

```
Operation                 | Complexity
─────────────────────────┼───────────
initLexer()              | O(1)
getNextToken()           | O(1) amortized
  - Character scan       | O(1)
  - Keyword lookup       | O(k) where k = # keywords (~25)
  - Number scan          | O(d) where d = # digits
  - String scan          | O(s) where s = string length
  - Identifier scan      | O(n) where n = identifier length
freeLexer()              | O(1)
─────────────────────────┼───────────
Overall                  | O(n) where n = source length
```

## Error Handling Strategy

```
Error Detected
     │
     ▼
Create Error Token
     │
     ├── type: TOKEN_ERROR
     ├── lexeme: error message string
     ├── length: message length
     └── line: current line
     │
     ▼
Return to caller
     │
     ▼
Caller decides:
  • Stop compilation?
  • Continue for more errors?
  • Recovery strategy?
```

## Extension Points

### Adding a New Keyword
1. Add to `TokenType` enum in `token.h`
2. Add to `keywords[]` table in `lexer.c`
3. Update `getTokenName()` in `main.c`

### Adding a New Operator
1. Add to `TokenType` enum in `token.h`
2. Add case in `scanToken()` switch
3. Update `getTokenName()` in `main.c`

### Adding a New Literal Type
1. Add to `TokenType` enum in `token.h`
2. Implement `scan<Type>()` function
3. Add character classification check
4. Update `getTokenName()` in `main.c`

## Testing Strategy

```
Unit Level:
  ├── Individual token types
  ├── Keyword recognition
  ├── Operator recognition
  └── Literal recognition

Integration Level:
  ├── Comment handling
  ├── Indentation tracking
  ├── Error conditions
  └── Complex expressions

System Level:
  ├── Complete programs
  ├── Edge cases
  ├── Stress tests
  └── Real-world code
```

## Summary

The EaC lexer architecture demonstrates:
- ✅ **Clean separation of concerns**
- ✅ **Proper encapsulation**
- ✅ **Extensible design**
- ✅ **Efficient implementation**
- ✅ **Clear data flow**
- ✅ **Professional patterns**

This architecture supports the current lexing requirements while being ready for future compiler phases (parser, semantic analysis, code generation).

