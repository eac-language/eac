#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "../common/token.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Lexer;

Lexer lexer;

void initLexer(const char* source) {
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

static bool isAtEnd() {
    return *lexer.current == '\0';
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer.line;
    return token;
}

static char advance() {
    lexer.current++;
    return lexer.current[-1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*lexer.current != expected) return false;
    lexer.current++;
    return true;
}

static void skipWhitespace() {
    for (;;) {
        char c = *lexer.current;
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                lexer.line++;
                advance();
                break;
            case '#':
                while (*lexer.current != '\n' && !isAtEnd()) {
                    advance();
                }
                break;
            case '/':
                if (lexer.current[1] == '*') {
                    advance(); // Consume the '/'
                    advance(); // Consume the '*'
                    while (!isAtEnd() && (*lexer.current != '*' || lexer.current[1] != '/')) {
                        if (*lexer.current == '\n') lexer.line++;
                        advance();
                    }
                    if (!isAtEnd()) advance(); // Consume the '*'
                    if (!isAtEnd()) advance(); // Consume the '/'
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (lexer.start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'b':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'o': return checkKeyword(2, 2, "ol", TOKEN_BOOL_KEYWORD);
                    case 'r': return checkKeyword(2, 3, "eak", TOKEN_BREAK);
                }
            }
            break;
        case 'c': return checkKeyword(1, 7, "ontinue", TOKEN_CONTINUE);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'i': return checkKeyword(2, 3, "xed", TOKEN_FIXED);
                    case 'l':
                        if (lexer.current - lexer.start > 2) {
                            switch (lexer.start[2]) {
                                case 'e': return checkKeyword(3, 1, "x", TOKEN_FLEX);
                                case 'o': return checkKeyword(3, 2, "at", TOKEN_FLOAT_KEYWORD);
                            }
                        }
                        break;
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'i':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'n':
                        if (lexer.current - lexer.start == 2) return TOKEN_IN;
                        return checkKeyword(2, 1, "t", TOKEN_INT_KEYWORD);
                }
            }
            break;
        case 'n': return checkKeyword(1, 2, "ot", TOKEN_NOT);
        case 'o':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'r': return TOKEN_OR;
                    case 'u': return checkKeyword(2, 4, "tput", TOKEN_OUTPUT);
                }
            }
            break;
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 2, "tr", TOKEN_STR_KEYWORD);
        case 't': return checkKeyword(1, 3, "rue", TOKEN_TRUE);
        case 'w':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'h':
                        if (lexer.current - lexer.start > 2) {
                            switch (lexer.start[2]) {
                                case 'e': return checkKeyword(3, 1, "n", TOKEN_WHEN);
                                case 'i': return checkKeyword(3, 2, "le", TOKEN_WHILE);
                            }
                        }
                        break;
                }
            }
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isalpha(*lexer.current) || isdigit(*lexer.current) || *lexer.current == '_') {
        advance();
    }
    return makeToken(identifierType());
}

static Token number() {
    while (isdigit(*lexer.current)) {
        advance();
    }

    if (*lexer.current == '.' && isdigit(lexer.current[1])) {
        advance();
        while (isdigit(*lexer.current)) {
            advance();
        }
        return makeToken(TOKEN_FLOAT);
    }

    return makeToken(TOKEN_INTEGER);
}

static Token string() {
    while (*lexer.current != '"' && !isAtEnd()) {
        if (*lexer.current == '\n') lexer.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    advance(); // The closing quote.
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipWhitespace();
    lexer.start = lexer.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isalpha(c) || c == '_') return identifier();
    if (isdigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TOKEN_LPAREN);
        case ')': return makeToken(TOKEN_RPAREN);
        case '[': return makeToken(TOKEN_LBRACKET);
        case ']': return makeToken(TOKEN_RBRACKET);
        case ':': return makeToken(TOKEN_COLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '+': return makeToken(match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
        case '-': return makeToken(match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
        case '*': return makeToken(match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
        case '/': return makeToken(match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
        case '%': return makeToken(match('=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
        case '^': return makeToken(TOKEN_CARET);
        case '|': return makeToken(TOKEN_PIPE);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_ERROR); // Assuming '!' is only for '!='
        case '"': return string();
    }

    return errorToken("Unexpected character.");
}
