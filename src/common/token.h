#ifndef EAC_TOKEN_H
#define EAC_TOKEN_H
typedef enum {
    // ===== Lifecycle Tokens =====
    TOKEN_EOF,              // End of file
    TOKEN_ERROR,            // Error token (with message)
    
    // ===== Whitespace & Structural Tokens =====
    TOKEN_NEWLINE,          // Newline (statement terminator)
    TOKEN_INDENT,           // Indentation increase
    TOKEN_DEDENT,           // Indentation decrease
    
    // ===== Literals =====
    TOKEN_IDENTIFIER,       // Variable/function names
    TOKEN_INTEGER,          // Integer literal (e.g., 42)
    TOKEN_FLOAT,            // Float literal (e.g., 3.14)
    TOKEN_CHAR,             // Character literal (e.g., 'a')
    TOKEN_STRING,           // String literal (e.g., "hello")
    TOKEN_COMMENT_LINE,     // # line comment
    TOKEN_COMMENT_BLOCK,    // /* block comment */
    
    // ===== Primary Keywords =====
    TOKEN_FLEX,             // flex - mutable variable declaration
    TOKEN_FIXED,            // fixed - immutable constant declaration
    TOKEN_WHEN,             // when - conditional statement (if)
    TOKEN_ELSE,             // else - alternative conditional branch
    TOKEN_OUTPUT,           // output - print function
    TOKEN_WHILE,            // while - while loop
    TOKEN_FOR,              // for - for loop
    TOKEN_IN,               // in - iteration operator
    TOKEN_BREAK,            // break - exit loop
    TOKEN_CONTINUE,         // continue - skip to next iteration
    TOKEN_RETURN,           // return - exit function
    TOKEN_FUNCTION,         // function - declare a function
    TOKEN_IMPORT,           // import - module import
    TOKEN_FROM,             // from - import source specifier
    TOKEN_TRUE,             // true - boolean literal
    TOKEN_FALSE,            // false - boolean literal
    TOKEN_INPUT,            // input - input function 
    
    // ===== Type Hint Keywords (Reserved Words) =====
    TOKEN_HINT_INT,         // int - integer type hint
    TOKEN_HINT_FLOAT,       // float - float type hint
    TOKEN_HINT_STR,         // str - string type hint
    TOKEN_HINT_BOOL,        // bool - boolean type hint
    TOKEN_HINT_CHAR,        // char - character type hint
    
    // ===== Arithmetic Operators =====
    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_STAR,             // *
    TOKEN_SLASH,            // /
    TOKEN_PERCENT,          // %
    TOKEN_CARET,            // ^ (exponent)
    TOKEN_VBAR,             // | (absolute value)
    TOKEN_FLOOR_DIV,        // // (floor division)
    
    // ===== Relational & Equality Operators =====
    TOKEN_LESS,             // <
    TOKEN_GREATER,          // >
    TOKEN_EQUAL_EQUAL,      // ==
    TOKEN_LESS_EQUAL,       // <=
    TOKEN_GREATER_EQUAL,    // >=
    TOKEN_BANG_EQUAL,       // !=
    
    // ===== Logical Operators =====
    TOKEN_AND,              // and
    TOKEN_OR,               // or
    TOKEN_NOT,              // not

    // ===== Noise Words =====
    TOKEN_AS,            // Noise words that are ignored by the parser
    TOKEN_EACH,
    TOKEN_OF,
    TOKEN_TO,
    TOKEN_THEN,

    // ===== Assignment Operators =====
    TOKEN_EQUAL,            // =
    TOKEN_PLUS_EQUAL,       // +=
    TOKEN_MINUS_EQUAL,      // -=
    TOKEN_STAR_EQUAL,       // *=
    TOKEN_SLASH_EQUAL,      // /=
    TOKEN_PERCENT_EQUAL,    // %=
    
    // ===== Delimiters & Punctuation =====
    TOKEN_LPAREN,           // (
    TOKEN_RPAREN,           // )
    TOKEN_LBRACKET,         // [
    TOKEN_RBRACKET,         // ]
    TOKEN_COLON,            // :
    TOKEN_COMMA,            // ,
    TOKEN_DOT,              // .
} TokenType;

/**
 * Token - Represents a single lexical token
 * 
 * @field type      The type of the token
 * @field lexeme    Pointer to the start of the token in source code
 * @field length    Length of the token lexeme
 * @field line      Line number where the token appears
 */
typedef struct {
    TokenType type;
    const char* lexeme;   
    int length;
    int line;
} Token;

#endif // EAC_TOKEN_H
