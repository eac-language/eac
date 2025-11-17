#include <stddef.h>
#include <stdbool.h>

#include "lexer.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

extern void* malloc(size_t size);
extern void free(void* ptr);
extern size_t strlen(const char* str);

typedef enum {
    DFA_ERROR = -1,
    DFA_START = 0,
    
    // States for 'a' prefix (as, and)
    DFA_A = 1,
    DFA_AN = 2,
    DFA_AND = 3,
    DFA_AS = 4,
    
    // States for 'b' prefix (break, bool)
    DFA_B = 5,
    DFA_BR = 6,
    DFA_BRE = 7,
    DFA_BREA = 8,
    DFA_BREAK = 9,
    DFA_BO = 10,
    DFA_BOO = 11,
    DFA_BOOL = 12,
    
    // States for 'c' prefix (continue, char)
    DFA_C = 13,
    DFA_CO = 14,
    DFA_CON = 15,
    DFA_CONT = 16,
    DFA_CONTI = 17,
    DFA_CONTIN = 18,
    DFA_CONTINU = 19,
    DFA_CONTINUE = 20,
    DFA_CH = 21,
    DFA_CHA = 22,
    DFA_CHAR = 23,
    
    // States for 'e' prefix (else, each)
    DFA_E = 24,
    DFA_EL = 25,
    DFA_ELS = 26,
    DFA_ELSE = 27,
    DFA_EA = 28,
    DFA_EAC = 29,
    DFA_EACH = 30,
    
    // States for 'f' prefix (flex, fixed, for, from, function, false, float)
    DFA_F = 31,
    DFA_FL = 32,
    DFA_FLE = 33,
    DFA_FLEX = 34,
    DFA_FLO = 35,
    DFA_FLOA = 36,
    DFA_FLOAT_KW = 37,
    DFA_FI = 38,
    DFA_FIX = 39,
    DFA_FIXE = 40,
    DFA_FIXED = 41,
    DFA_FO = 42,
    DFA_FOR = 43,
    DFA_FR = 44,
    DFA_FRO = 45,
    DFA_FROM = 46,
    DFA_FU = 47,
    DFA_FUN = 48,
    DFA_FUNC = 49,
    DFA_FUNCT = 50,
    DFA_FUNCTI = 51,
    DFA_FUNCTIO = 52,
    DFA_FUNCTION = 53,
    DFA_FA = 54,
    DFA_FAL = 55,
    DFA_FALS = 56,
    DFA_FALSE = 57,
    
    // States for 'i' prefix (in, import, int)
    DFA_I = 58,
    DFA_IN = 59,
    DFA_IM = 60,
    DFA_IMP = 61,
    DFA_IMPO = 62,
    DFA_IMPOR = 63,
    DFA_IMPORT = 64,
    DFA_INT = 65,
    
    // States for 'n' prefix (not)
    DFA_N = 66,
    DFA_NO = 67,
    DFA_NOT = 68,
    
    // States for 'o' prefix (output, of, or)
    DFA_O = 69,
    DFA_OF = 70,
    DFA_OU = 71,
    DFA_OUT = 72,
    DFA_OUTP = 73,
    DFA_OUTPU = 74,
    DFA_OUTPUT = 75,
    DFA_OR = 76,
    
    // States for 'r' prefix (return)
    DFA_R = 77,
    DFA_RE = 78,
    DFA_RET = 79,
    DFA_RETU = 80,
    DFA_RETUR = 81,
    DFA_RETURN = 82,
    
    // States for 's' prefix (str)
    DFA_S = 83,
    DFA_ST = 84,
    DFA_STR = 85,
    
    // States for 't' prefix (to, then, true)
    DFA_T = 86,
    DFA_TO = 87,
    DFA_TH = 88,
    DFA_THE = 89,
    DFA_THEN = 90,
    DFA_TR = 91,
    DFA_TRU = 92,
    DFA_TRUE = 93,
    
    // States for 'w' prefix (when, while)
    DFA_W = 94,
    DFA_WH = 95,
    DFA_WHE = 96,
    DFA_WHEN = 97,
    DFA_WHI = 98,
    DFA_WHIL = 99,
    DFA_WHILE = 100,
    
    // Identifier state
    DFA_IDENTIFIER = 101,
    
    DFA_NUMBER_INT = 102,         // Integer part
    DFA_NUMBER_DOT = 103,         // After decimal point
    DFA_NUMBER_FLOAT = 104,       // Float with fractional part
    
    DFA_STRING_START = 105,       // After opening "
    DFA_STRING_BODY = 106,        // Inside string
    DFA_STRING_END = 107,         // After closing " (accepting)
    
    DFA_CHAR_START = 108,         // After opening '
    DFA_CHAR_BODY = 109,          // Character content
    DFA_CHAR_ESCAPE = 110,        // After backslash
    DFA_CHAR_ESCAPE_DONE = 111,   // After escape sequence
    DFA_CHAR_END = 112,           // After closing ' (accepting)
    
    DFA_COMMENT_HASH = 113,       // After #
    DFA_COMMENT_LINE_BODY = 114,  // Line comment body
    DFA_COMMENT_LINE_END = 115,   // After newline (accepting)
    DFA_COMMENT_SLASH = 116,      // After first /
    DFA_COMMENT_BLOCK = 117,      // Block comment body
    DFA_COMMENT_BLOCK_STAR = 118, // After * in block comment
    DFA_COMMENT_BLOCK_END = 119,  // After */ (accepting)
    
    DFA_PLUS = 120,               // +
    DFA_PLUS_EQUAL = 121,         // +=
    DFA_MINUS = 122,              // -
    DFA_MINUS_EQUAL = 123,        // -=
    DFA_STAR = 124,               // *
    DFA_STAR_EQUAL = 125,         // *=
    DFA_SLASH = 126,              // /
    DFA_SLASH_EQUAL = 127,        // /=
    DFA_SLASH_SLASH = 128,        // //
    DFA_PERCENT = 129,            // %
    DFA_PERCENT_EQUAL = 130,      // %=
    DFA_EQUAL = 131,              // =
    DFA_EQUAL_EQUAL = 132,        // ==
    DFA_BANG = 133,               // ! (not accepting - must be !=)
    DFA_BANG_EQUAL = 134,         // !=
    DFA_LESS = 135,               // <
    DFA_LESS_EQUAL = 136,         // <=
    DFA_GREATER = 137,            // >
    DFA_GREATER_EQUAL = 138,      // >=
    DFA_VBAR = 139,               // | (for absolute value)
    
    DFA_LPAREN = 140,             // (
    DFA_RPAREN = 141,             // )
    DFA_LBRACKET = 142,           // [
    DFA_RBRACKET = 143,           // ]
    DFA_COLON = 144,              // :
    DFA_COMMA = 145,              // ,
    DFA_DOT = 146,                // .
    DFA_CARET = 147,              // ^
    DFA_NEWLINE = 148             // \n
} DFAState;

struct Lexer {
    const char* source;
    const char* start;
    const char* current;
    int line;
};

static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

static inline bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static inline char peek(Lexer* lexer) {
    return *lexer->current;
}

static char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

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

static DFAState dfaTransition(DFAState currentState, char input) {
    switch (currentState) {
        case DFA_START:
            // Keywords and identifiers
            if (input == 'a') return DFA_A;
            if (input == 'b') return DFA_B;
            if (input == 'c') return DFA_C;
            if (input == 'e') return DFA_E;
            if (input == 'f') return DFA_F;
            if (input == 'i') return DFA_I;
            if (input == 'n') return DFA_N;
            if (input == 'o') return DFA_O;
            if (input == 'r') return DFA_R;
            if (input == 's') return DFA_S;
            if (input == 't') return DFA_T;
            if (input == 'w') return DFA_W;
            if (isAlpha(input)) return DFA_IDENTIFIER;
            
            // Numbers
            if (isDigit(input)) return DFA_NUMBER_INT;
            
            // String
            if (input == '"') return DFA_STRING_BODY;
            
            // Char
            if (input == '\'') return DFA_CHAR_START;
            
            // Comments
            if (input == '#') return DFA_COMMENT_LINE_BODY;
            if (input == '/') return DFA_COMMENT_SLASH;
            
            // Operators
            if (input == '+') return DFA_PLUS;
            if (input == '-') return DFA_MINUS;
            if (input == '*') return DFA_STAR;
            if (input == '%') return DFA_PERCENT;
            if (input == '=') return DFA_EQUAL;
            if (input == '!') return DFA_BANG;
            if (input == '<') return DFA_LESS;
            if (input == '>') return DFA_GREATER;
            if (input == '|') return DFA_VBAR;
            
            // Single character tokens
            if (input == '(') return DFA_LPAREN;
            if (input == ')') return DFA_RPAREN;
            if (input == '[') return DFA_LBRACKET;
            if (input == ']') return DFA_RBRACKET;
            if (input == ':') return DFA_COLON;
            if (input == ',') return DFA_COMMA;
            if (input == '.') return DFA_DOT;
            if (input == '^') return DFA_CARET;
            if (input == '\n') return DFA_NEWLINE;
            
            return DFA_ERROR;
        
        // 'a' prefix (and, as)
        case DFA_A:
            if (input == 'n') return DFA_AN;
            if (input == 's') return DFA_AS;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_AN: return (input == 'd') ? DFA_AND : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_AND: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        case DFA_AS: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'b' prefix (break, bool)
        case DFA_B:
            if (input == 'r') return DFA_BR;
            if (input == 'o') return DFA_BO;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_BR: return (input == 'e') ? DFA_BRE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_BRE: return (input == 'a') ? DFA_BREA : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_BREA: return (input == 'k') ? DFA_BREAK : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_BREAK: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_BO: return (input == 'o') ? DFA_BOO : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_BOO: return (input == 'l') ? DFA_BOOL : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_BOOL: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'c' prefix (continue, char)
        case DFA_C:
            if (input == 'o') return DFA_CO;
            if (input == 'h') return DFA_CH;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_CO: return (input == 'n') ? DFA_CON : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CON: return (input == 't') ? DFA_CONT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CONT: return (input == 'i') ? DFA_CONTI : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CONTI: return (input == 'n') ? DFA_CONTIN : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CONTIN: return (input == 'u') ? DFA_CONTINU : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CONTINU: return (input == 'e') ? DFA_CONTINUE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CONTINUE: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_CH: return (input == 'a') ? DFA_CHA : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CHA: return (input == 'r') ? DFA_CHAR : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_CHAR: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'e' prefix (else, each)
        case DFA_E:
            if (input == 'l') return DFA_EL;
            if (input == 'a') return DFA_EA;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_EL: return (input == 's') ? DFA_ELS : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_ELS: return (input == 'e') ? DFA_ELSE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_ELSE: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_EA: return (input == 'c') ? DFA_EAC : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_EAC: return (input == 'h') ? DFA_EACH : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_EACH: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'f' prefix (flex, fixed, for, from, function, false, float)
        case DFA_F:
            if (input == 'l') return DFA_FL;
            if (input == 'i') return DFA_FI;
            if (input == 'o') return DFA_FO;
            if (input == 'r') return DFA_FR;
            if (input == 'u') return DFA_FU;
            if (input == 'a') return DFA_FA;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FL:
            if (input == 'e') return DFA_FLE;
            if (input == 'o') return DFA_FLO;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FLE: return (input == 'x') ? DFA_FLEX : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FLEX: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FLO: return (input == 'a') ? DFA_FLOA : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FLOA: return (input == 't') ? DFA_FLOAT_KW : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FLOAT_KW: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FI: return (input == 'x') ? DFA_FIX : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FIX: return (input == 'e') ? DFA_FIXE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FIXE: return (input == 'd') ? DFA_FIXED : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FIXED: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FO: return (input == 'r') ? DFA_FOR : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FOR: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FR: return (input == 'o') ? DFA_FRO : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FRO: return (input == 'm') ? DFA_FROM : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FROM: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FU: return (input == 'n') ? DFA_FUN : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUN: return (input == 'c') ? DFA_FUNC : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUNC: return (input == 't') ? DFA_FUNCT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUNCT: return (input == 'i') ? DFA_FUNCTI : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUNCTI: return (input == 'o') ? DFA_FUNCTIO : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUNCTIO: return (input == 'n') ? DFA_FUNCTION : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FUNCTION: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_FA: return (input == 'l') ? DFA_FAL : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FAL: return (input == 's') ? DFA_FALS : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FALS: return (input == 'e') ? DFA_FALSE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_FALSE: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'i' prefix (in, import, int)
        case DFA_I:
            if (input == 'n') return DFA_IN;
            if (input == 'm') return DFA_IM;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_IN: return (input == 't') ? DFA_INT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_INT: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_IM: return (input == 'p') ? DFA_IMP : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_IMP: return (input == 'o') ? DFA_IMPO : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_IMPO: return (input == 'r') ? DFA_IMPOR : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_IMPOR: return (input == 't') ? DFA_IMPORT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_IMPORT: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'n' prefix (not)
        case DFA_N: return (input == 'o') ? DFA_NO : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_NO: return (input == 't') ? DFA_NOT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_NOT: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'o' prefix (output, of, or)
        case DFA_O:
            if (input == 'u') return DFA_OU;
            if (input == 'f') return DFA_OF;
            if (input == 'r') return DFA_OR;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_OF: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        case DFA_OR: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_OU: return (input == 't') ? DFA_OUT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_OUT: return (input == 'p') ? DFA_OUTP : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_OUTP: return (input == 'u') ? DFA_OUTPU : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_OUTPU: return (input == 't') ? DFA_OUTPUT : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_OUTPUT: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'r' prefix (return)
        case DFA_R: return (input == 'e') ? DFA_RE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_RE: return (input == 't') ? DFA_RET : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_RET: return (input == 'u') ? DFA_RETU : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_RETU: return (input == 'r') ? DFA_RETUR : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_RETUR: return (input == 'n') ? DFA_RETURN : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_RETURN: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 's' prefix (str)
        case DFA_S: return (input == 't') ? DFA_ST : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_ST: return (input == 'r') ? DFA_STR : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_STR: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 't' prefix (to, then, true)
        case DFA_T:
            if (input == 'o') return DFA_TO;
            if (input == 'h') return DFA_TH;
            if (input == 'r') return DFA_TR;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_TO: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_TH: return (input == 'e') ? DFA_THE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_THE: return (input == 'n') ? DFA_THEN : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_THEN: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_TR: return (input == 'u') ? DFA_TRU : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_TRU: return (input == 'e') ? DFA_TRUE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_TRUE: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        // 'w' prefix (when, while)
        case DFA_W: return (input == 'h') ? DFA_WH : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_WH:
            if (input == 'e') return DFA_WHE;
            if (input == 'i') return DFA_WHI;
            return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_WHE: return (input == 'n') ? DFA_WHEN : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_WHEN: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_WHI: return (input == 'l') ? DFA_WHIL : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_WHIL: return (input == 'e') ? DFA_WHILE : (isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR);
        case DFA_WHILE: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_IDENTIFIER: return isAlphaNumeric(input) ? DFA_IDENTIFIER : DFA_ERROR;
        
        case DFA_NUMBER_INT:
            if (isDigit(input)) return DFA_NUMBER_INT;
            if (input == '.') return DFA_NUMBER_DOT;
            return DFA_ERROR;
        
        case DFA_NUMBER_DOT:
            if (isDigit(input)) return DFA_NUMBER_FLOAT;
            return DFA_ERROR;
        
        case DFA_NUMBER_FLOAT:
            if (isDigit(input)) return DFA_NUMBER_FLOAT;
            return DFA_ERROR;
        
        case DFA_STRING_BODY:
            if (input == '"') return DFA_STRING_END;
            if (input == '\0' || input == '\n') return DFA_ERROR;
            return DFA_STRING_BODY;
        
        case DFA_STRING_END:
            return DFA_ERROR;
        
        case DFA_CHAR_START:
            if (input == '\\') return DFA_CHAR_ESCAPE;
            if (input == '\'' || input == '\0' || input == '\n') return DFA_ERROR;
            return DFA_CHAR_BODY;
        
        case DFA_CHAR_BODY:
            if (input == '\'') return DFA_CHAR_END;
            return DFA_ERROR;
        
        case DFA_CHAR_ESCAPE:
            if (input == '\0' || input == '\n') return DFA_ERROR;
            return DFA_CHAR_ESCAPE_DONE;
        
        case DFA_CHAR_ESCAPE_DONE:
            if (input == '\'') return DFA_CHAR_END;
            return DFA_ERROR;
        
        case DFA_CHAR_END:
            return DFA_ERROR;
        
        case DFA_COMMENT_LINE_BODY:
            if (input == '\n') return DFA_COMMENT_LINE_END;
            if (input == '\0') return DFA_COMMENT_LINE_END; // EOF ends comment
            return DFA_COMMENT_LINE_BODY;
        
        case DFA_COMMENT_LINE_END:
            return DFA_ERROR;
        
        case DFA_COMMENT_SLASH:
            if (input == '=') return DFA_SLASH_EQUAL;
            if (input == '*') return DFA_COMMENT_BLOCK;
            if (input == '/') return DFA_SLASH_SLASH;
            return DFA_ERROR;
        
        case DFA_COMMENT_BLOCK:
            if (input == '*') return DFA_COMMENT_BLOCK_STAR;
            if (input == '\0') return DFA_ERROR;
            return DFA_COMMENT_BLOCK;
        
        case DFA_COMMENT_BLOCK_STAR:
            if (input == '/') return DFA_COMMENT_BLOCK_END;
            if (input == '*') return DFA_COMMENT_BLOCK_STAR;
            if (input == '\0') return DFA_ERROR;
            return DFA_COMMENT_BLOCK;
        
        case DFA_COMMENT_BLOCK_END:
            return DFA_ERROR;
        
        case DFA_PLUS:
            if (input == '=') return DFA_PLUS_EQUAL;
            return DFA_ERROR;
        
        case DFA_MINUS:
            if (input == '=') return DFA_MINUS_EQUAL;
            return DFA_ERROR;
        
        case DFA_STAR:
            if (input == '=') return DFA_STAR_EQUAL;
            return DFA_ERROR;
        
        case DFA_PERCENT:
            if (input == '=') return DFA_PERCENT_EQUAL;
            return DFA_ERROR;
        
        case DFA_EQUAL:
            if (input == '=') return DFA_EQUAL_EQUAL;
            return DFA_ERROR;
        
        case DFA_BANG:
            if (input == '=') return DFA_BANG_EQUAL;
            return DFA_ERROR;
        
        case DFA_LESS:
            if (input == '=') return DFA_LESS_EQUAL;
            return DFA_ERROR;
        
        case DFA_GREATER:
            if (input == '=') return DFA_GREATER_EQUAL;
            return DFA_ERROR;
        
        // All accepting states return error on any input
        case DFA_PLUS_EQUAL:
        case DFA_MINUS_EQUAL:
        case DFA_STAR_EQUAL:
        case DFA_SLASH_EQUAL:
        case DFA_SLASH_SLASH:
        case DFA_PERCENT_EQUAL:
        case DFA_EQUAL_EQUAL:
        case DFA_BANG_EQUAL:
        case DFA_LESS_EQUAL:
        case DFA_GREATER_EQUAL:
        case DFA_LPAREN:
        case DFA_RPAREN:
        case DFA_LBRACKET:
        case DFA_RBRACKET:
        case DFA_COLON:
        case DFA_COMMA:
        case DFA_DOT:
        case DFA_CARET:
        case DFA_VBAR:
        case DFA_NEWLINE:
            return DFA_ERROR;
        
        default:
            return DFA_ERROR;
    }
}

static bool isAcceptingState(DFAState state) {
    switch (state) {
        // Keywords
        case DFA_AND:
        case DFA_AS:
        case DFA_BREAK:
        case DFA_BOOL:
        case DFA_CONTINUE:
        case DFA_CHAR:
        case DFA_ELSE:
        case DFA_EACH:
        case DFA_FLEX:
        case DFA_FLOAT_KW:
        case DFA_FIXED:
        case DFA_FOR:
        case DFA_FROM:
        case DFA_FUNCTION:
        case DFA_FALSE:
        case DFA_IN:
        case DFA_IMPORT:
        case DFA_INT:
        case DFA_NOT:
        case DFA_OF:
        case DFA_OR:
        case DFA_OUTPUT:
        case DFA_RETURN:
        case DFA_STR:
        case DFA_TO:
        case DFA_THEN:
        case DFA_TRUE:
        case DFA_WHEN:
        case DFA_WHILE:
        case DFA_IDENTIFIER:
        
        // Single letter identifiers (keyword prefixes)
        case DFA_A:
        case DFA_B:
        case DFA_C:
        case DFA_E:
        case DFA_F:
        case DFA_I:
        case DFA_N:
        case DFA_O:
        case DFA_R:
        case DFA_S:
        case DFA_T:
        case DFA_W:
        
        // Numbers
        case DFA_NUMBER_INT:
        case DFA_NUMBER_FLOAT:
        
        // Strings and chars
        case DFA_STRING_END:
        case DFA_CHAR_END:
        
        // Comments
        case DFA_COMMENT_LINE_END:
        case DFA_COMMENT_BLOCK_END:
        
        // Operators
        case DFA_PLUS:
        case DFA_PLUS_EQUAL:
        case DFA_MINUS:
        case DFA_MINUS_EQUAL:
        case DFA_STAR:
        case DFA_STAR_EQUAL:
        case DFA_COMMENT_SLASH:
        case DFA_SLASH_EQUAL:
        case DFA_SLASH_SLASH:
        case DFA_PERCENT:
        case DFA_PERCENT_EQUAL:
        case DFA_EQUAL:
        case DFA_EQUAL_EQUAL:
        case DFA_BANG_EQUAL:
        case DFA_LESS:
        case DFA_LESS_EQUAL:
        case DFA_GREATER:
        case DFA_GREATER_EQUAL:
        
        // Single character tokens
        case DFA_LPAREN:
        case DFA_RPAREN:
        case DFA_LBRACKET:
        case DFA_RBRACKET:
        case DFA_COLON:
        case DFA_COMMA:
        case DFA_DOT:
        case DFA_CARET:
        case DFA_VBAR:
        case DFA_NEWLINE:
            return true;
        
        default:
            return false;
    }
}

static TokenType getTokenTypeFromDFAState(DFAState state) {
    switch (state) {
        // Keywords
        case DFA_AND: return TOKEN_AND;
        case DFA_AS: return TOKEN_AS;
        case DFA_BREAK: return TOKEN_BREAK;
        case DFA_BOOL: return TOKEN_HINT_BOOL;
        case DFA_CONTINUE: return TOKEN_CONTINUE;
        case DFA_CHAR: return TOKEN_HINT_CHAR;
        case DFA_ELSE: return TOKEN_ELSE;
        case DFA_EACH: return TOKEN_EACH;
        case DFA_FLEX: return TOKEN_FLEX;
        case DFA_FLOAT_KW: return TOKEN_HINT_FLOAT;
        case DFA_FIXED: return TOKEN_FIXED;
        case DFA_FOR: return TOKEN_FOR;
        case DFA_FROM: return TOKEN_FROM;
        case DFA_FUNCTION: return TOKEN_FUNCTION;
        case DFA_FALSE: return TOKEN_FALSE;
        case DFA_IN: return TOKEN_IN;
        case DFA_IMPORT: return TOKEN_IMPORT;
        case DFA_INT: return TOKEN_HINT_INT;
        case DFA_NOT: return TOKEN_NOT;
        case DFA_OF: return TOKEN_OF;
        case DFA_OR: return TOKEN_OR;
        case DFA_OUTPUT: return TOKEN_OUTPUT;
        case DFA_RETURN: return TOKEN_RETURN;
        case DFA_STR: return TOKEN_HINT_STR;
        case DFA_TO: return TOKEN_TO;
        case DFA_THEN: return TOKEN_THEN;
        case DFA_TRUE: return TOKEN_TRUE;
        case DFA_WHEN: return TOKEN_WHEN;
        case DFA_WHILE: return TOKEN_WHILE;
        case DFA_IDENTIFIER: return TOKEN_IDENTIFIER;
        
        // Single letter identifiers (all are identifiers, not keywords)
        case DFA_A:
        case DFA_B:
        case DFA_C:
        case DFA_E:
        case DFA_F:
        case DFA_I:
        case DFA_N:
        case DFA_O:
        case DFA_R:
        case DFA_S:
        case DFA_T:
        case DFA_W:
            return TOKEN_IDENTIFIER;
        
        // Numbers
        case DFA_NUMBER_INT: return TOKEN_INTEGER;
        case DFA_NUMBER_FLOAT: return TOKEN_FLOAT;
        
        // Strings and chars
        case DFA_STRING_END: return TOKEN_STRING;
        case DFA_CHAR_END: return TOKEN_CHAR;
        
        // Comments
        case DFA_COMMENT_LINE_END: return TOKEN_COMMENT_LINE;
        case DFA_COMMENT_BLOCK_END: return TOKEN_COMMENT_BLOCK;
        
        // Operators
        case DFA_PLUS: return TOKEN_PLUS;
        case DFA_PLUS_EQUAL: return TOKEN_PLUS_EQUAL;
        case DFA_MINUS: return TOKEN_MINUS;
        case DFA_MINUS_EQUAL: return TOKEN_MINUS_EQUAL;
        case DFA_STAR: return TOKEN_STAR;
        case DFA_STAR_EQUAL: return TOKEN_STAR_EQUAL;
        case DFA_COMMENT_SLASH: return TOKEN_SLASH;
        case DFA_SLASH_EQUAL: return TOKEN_SLASH_EQUAL;
        case DFA_SLASH_SLASH: return TOKEN_FLOOR_DIV;
        case DFA_PERCENT: return TOKEN_PERCENT;
        case DFA_PERCENT_EQUAL: return TOKEN_PERCENT_EQUAL;
        case DFA_EQUAL: return TOKEN_EQUAL;
        case DFA_EQUAL_EQUAL: return TOKEN_EQUAL_EQUAL;
        case DFA_BANG_EQUAL: return TOKEN_BANG_EQUAL;
        case DFA_LESS: return TOKEN_LESS;
        case DFA_LESS_EQUAL: return TOKEN_LESS_EQUAL;
        case DFA_GREATER: return TOKEN_GREATER;
        case DFA_GREATER_EQUAL: return TOKEN_GREATER_EQUAL;
        
        // Single character tokens
        case DFA_LPAREN: return TOKEN_LPAREN;
        case DFA_RPAREN: return TOKEN_RPAREN;
        case DFA_LBRACKET: return TOKEN_LBRACKET;
        case DFA_RBRACKET: return TOKEN_RBRACKET;
        case DFA_COLON: return TOKEN_COLON;
        case DFA_COMMA: return TOKEN_COMMA;
        case DFA_DOT: return TOKEN_DOT;
        case DFA_CARET: return TOKEN_CARET;
        case DFA_VBAR: return TOKEN_VBAR;
        case DFA_NEWLINE: return TOKEN_NEWLINE;
        
        default: return TOKEN_ERROR;
    }
}

static Token scanTokenDFA(Lexer* lexer) {
    DFAState state = DFA_START;
    DFAState lastAcceptingState = DFA_ERROR;
    const char* lastAcceptingPos = lexer->start;
    
    if (isAtEnd(lexer)) {
        return makeToken(lexer, TOKEN_EOF);
    }
    
    // Run the DFA
    while (!isAtEnd(lexer)) {
        char c = peek(lexer);
        DFAState nextState = dfaTransition(state, c);
        
        if (nextState == DFA_ERROR) {
            // No valid transition - check if we have an accepting state
            if (lastAcceptingState != DFA_ERROR) {
                lexer->current = lastAcceptingPos;
                TokenType type = getTokenTypeFromDFAState(lastAcceptingState);
                
                // Update line count for newlines in the token
                const char* p = lexer->start;
                while (p < lexer->current) {
                    if (*p == '\n') lexer->line++;
                    p++;
                }
                
                return makeToken(lexer, type);
            }

            if (state == DFA_START) {
                advance(lexer);
                return errorToken(lexer, "Unexpected character.");
            }
            
            if (state == DFA_STRING_BODY) {
                return errorToken(lexer, "Unterminated string literal.");
            }
            if (state >= DFA_CHAR_START && state <= DFA_CHAR_ESCAPE_DONE) {
                return errorToken(lexer, "Unterminated char literal.");
            }
            if (state == DFA_COMMENT_BLOCK || state == DFA_COMMENT_BLOCK_STAR) {
                return errorToken(lexer, "Unterminated block comment.");
            }
            if (state == DFA_BANG) {
                return errorToken(lexer, "Expected '=' after '!'.");
            }
            if (state == DFA_NUMBER_DOT) {
                return errorToken(lexer, "Expected digit after decimal point.");
            }
            
            return errorToken(lexer, "Invalid token.");
        }
        
        // Valid transition - advance
        advance(lexer);
        state = nextState;
        
        // Update last accepting state if this is an accepting state
        if (isAcceptingState(state)) {
            lastAcceptingState = state;
            lastAcceptingPos = lexer->current;
        }
    }
    
    // Reached EOF - check if we're in an accepting state
    if (isAcceptingState(state)) {
        TokenType type = getTokenTypeFromDFAState(state);
        
        const char* p = lexer->start;
        while (p < lexer->current) {
            if (*p == '\n') lexer->line++;
            p++;
        }
        
        return makeToken(lexer, type);
    }
    
    if (lastAcceptingState != DFA_ERROR) {
        lexer->current = lastAcceptingPos;
        TokenType type = getTokenTypeFromDFAState(lastAcceptingState);
        
        const char* p = lexer->start;
        while (p < lexer->current) {
            if (*p == '\n') lexer->line++;
            p++;
        }
        
        return makeToken(lexer, type);
    }
    
    // EOF in non-accepting state
    if (state == DFA_STRING_BODY) {
        return errorToken(lexer, "Unterminated string literal.");
    }
    if (state >= DFA_CHAR_START && state <= DFA_CHAR_ESCAPE_DONE) {
        return errorToken(lexer, "Unterminated char literal.");
    }
    if (state == DFA_COMMENT_BLOCK || state == DFA_COMMENT_BLOCK_STAR) {
        return errorToken(lexer, "Unterminated block comment.");
    }
    
    return errorToken(lexer, "Unexpected end of input.");
}

static void skipWhitespace(Lexer* lexer) {
    while (!isAtEnd(lexer)) {
        char c = peek(lexer);
        if (c == ' ' || c == '\r' || c == '\t') {
            advance(lexer);
        } else {
            break;
        }
    }
}

static Token scanToken(Lexer* lexer) {
    skipWhitespace(lexer);

    lexer->start = lexer->current;
    
    if (isAtEnd(lexer)) {
        return makeToken(lexer, TOKEN_EOF);
    }
    
    return scanTokenDFA(lexer);
}

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