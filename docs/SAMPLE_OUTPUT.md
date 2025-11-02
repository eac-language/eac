# EaC Lexer - Sample Output

This document shows sample output from the lexer for various EaC code snippets.

## Simple Variable Declaration

### Input (`example1.eac`):
```eac
flex age = 25
```

### Output:
```
===============================================
    EaC Lexical Analyzer - Test Harness
===============================================

Reading source file: example1.eac

Tokens:
-----------------------------------------------
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'age'
Line    1 | EQUAL              | '='
Line    1 | INTEGER            | '25'
Line    1 | NEWLINE            
Line    2 | EOF                
-----------------------------------------------
Total tokens: 6
Status: SUCCESS
===============================================
```

## Variable with Type Hint

### Input:
```eac
flex count: int = 42
fixed PI: float = 3.14
```

### Output:
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'count'
Line    1 | COLON              | ':'
Line    1 | HINT_INT           | 'int'
Line    1 | EQUAL              | '='
Line    1 | INTEGER            | '42'
Line    1 | NEWLINE            
Line    2 | FIXED              | 'fixed'
Line    2 | IDENTIFIER         | 'PI'
Line    2 | COLON              | ':'
Line    2 | HINT_FLOAT         | 'float'
Line    2 | EQUAL              | '='
Line    2 | FLOAT              | '3.14'
Line    2 | NEWLINE            
Line    3 | EOF                
```

## Conditional with Indentation

### Input:
```eac
when x > 5:
    output("yes")
else:
    output("no")
```

### Output:
```
Line    1 | WHEN               | 'when'
Line    1 | IDENTIFIER         | 'x'
Line    1 | GREATER            | '>'
Line    1 | INTEGER            | '5'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | OUTPUT             | 'output'
Line    2 | LPAREN             | '('
Line    2 | STRING             | '"yes"'
Line    2 | RPAREN             | ')'
Line    2 | NEWLINE            
Line    3 | DEDENT             
Line    3 | ELSE               | 'else'
Line    3 | COLON              | ':'
Line    3 | NEWLINE            
Line    4 | INDENT             
Line    4 | OUTPUT             | 'output'
Line    4 | LPAREN             | '('
Line    4 | STRING             | '"no"'
Line    4 | RPAREN             | ')'
Line    4 | NEWLINE            
Line    5 | DEDENT             
Line    5 | EOF                
```

## Nested Indentation

### Input:
```eac
when a > 0:
    when b > 0:
        output("both positive")
    output("a positive")
```

### Output:
```
Line    1 | WHEN               | 'when'
Line    1 | IDENTIFIER         | 'a'
Line    1 | GREATER            | '>'
Line    1 | INTEGER            | '0'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | WHEN               | 'when'
Line    2 | IDENTIFIER         | 'b'
Line    2 | GREATER            | '>'
Line    2 | INTEGER            | '0'
Line    2 | COLON              | ':'
Line    2 | NEWLINE            
Line    3 | INDENT             
Line    3 | OUTPUT             | 'output'
Line    3 | LPAREN             | '('
Line    3 | STRING             | '"both positive"'
Line    3 | RPAREN             | ')'
Line    3 | NEWLINE            
Line    4 | DEDENT             
Line    4 | OUTPUT             | 'output'
Line    4 | LPAREN             | '('
Line    4 | STRING             | '"a positive"'
Line    4 | RPAREN             | ')'
Line    4 | NEWLINE            
Line    5 | DEDENT             
Line    5 | EOF                
```

## All Operators

### Input:
```eac
flex x = 10 + 5 - 3 * 2 / 4 % 3
flex y = x ^ 2
flex z = |x|
```

### Output:
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'x'
Line    1 | EQUAL              | '='
Line    1 | INTEGER            | '10'
Line    1 | PLUS               | '+'
Line    1 | INTEGER            | '5'
Line    1 | MINUS              | '-'
Line    1 | INTEGER            | '3'
Line    1 | STAR               | '*'
Line    1 | INTEGER            | '2'
Line    1 | SLASH              | '/'
Line    1 | INTEGER            | '4'
Line    1 | PERCENT            | '%'
Line    1 | INTEGER            | '3'
Line    1 | NEWLINE            
Line    2 | FLEX               | 'flex'
Line    2 | IDENTIFIER         | 'y'
Line    2 | EQUAL              | '='
Line    2 | IDENTIFIER         | 'x'
Line    2 | CARET              | '^'
Line    2 | INTEGER            | '2'
Line    2 | NEWLINE            
Line    3 | FLEX               | 'flex'
Line    3 | IDENTIFIER         | 'z'
Line    3 | EQUAL              | '='
Line    3 | VBAR               | '|'
Line    3 | IDENTIFIER         | 'x'
Line    3 | VBAR               | '|'
Line    3 | NEWLINE            
Line    4 | EOF                
```

## Relational and Logical Operators

### Input:
```eac
flex test = a < b and c > d or not e
flex equal = x == y
flex notequal = x != y
```

### Output:
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'test'
Line    1 | EQUAL              | '='
Line    1 | IDENTIFIER         | 'a'
Line    1 | LESS               | '<'
Line    1 | IDENTIFIER         | 'b'
Line    1 | AND                | 'and'
Line    1 | IDENTIFIER         | 'c'
Line    1 | GREATER            | '>'
Line    1 | IDENTIFIER         | 'd'
Line    1 | OR                 | 'or'
Line    1 | NOT                | 'not'
Line    1 | IDENTIFIER         | 'e'
Line    1 | NEWLINE            
Line    2 | FLEX               | 'flex'
Line    2 | IDENTIFIER         | 'equal'
Line    2 | EQUAL              | '='
Line    2 | IDENTIFIER         | 'x'
Line    2 | EQUAL_EQUAL        | '=='
Line    2 | IDENTIFIER         | 'y'
Line    2 | NEWLINE            
Line    3 | FLEX               | 'flex'
Line    3 | IDENTIFIER         | 'notequal'
Line    3 | EQUAL              | '='
Line    3 | IDENTIFIER         | 'x'
Line    3 | BANG_EQUAL         | '!='
Line    3 | IDENTIFIER         | 'y'
Line    3 | NEWLINE            
Line    4 | EOF                
```

## Assignment Operators

### Input:
```eac
x += 10
x -= 5
x *= 2
x /= 4
x %= 3
```

### Output:
```
Line    1 | IDENTIFIER         | 'x'
Line    1 | PLUS_EQUAL         | '+='
Line    1 | INTEGER            | '10'
Line    1 | NEWLINE            
Line    2 | IDENTIFIER         | 'x'
Line    2 | MINUS_EQUAL        | '-='
Line    2 | INTEGER            | '5'
Line    2 | NEWLINE            
Line    3 | IDENTIFIER         | 'x'
Line    3 | STAR_EQUAL         | '*='
Line    3 | INTEGER            | '2'
Line    3 | NEWLINE            
Line    4 | IDENTIFIER         | 'x'
Line    4 | SLASH_EQUAL        | '/='
Line    4 | INTEGER            | '4'
Line    4 | NEWLINE            
Line    5 | IDENTIFIER         | 'x'
Line    5 | PERCENT_EQUAL      | '%='
Line    5 | INTEGER            | '3'
Line    5 | NEWLINE            
Line    6 | EOF                
```

## Comments

### Input:
```eac
# This is a comment
flex x = 10  # Inline comment

/*
 * Multi-line
 * comment
 */
flex y = 20
```

### Output:
```
Line    2 | FLEX               | 'flex'
Line    2 | IDENTIFIER         | 'x'
Line    2 | EQUAL              | '='
Line    2 | INTEGER            | '10'
Line    2 | NEWLINE            
Line    8 | FLEX               | 'flex'
Line    8 | IDENTIFIER         | 'y'
Line    8 | EQUAL              | '='
Line    8 | INTEGER            | '20'
Line    8 | NEWLINE            
Line    9 | EOF                
```

Note: Comments are completely skipped by the lexer and do not appear in the token stream.

## While Loop

### Input:
```eac
while x < 10:
    output(x)
    x += 1
```

### Output:
```
Line    1 | WHILE              | 'while'
Line    1 | IDENTIFIER         | 'x'
Line    1 | LESS               | '<'
Line    1 | INTEGER            | '10'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | OUTPUT             | 'output'
Line    2 | LPAREN             | '('
Line    2 | IDENTIFIER         | 'x'
Line    2 | RPAREN             | ')'
Line    2 | NEWLINE            
Line    3 | IDENTIFIER         | 'x'
Line    3 | PLUS_EQUAL         | '+='
Line    3 | INTEGER            | '1'
Line    3 | NEWLINE            
Line    4 | DEDENT             
Line    4 | EOF                
```

## For Loop

### Input:
```eac
for i in range(10):
    output(i)
```

### Output:
```
Line    1 | FOR                | 'for'
Line    1 | IDENTIFIER         | 'i'
Line    1 | IN                 | 'in'
Line    1 | IDENTIFIER         | 'range'
Line    1 | LPAREN             | '('
Line    1 | INTEGER            | '10'
Line    1 | RPAREN             | ')'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | OUTPUT             | 'output'
Line    2 | LPAREN             | '('
Line    2 | IDENTIFIER         | 'i'
Line    2 | RPAREN             | ')'
Line    2 | NEWLINE            
Line    3 | DEDENT             
Line    3 | EOF                
```

## Boolean Literals

### Input:
```eac
flex flag1 = true
flex flag2 = false
```

### Output:
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'flag1'
Line    1 | EQUAL              | '='
Line    1 | TRUE               | 'true'
Line    1 | NEWLINE            
Line    2 | FLEX               | 'flex'
Line    2 | IDENTIFIER         | 'flag2'
Line    2 | EQUAL              | '='
Line    2 | FALSE              | 'false'
Line    2 | NEWLINE            
Line    3 | EOF                
```

## Error Examples

### Unterminated String

#### Input:
```eac
flex name = "Alice
```

#### Output:
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'name'
Line    1 | EQUAL              | '='
Line    1 | ERROR              | Error: Unterminated string literal.
-----------------------------------------------
Total tokens: 4
Status: LEXICAL ERRORS DETECTED
===============================================
```

### Invalid Indentation

#### Input:
```eac
when x > 5:
    output("yes")
  output("wrong")
```

#### Output:
```
Line    1 | WHEN               | 'when'
Line    1 | IDENTIFIER         | 'x'
Line    1 | GREATER            | '>'
Line    1 | INTEGER            | '5'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | OUTPUT             | 'output'
Line    2 | LPAREN             | '('
Line    2 | STRING             | '"yes"'
Line    2 | RPAREN             | ')'
Line    2 | NEWLINE            
Line    3 | ERROR              | Error: Indentation error: dedent to invalid level.
-----------------------------------------------
Total tokens: 13
Status: LEXICAL ERRORS DETECTED
===============================================
```

## Key Observations

1. **Line Numbers**: Every token shows the line where it appears
2. **Token Types**: Clear token type names (e.g., FLEX, IDENTIFIER, EQUAL)
3. **Lexemes**: Original text from source (in single quotes)
4. **Structural Tokens**: NEWLINE, INDENT, DEDENT are emitted correctly
5. **Comments**: Completely ignored, no tokens generated
6. **Errors**: Clear error messages with line numbers
7. **EOF**: Always the last token

## Token Summary

The lexer recognizes 89 distinct token types across these categories:
- Lifecycle (2): EOF, ERROR
- Structural (3): NEWLINE, INDENT, DEDENT
- Literals (4): INTEGER, FLOAT, STRING, IDENTIFIER
- Keywords (13): flex, fixed, when, else, output, while, for, in, break, continue, return, true, false
- Type Hints (4): int, float, str, bool
- Arithmetic (7): +, -, *, /, %, ^, |
- Relational (6): <, >, ==, <=, >=, !=
- Logical (3): and, or, not
- Assignment (6): =, +=, -=, *=, /=, %=
- Delimiters (7): (, ), [, ], :, ,, .

This comprehensive token set enables full parsing of the EaC language.

