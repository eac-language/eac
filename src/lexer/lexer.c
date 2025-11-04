/**
 * lexer.c - Lexical Analyzer Implementation for EaC Compiler
 * 
 * This file implements the complete lexical analysis phase,
 * including Python-style significant whitespace/indentation handling.
 */

#include <stddef.h>
#include <stdbool.h>

#include "lexer.h"

// Ensure NULL is defined
#ifndef NULL
#define NULL ((void*)0)
#endif

// Standard library function declarations
extern void* malloc(size_t size);
extern void free(void* ptr);
extern size_t strlen(const char* str);
extern int memcmp(const void* s1, const void* s2, size_t n);

// ===== Configuration Constants =====
#define MAX_INDENT_DEPTH 256

// ===== Lexer Structure Definition =====
/**
 * Lexer - Internal structure for lexer state
 */
struct Lexer {
    const char* source;         // Original source code
    const char* start;          // Start of current lexeme
    const char* current;        // Current character being scanned
    int line;                   // Current line number
    
    // Indentation tracking
    int indentStack[MAX_INDENT_DEPTH];  // Stack of indentation levels
    int indentCount;                     // Number of items in indent stack
    int pendingDedents;                  // Number of DEDENT tokens to emit
    
    // State flags
    bool atLineStart;           // Are we at the start of a line?
    bool needsNewline;          // Should we emit a newline token?
};

// ===== Keyword Lookup Table =====
/**
 * KeywordEntry - Entry in the keyword lookup table
 */
typedef struct {
    const char* keyword;
    TokenType type;
} KeywordEntry;

/**
 * keywords - Lookup table for all EaC keywords
 * 
 * This table provides DRY (Don't Repeat Yourself) keyword recognition.
 * All keywords are defined in one place, making maintenance easier.
 */
static const KeywordEntry keywords[] = {
    // Primary keywords
    {"flex",     TOKEN_FLEX},
    {"fixed",    TOKEN_FIXED},
    {"when",     TOKEN_WHEN},
    {"else",     TOKEN_ELSE},
    {"output",   TOKEN_OUTPUT},
    {"while",    TOKEN_WHILE},
    {"for",      TOKEN_FOR},
    {"in",       TOKEN_IN},
    {"break",    TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"return",   TOKEN_RETURN},
    {"function", TOKEN_FUNCTION},
    {"import",   TOKEN_IMPORT},
    {"from",     TOKEN_FROM},
    {"true",     TOKEN_TRUE},
    {"false",    TOKEN_FALSE},
    
    // Type hint keywords
    {"int",      TOKEN_HINT_INT},
    {"float",    TOKEN_HINT_FLOAT},
    {"str",      TOKEN_HINT_STR},
    {"bool",     TOKEN_HINT_BOOL},
    {"char",     TOKEN_HINT_CHAR},
    
    // Logical operators (word-based)
    {"and",      TOKEN_AND},
    {"or",       TOKEN_OR},
    {"not",      TOKEN_NOT},
    
    {NULL, 0}  // Sentinel
};

// ===== Noise Word Lookup Table =====
static const KeywordEntry noiseWords[] = {
    {"please",   TOKEN_NOISE},
    {"kindly",   TOKEN_NOISE},
    {"maybe",    TOKEN_NOISE},
    {NULL, 0}
};

// ===== Helper Functions - Character Classification =====

static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

// ===== Helper Functions - Lexer State =====

static inline bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static inline char peek(Lexer* lexer) {
    return *lexer->current;
}

static inline char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

static bool match(Lexer* lexer, char expected) {
    if (isAtEnd(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    return true;
}

// ===== Helper Functions - Token Creation =====

static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    return token;
}

static Token syntheticToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = "";
    token.length = 0;
    token.line = lexer->line;
    return token;
}

// ===== Comment Handling =====

static Token scanLineComment(Lexer* lexer) {
    while (peek(lexer) != '\n' && !isAtEnd(lexer)) {
        advance(lexer);
    }
    return makeToken(lexer, TOKEN_COMMENT_LINE);
}

static Token scanBlockComment(Lexer* lexer) {
    while (!isAtEnd(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            advance(lexer);
        } else if (peek(lexer) == '*' && peekNext(lexer) == '/') {
            advance(lexer);
            advance(lexer);
            return makeToken(lexer, TOKEN_COMMENT_BLOCK);
        } else {
            advance(lexer);
        }
    }
    return errorToken(lexer, "Unterminated block comment.");
}

static void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            default:
                return;
        }
    }
}

// ===== Keyword Lookup =====

static TokenType identifierType(Lexer* lexer) {
    int length = (int)(lexer->current - lexer->start);
    
    // Linear search through keyword table
    // For a small number of keywords (~25), this is efficient enough
    // Could be optimized with a hash table or trie for larger keyword sets
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        const char* kw = keywords[i].keyword;
        int kwLen = strlen(kw);
        
        if (length == kwLen && memcmp(lexer->start, kw, length) == 0) {
            return keywords[i].type;
        }
    }

    // Noise words are recognized but treated specially
    for (int i = 0; noiseWords[i].keyword != NULL; i++) {
        const char* nw = noiseWords[i].keyword;
        int nwLen = strlen(nw);

        if (length == nwLen && memcmp(lexer->start, nw, length) == 0) {
            return noiseWords[i].type;
        }
    }
    
    return TOKEN_IDENTIFIER;
}

// ===== Token Scanning Functions =====

static Token scanString(Lexer* lexer) {
    // Consume characters until closing quote or EOF
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
        }
        advance(lexer);
    }
    
    if (isAtEnd(lexer)) {
        return errorToken(lexer, "Unterminated string literal.");
    }
    
    // Consume closing quote
    advance(lexer);
    return makeToken(lexer, TOKEN_STRING);
}

static Token scanChar(Lexer* lexer) {
    if (isAtEnd(lexer)) {
        return errorToken(lexer, "Unterminated char literal.");
    }

    char c = advance(lexer);
    if (c == '\\') {
        if (isAtEnd(lexer)) {
            return errorToken(lexer, "Unterminated escape in char literal.");
        }
        advance(lexer); // Consume escaped character
    }

    if (peek(lexer) != '\'') {
        return errorToken(lexer, "Char literal must contain exactly one character.");
    }

    advance(lexer); // Consume closing quote
    return makeToken(lexer, TOKEN_CHAR);
}

static Token scanNumber(Lexer* lexer) {
    // Consume all digits
    while (isDigit(peek(lexer))) {
        advance(lexer);
    }
    
    // Check for decimal point
    if (peek(lexer) == '.' && isDigit(peekNext(lexer))) {
        // Consume the '.'
        advance(lexer);
        
        // Consume fractional part
        while (isDigit(peek(lexer))) {
            advance(lexer);
        }
        
        return makeToken(lexer, TOKEN_FLOAT);
    }
    
    return makeToken(lexer, TOKEN_INTEGER);
}

static Token scanIdentifier(Lexer* lexer) {
    // Consume all alphanumeric characters and underscores
    while (isAlphaNumeric(peek(lexer))) {
        advance(lexer);
    }
    
    // Check if it's a keyword
    TokenType type = identifierType(lexer);
    return makeToken(lexer, type);
}

// ===== Indentation Handling =====

static int measureIndentation(Lexer* lexer) {
    int indent = 0;
    const char* ptr = lexer->current;
    
    while (*ptr == ' ' || *ptr == '\t') {
        if (*ptr == '\t') {
            indent += 4; // Treat tab as 4 spaces
        } else {
            indent += 1;
        }
        ptr++;
    }
    
    return indent;
}

static Token handleIndentation(Lexer* lexer) {
    // Measure the indentation of the current line
    int newIndent = measureIndentation(lexer);
    
    // Skip the actual whitespace
    while (peek(lexer) == ' ' || peek(lexer) == '\t') {
        advance(lexer);
    }
    
    // Check if this is a blank line
    if (peek(lexer) == '\n') {
        return syntheticToken(lexer, TOKEN_EOF); // Signal to skip this line
    }
    
    int currentIndent = lexer->indentStack[lexer->indentCount - 1];
    
    if (newIndent > currentIndent) {
        // INDENT: Push new level onto stack
        if (lexer->indentCount >= MAX_INDENT_DEPTH) {
            return errorToken(lexer, "Maximum indentation depth exceeded.");
        }
        lexer->indentStack[lexer->indentCount++] = newIndent;
        lexer->start = lexer->current;
        return syntheticToken(lexer, TOKEN_INDENT);
    } else if (newIndent < currentIndent) {
        // DEDENT: Pop levels until we match
        while (lexer->indentCount > 1 && 
               lexer->indentStack[lexer->indentCount - 1] > newIndent) {
            lexer->indentCount--;
            lexer->pendingDedents++;
        }
        
        // Check for indentation error (dedent to non-existent level)
        if (lexer->indentStack[lexer->indentCount - 1] != newIndent) {
            return errorToken(lexer, "Indentation error: dedent to invalid level.");
        }
        
        // Emit one DEDENT token
        lexer->pendingDedents--;
        lexer->start = lexer->current;
        return syntheticToken(lexer, TOKEN_DEDENT);
    }
    
    // Same indentation level - no INDENT/DEDENT needed
    return syntheticToken(lexer, TOKEN_EOF); // Signal to continue scanning
}

// ===== Main Token Scanner =====

static Token scanToken(Lexer* lexer) {
    // Handle pending dedents first
    if (lexer->pendingDedents > 0) {
        lexer->pendingDedents--;
        return syntheticToken(lexer, TOKEN_DEDENT);
    }
    
    // Handle pending newline
    if (lexer->needsNewline) {
        lexer->needsNewline = false;
        lexer->atLineStart = true;
        return syntheticToken(lexer, TOKEN_NEWLINE);
    }
    
    // Handle indentation at line start
    if (lexer->atLineStart && !isAtEnd(lexer)) {
        lexer->atLineStart = false;
        
        // Skip blank lines and measure indentation
        while (peek(lexer) == '\n' || peek(lexer) == ' ' || 
               peek(lexer) == '\t' || peek(lexer) == '\r') {
            if (peek(lexer) == '\n') {
                lexer->line++;
                advance(lexer);
            } else {
                // Start of a non-empty line
                Token indentToken = handleIndentation(lexer);
                if (indentToken.type == TOKEN_INDENT || 
                    indentToken.type == TOKEN_DEDENT ||
                    indentToken.type == TOKEN_ERROR) {
                    return indentToken;
                }
                // Otherwise, continue scanning
                break;
            }
        }
    }
    
    skipWhitespace(lexer);
    lexer->start = lexer->current;
    
    if (isAtEnd(lexer)) {
        // Emit remaining dedents at EOF
        if (lexer->indentCount > 1) {
            lexer->indentCount--;
            return syntheticToken(lexer, TOKEN_DEDENT);
        }
        return makeToken(lexer, TOKEN_EOF);
    }
    
    char c = advance(lexer);
    
    // Identifiers and keywords
    if (isAlpha(c)) {
        return scanIdentifier(lexer);
    }
    
    // Numbers
    if (isDigit(c)) {
        return scanNumber(lexer);
    }
    
    // Multi-character operators and single-character tokens
    switch (c) {
        // Newline
        case '\n':
            lexer->line++;
            lexer->atLineStart = true;
            return syntheticToken(lexer, TOKEN_NEWLINE);
        
        // String literals
        case '"':
            return scanString(lexer);

        // Character literals
        case '\'':
            return scanChar(lexer);
        
        // Delimiters
        case '(':  return makeToken(lexer, TOKEN_LPAREN);
        case ')':  return makeToken(lexer, TOKEN_RPAREN);
        case '[':  return makeToken(lexer, TOKEN_LBRACKET);
        case ']':  return makeToken(lexer, TOKEN_RBRACKET);
        case ':':  return makeToken(lexer, TOKEN_COLON);
        case ',':  return makeToken(lexer, TOKEN_COMMA);
        case '.':  return makeToken(lexer, TOKEN_DOT);
        
        // Arithmetic operators (with compound assignment)
        case '+':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
        case '-':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
        case '*':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
        case '%':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
        
        // Division and comments
        case '/':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_SLASH_EQUAL);
            } else if (match(lexer, '*')) {
                return scanBlockComment(lexer);
            }
            return makeToken(lexer, TOKEN_SLASH);
        
        // Exponent and absolute value
        case '^':  return makeToken(lexer, TOKEN_CARET);
        case '|':  return makeToken(lexer, TOKEN_VBAR);
        
        // Relational and equality operators
        case '=':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '!':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_BANG_EQUAL);
            }
            return errorToken(lexer, "Unexpected character '!'.");
        
        // Line comments
        case '#':
            return scanLineComment(lexer);
    }
    
    return errorToken(lexer, "Unexpected character.");
}

// ===== Public API Implementation =====

Lexer* initLexer(const char* source) {
    if (source == NULL) {
        return NULL;
    }
    
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        return NULL;
    }
    
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    
    // Initialize indentation tracking
    lexer->indentStack[0] = 0; // Base indentation level
    lexer->indentCount = 1;
    lexer->pendingDedents = 0;
    
    // Initialize state flags
    lexer->atLineStart = true;
    lexer->needsNewline = false;
    
    return lexer;
}

Token getNextToken(Lexer* lexer) {
    if (lexer == NULL) {
        Token errorTok;
        errorTok.type = TOKEN_ERROR;
        errorTok.lexeme = "Lexer is NULL";
        errorTok.length = 13;
        errorTok.line = 0;
        return errorTok;
    }
    
    return scanToken(lexer);
}

void freeLexer(Lexer* lexer) {
    if (lexer != NULL) {
        free(lexer);
    }
}
