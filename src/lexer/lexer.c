#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

typedef enum {
    Q_ERROR = -1,
    Q_START = 0,
    
    // Keywords
    Q_A = 1, Q_AN = 2, Q_AND = 3, Q_AS = 4,
    Q_B = 5, Q_BR = 6, Q_BRE = 7, Q_BREA = 8, Q_BREAK = 9,
    Q_BO = 10, Q_BOO = 11, Q_BOOL = 12,
    Q_C = 13, Q_CO = 14, Q_CON = 15, Q_CONT = 16, Q_CONTI = 17,
    Q_CONTIN = 18, Q_CONTINU = 19, Q_CONTINUE = 20,
    Q_CH = 21, Q_CHA = 22, Q_CHAR = 23,
    Q_E = 24, Q_EL = 25, Q_ELS = 26, Q_ELSE = 27,
    Q_EA = 28, Q_EAC = 29, Q_EACH = 30,
    Q_F = 31, Q_FL = 32, Q_FLE = 33, Q_FLEX = 34,
    Q_FLO = 35, Q_FLOA = 36, Q_FLOAT_KW = 37,
    Q_FI = 38, Q_FIX = 39, Q_FIXE = 40, Q_FIXED = 41,
    Q_FO = 42, Q_FOR = 43, Q_FR = 44, Q_FRO = 45, Q_FROM = 46,
    Q_FU = 47, Q_FUN = 48, Q_FUNC = 49, Q_FUNCT = 50,
    Q_FUNCTI = 51, Q_FUNCTIO = 52, Q_FUNCTION = 53,
    Q_FA = 54, Q_FAL = 55, Q_FALS = 56, Q_FALSE = 57,
    Q_I = 58, Q_IN = 59, Q_INP = 149, Q_INPU = 150, Q_INPUT = 151,
    Q_IM = 60, Q_IMP = 61, Q_IMPO = 62, Q_IMPOR = 63, Q_IMPORT = 64, 
    Q_INT = 65,
    Q_N = 66, Q_NO = 67, Q_NOT = 68,
    Q_O = 69, Q_OF = 70, Q_OU = 71, Q_OUT = 72,
    Q_OUTP = 73, Q_OUTPU = 74, Q_OUTPUT = 75, Q_OR = 76,
    Q_R = 77, Q_RE = 78, Q_RET = 79, Q_RETU = 80,
    Q_RETUR = 81, Q_RETURN = 82,
    Q_S = 83, Q_ST = 84, Q_STR = 85,
    Q_T = 86, Q_TO = 87, Q_TH = 88, Q_THE = 89, Q_THEN = 90,
    Q_TR = 91, Q_TRU = 92, Q_TRUE = 93,
    Q_W = 94, Q_WH = 95, Q_WHE = 96, Q_WHEN = 97,
    Q_WHI = 98, Q_WHIL = 99, Q_WHILE = 100,
    
    Q_IDENTIFIER = 101,
    Q_NUMBER_INT = 102, Q_NUMBER_DOT = 103, Q_NUMBER_FLOAT = 104,
    Q_STRING_BODY = 106, Q_STRING_END = 107,
    Q_CHAR_START = 108, Q_CHAR_BODY = 109, Q_CHAR_ESCAPE = 110,
    Q_CHAR_ESCAPE_DONE = 111, Q_CHAR_END = 112,
    Q_COMMENT_LINE_BODY = 114, Q_COMMENT_LINE_END = 115,
    Q_COMMENT_SLASH = 116, Q_COMMENT_BLOCK = 117,
    Q_COMMENT_BLOCK_STAR = 118, Q_COMMENT_BLOCK_END = 119,
    Q_PLUS = 120, Q_PLUS_EQUAL = 121, Q_MINUS = 122, Q_MINUS_EQUAL = 123,
    Q_STAR = 124, Q_STAR_EQUAL = 125, Q_SLASH_EQUAL = 127,
    Q_SLASH_SLASH = 128, Q_PERCENT = 129, Q_PERCENT_EQUAL = 130,
    Q_EQUAL = 131, Q_EQUAL_EQUAL = 132, Q_BANG = 133, Q_BANG_EQUAL = 134,
    Q_LESS = 135, Q_LESS_EQUAL = 136, Q_GREATER = 137, Q_GREATER_EQUAL = 138,
    Q_VBAR = 139, Q_LPAREN = 140, Q_RPAREN = 141, Q_LBRACKET = 142,
    Q_RBRACKET = 143, Q_COLON = 144, Q_COMMA = 145, Q_DOT = 146,
    Q_CARET = 147, Q_NEWLINE = 148
} State;

struct Lexer {
    const char* source;
    const char* start;
    const char* current;
    int line;

    int* indentStack;      
    int indentCapacity;    
    int indentCount;       
    int pendingDedents;    
    bool atLineStart;      
    int currentIndent;     
};

#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALNUM(c) (IS_ALPHA(c) || IS_DIGIT(c))

static State transition(State state, char c) {
    switch (state) {
        case Q_START:
            if (c == 'a') return Q_A;
            if (c == 'b') return Q_B;
            if (c == 'c') return Q_C;
            if (c == 'e') return Q_E;
            if (c == 'f') return Q_F;
            if (c == 'i') return Q_I;
            if (c == 'n') return Q_N;
            if (c == 'o') return Q_O;
            if (c == 'r') return Q_R;
            if (c == 's') return Q_S;
            if (c == 't') return Q_T;
            if (c == 'w') return Q_W;
            if (IS_ALPHA(c)) return Q_IDENTIFIER;
            if (IS_DIGIT(c)) return Q_NUMBER_INT;
            if (c == '"') return Q_STRING_BODY;
            if (c == '\'') return Q_CHAR_START;
            if (c == '#') return Q_COMMENT_LINE_BODY;
            if (c == '/') return Q_COMMENT_SLASH;
            if (c == '+') return Q_PLUS;
            if (c == '-') return Q_MINUS;
            if (c == '*') return Q_STAR;
            if (c == '%') return Q_PERCENT;
            if (c == '=') return Q_EQUAL;
            if (c == '!') return Q_BANG;
            if (c == '<') return Q_LESS;
            if (c == '>') return Q_GREATER;
            if (c == '|') return Q_VBAR;
            if (c == '(') return Q_LPAREN;
            if (c == ')') return Q_RPAREN;
            if (c == '[') return Q_LBRACKET;
            if (c == ']') return Q_RBRACKET;
            if (c == ':') return Q_COLON;
            if (c == ',') return Q_COMMA;
            if (c == '.') return Q_DOT;
            if (c == '^') return Q_CARET;
            if (c == '\n') return Q_NEWLINE;
            return Q_ERROR;
        
        case Q_A:
            if (c == 'n') return Q_AN;
            if (c == 's') return Q_AS;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_AN:
            if (c == 'd') return Q_AND;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_AND:
        case Q_AS:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_B:
            if (c == 'r') return Q_BR;
            if (c == 'o') return Q_BO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BR:
            if (c == 'e') return Q_BRE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BRE:
            if (c == 'a') return Q_BREA;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BREA:
            if (c == 'k') return Q_BREAK;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BREAK:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BO:
            if (c == 'o') return Q_BOO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BOO:
            if (c == 'l') return Q_BOOL;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_BOOL:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_C:
            if (c == 'o') return Q_CO;
            if (c == 'h') return Q_CH;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CO:
            if (c == 'n') return Q_CON;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CON:
            if (c == 't') return Q_CONT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CONT:
            if (c == 'i') return Q_CONTI;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CONTI:
            if (c == 'n') return Q_CONTIN;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CONTIN:
            if (c == 'u') return Q_CONTINU;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CONTINU:
            if (c == 'e') return Q_CONTINUE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CONTINUE:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CH:
            if (c == 'a') return Q_CHA;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CHA:
            if (c == 'r') return Q_CHAR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_CHAR:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_E:
            if (c == 'l') return Q_EL;
            if (c == 'a') return Q_EA;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_EL:
            if (c == 's') return Q_ELS;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_ELS:
            if (c == 'e') return Q_ELSE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_ELSE:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_EA:
            if (c == 'c') return Q_EAC;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_EAC:
            if (c == 'h') return Q_EACH;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_EACH:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_F:
            if (c == 'l') return Q_FL;
            if (c == 'i') return Q_FI;
            if (c == 'o') return Q_FO;
            if (c == 'r') return Q_FR;
            if (c == 'u') return Q_FU;
            if (c == 'a') return Q_FA;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FL:
            if (c == 'e') return Q_FLE;
            if (c == 'o') return Q_FLO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FLE:
            if (c == 'x') return Q_FLEX;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FLEX:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FLO:
            if (c == 'a') return Q_FLOA;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FLOA:
            if (c == 't') return Q_FLOAT_KW;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FLOAT_KW:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FI:
            if (c == 'x') return Q_FIX;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FIX:
            if (c == 'e') return Q_FIXE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FIXE:
            if (c == 'd') return Q_FIXED;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FIXED:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FO:
            if (c == 'r') return Q_FOR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FOR:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FR:
            if (c == 'o') return Q_FRO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FRO:
            if (c == 'm') return Q_FROM;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FROM:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FU:
            if (c == 'n') return Q_FUN;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUN:
            if (c == 'c') return Q_FUNC;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUNC:
            if (c == 't') return Q_FUNCT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUNCT:
            if (c == 'i') return Q_FUNCTI;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUNCTI:
            if (c == 'o') return Q_FUNCTIO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUNCTIO:
            if (c == 'n') return Q_FUNCTION;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FUNCTION:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FA:
            if (c == 'l') return Q_FAL;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FAL:
            if (c == 's') return Q_FALS;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FALS:
            if (c == 'e') return Q_FALSE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_FALSE:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_I:
            if (c == 'n') return Q_IN;
            if (c == 'm') return Q_IM;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IN:
            if (c == 't') return Q_INT;
            if (c == 'p') return Q_INP;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_INP:
            if (c == 'u') return Q_INPU;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_INPU:
            if (c == 't') return Q_INPUT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;

        case Q_INPUT:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_INT:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IM:
            if (c == 'p') return Q_IMP;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IMP:
            if (c == 'o') return Q_IMPO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IMPO:
            if (c == 'r') return Q_IMPOR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IMPOR:
            if (c == 't') return Q_IMPORT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_IMPORT:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_N:
            if (c == 'o') return Q_NO;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_NO:
            if (c == 't') return Q_NOT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_NOT:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_O:
            if (c == 'u') return Q_OU;
            if (c == 'f') return Q_OF;
            if (c == 'r') return Q_OR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OF:
        case Q_OR:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OU:
            if (c == 't') return Q_OUT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OUT:
            if (c == 'p') return Q_OUTP;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OUTP:
            if (c == 'u') return Q_OUTPU;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OUTPU:
            if (c == 't') return Q_OUTPUT;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_OUTPUT:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_R:
            if (c == 'e') return Q_RE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_RE:
            if (c == 't') return Q_RET;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_RET:
            if (c == 'u') return Q_RETU;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_RETU:
            if (c == 'r') return Q_RETUR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_RETUR:
            if (c == 'n') return Q_RETURN;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_RETURN:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_S:
            if (c == 't') return Q_ST;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_ST:
            if (c == 'r') return Q_STR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_STR:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_T:
            if (c == 'o') return Q_TO;
            if (c == 'h') return Q_TH;
            if (c == 'r') return Q_TR;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_TO:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_TH:
            if (c == 'e') return Q_THE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_THE:
            if (c == 'n') return Q_THEN;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_THEN:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_TR:
            if (c == 'u') return Q_TRU;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_TRU:
            if (c == 'e') return Q_TRUE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_TRUE:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_W:
            if (c == 'h') return Q_WH;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WH:
            if (c == 'e') return Q_WHE;
            if (c == 'i') return Q_WHI;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WHE:
            if (c == 'n') return Q_WHEN;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WHEN:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WHI:
            if (c == 'l') return Q_WHIL;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WHIL:
            if (c == 'e') return Q_WHILE;
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
            
        case Q_WHILE:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_IDENTIFIER:
            if (IS_ALNUM(c)) return Q_IDENTIFIER;
            return Q_ERROR;
        
        case Q_NUMBER_INT:
            if (IS_DIGIT(c)) return Q_NUMBER_INT;
            if (c == '.') return Q_NUMBER_DOT;
            return Q_ERROR;
            
        case Q_NUMBER_DOT:
            if (IS_DIGIT(c)) return Q_NUMBER_FLOAT;
            return Q_ERROR;
            
        case Q_NUMBER_FLOAT:
            if (IS_DIGIT(c)) return Q_NUMBER_FLOAT;
            return Q_ERROR;
        
        case Q_STRING_BODY:
            if (c == '"') return Q_STRING_END;
            if (c == '\0' || c == '\n') return Q_ERROR;
            return Q_STRING_BODY;
            
        case Q_STRING_END:
            return Q_ERROR;
        
        case Q_CHAR_START:
            if (c == '\\') return Q_CHAR_ESCAPE;
            if (c == '\'' || c == '\0' || c == '\n') return Q_ERROR;
            return Q_CHAR_BODY;
            
        case Q_CHAR_BODY:
            if (c == '\'') return Q_CHAR_END;
            return Q_ERROR;
            
        case Q_CHAR_ESCAPE:
            if (c == '\0' || c == '\n') return Q_ERROR;
            return Q_CHAR_ESCAPE_DONE;
            
        case Q_CHAR_ESCAPE_DONE:
            if (c == '\'') return Q_CHAR_END;
            return Q_ERROR;
            
        case Q_CHAR_END:
            return Q_ERROR;
        
        case Q_COMMENT_LINE_BODY:
            if (c == '\n' || c == '\0') return Q_COMMENT_LINE_END;
            return Q_COMMENT_LINE_BODY;
            
        case Q_COMMENT_LINE_END:
            return Q_ERROR;
        
        case Q_COMMENT_SLASH:
            if (c == '=') return Q_SLASH_EQUAL;
            if (c == '*') return Q_COMMENT_BLOCK;
            if (c == '/') return Q_SLASH_SLASH;
            return Q_ERROR;
            
        case Q_COMMENT_BLOCK:
            if (c == '*') return Q_COMMENT_BLOCK_STAR;
            if (c == '\0') return Q_ERROR;
            return Q_COMMENT_BLOCK;
            
        case Q_COMMENT_BLOCK_STAR:
            if (c == '/') return Q_COMMENT_BLOCK_END;
            if (c == '*') return Q_COMMENT_BLOCK_STAR;
            if (c == '\0') return Q_ERROR;
            return Q_COMMENT_BLOCK;
            
        case Q_COMMENT_BLOCK_END:
            return Q_ERROR;
        
        case Q_PLUS:
            if (c == '=') return Q_PLUS_EQUAL;
            return Q_ERROR;
            
        case Q_MINUS:
            if (c == '=') return Q_MINUS_EQUAL;
            return Q_ERROR;
            
        case Q_STAR:
            if (c == '=') return Q_STAR_EQUAL;
            return Q_ERROR;
            
        case Q_PERCENT:
            if (c == '=') return Q_PERCENT_EQUAL;
            return Q_ERROR;
            
        case Q_EQUAL:
            if (c == '=') return Q_EQUAL_EQUAL;
            return Q_ERROR;
            
        case Q_BANG:
            if (c == '=') return Q_BANG_EQUAL;
            return Q_ERROR;
            
        case Q_LESS:
            if (c == '=') return Q_LESS_EQUAL;
            return Q_ERROR;
            
        case Q_GREATER:
            if (c == '=') return Q_GREATER_EQUAL;
            return Q_ERROR;
        
        default:
            return Q_ERROR;
    }
}

static bool isAcceptingState(State state) {
    return (state >= Q_A && state <= Q_WHILE) ||
           state == Q_IDENTIFIER ||
           state == Q_INPUT ||
           state == Q_NUMBER_INT || state == Q_NUMBER_FLOAT ||
           state == Q_STRING_END || state == Q_CHAR_END ||
           state == Q_COMMENT_LINE_END || state == Q_COMMENT_BLOCK_END ||
           state == Q_COMMENT_SLASH ||
           (state >= Q_PLUS && state <= Q_NEWLINE && state != Q_BANG && 
            state != Q_NUMBER_DOT);
}

static TokenType getTokenType(State state) {
    switch (state) {
        case Q_AND: return TOKEN_AND;
        case Q_AS: return TOKEN_AS;
        case Q_BREAK: return TOKEN_BREAK;
        case Q_BOOL: return TOKEN_HINT_BOOL;
        case Q_CONTINUE: return TOKEN_CONTINUE;
        case Q_CHAR: return TOKEN_HINT_CHAR;
        case Q_ELSE: return TOKEN_ELSE;
        case Q_EACH: return TOKEN_EACH;
        case Q_FLEX: return TOKEN_FLEX;
        case Q_FLOAT_KW: return TOKEN_HINT_FLOAT;
        case Q_FIXED: return TOKEN_FIXED;
        case Q_FOR: return TOKEN_FOR;
        case Q_FROM: return TOKEN_FROM;
        case Q_FUNCTION: return TOKEN_FUNCTION;
        case Q_FALSE: return TOKEN_FALSE;
        case Q_IN: return TOKEN_IN;
        case Q_IMPORT: return TOKEN_IMPORT;
        case Q_INT: return TOKEN_HINT_INT;
        case Q_INPUT: return TOKEN_INPUT;
        case Q_NOT: return TOKEN_NOT;
        case Q_OF: return TOKEN_OF;
        case Q_OR: return TOKEN_OR;
        case Q_OUTPUT: return TOKEN_OUTPUT;
        case Q_RETURN: return TOKEN_RETURN;
        case Q_STR: return TOKEN_HINT_STR;
        case Q_TO: return TOKEN_TO;
        case Q_THEN: return TOKEN_THEN;
        case Q_TRUE: return TOKEN_TRUE;
        case Q_WHEN: return TOKEN_WHEN;
        case Q_WHILE: return TOKEN_WHILE;
        case Q_A: case Q_B: case Q_C: case Q_E: case Q_F:
        case Q_I: case Q_N: case Q_O: case Q_R: case Q_S:
        case Q_T: case Q_W: case Q_IDENTIFIER: return TOKEN_IDENTIFIER;
        case Q_NUMBER_INT: return TOKEN_INTEGER;
        case Q_NUMBER_FLOAT: return TOKEN_FLOAT;
        case Q_STRING_END: return TOKEN_STRING;
        case Q_CHAR_END: return TOKEN_CHAR;
        case Q_COMMENT_LINE_END: return TOKEN_COMMENT_LINE;
        case Q_COMMENT_BLOCK_END: return TOKEN_COMMENT_BLOCK;
        case Q_PLUS: return TOKEN_PLUS;
        case Q_PLUS_EQUAL: return TOKEN_PLUS_EQUAL;
        case Q_MINUS: return TOKEN_MINUS;
        case Q_MINUS_EQUAL: return TOKEN_MINUS_EQUAL;
        case Q_STAR: return TOKEN_STAR;
        case Q_STAR_EQUAL: return TOKEN_STAR_EQUAL;
        case Q_COMMENT_SLASH: return TOKEN_SLASH;
        case Q_SLASH_EQUAL: return TOKEN_SLASH_EQUAL;
        case Q_SLASH_SLASH: return TOKEN_FLOOR_DIV;
        case Q_PERCENT: return TOKEN_PERCENT;
        case Q_PERCENT_EQUAL: return TOKEN_PERCENT_EQUAL;
        case Q_EQUAL: return TOKEN_EQUAL;
        case Q_EQUAL_EQUAL: return TOKEN_EQUAL_EQUAL;
        case Q_BANG_EQUAL: return TOKEN_BANG_EQUAL;
        case Q_LESS: return TOKEN_LESS;
        case Q_LESS_EQUAL: return TOKEN_LESS_EQUAL;
        case Q_GREATER: return TOKEN_GREATER;
        case Q_GREATER_EQUAL: return TOKEN_GREATER_EQUAL;
        case Q_LPAREN: return TOKEN_LPAREN;
        case Q_RPAREN: return TOKEN_RPAREN;
        case Q_LBRACKET: return TOKEN_LBRACKET;
        case Q_RBRACKET: return TOKEN_RBRACKET;
        case Q_COLON: return TOKEN_COLON;
        case Q_COMMA: return TOKEN_COMMA;
        case Q_DOT: return TOKEN_DOT;
        case Q_CARET: return TOKEN_CARET;
        case Q_VBAR: return TOKEN_VBAR;
        case Q_NEWLINE: return TOKEN_NEWLINE;
        default: return TOKEN_ERROR;
    }
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
static void pushIndent(Lexer* lexer, int level) {
    if (lexer->indentCount >= lexer->indentCapacity) {
        lexer->indentCapacity *= 2;
        int* newStack = (int*)realloc(lexer->indentStack, 
                                       lexer->indentCapacity * sizeof(int));
        if (newStack == NULL) {
            return;
        }
        lexer->indentStack = newStack;
    }
    lexer->indentStack[lexer->indentCount++] = level;
}

static int popIndent(Lexer* lexer) {
    if (lexer->indentCount > 1) {
        return lexer->indentStack[--lexer->indentCount];
    }
    return 0;
}

static int peekIndent(Lexer* lexer) {
    if (lexer->indentCount > 0) {
        return lexer->indentStack[lexer->indentCount - 1];
    }
    return 0;
}

static int countIndentation(Lexer* lexer) {
    int indent = 0;
    const char* lineStart = lexer->current;
    
    // Find the actual start of the line
    while (lineStart > lexer->source && *(lineStart - 1) != '\n') {
        lineStart--;
    }
    
    const char* p = lineStart;
    
    while (*p == ' ' || *p == '\t') {
        if (*p == ' ') {
            indent++;
        } else if (*p == '\t') {
            indent += 4; // Treat tab as 4 spaces
        }
        p++;
    }
    
    return indent;
}
static Token scanToken(Lexer* lexer) {
    // Skip whitespace
    while (*lexer->current == ' ' || *lexer->current == '\r' || *lexer->current == '\t') {
        lexer->current++;
    }
    
    lexer->start = lexer->current;
    
    if (*lexer->current == '\0') {
        return makeToken(lexer, TOKEN_EOF);
    }
    
    State state = Q_START;
    State lastAccept = Q_ERROR;
    const char* lastAcceptPos = lexer->start;
    
    while (*lexer->current != '\0') {
        char c = *lexer->current;
        State next = transition(state, c);
        
        if (next == Q_ERROR) {
            if (lastAccept != Q_ERROR) {
                lexer->current = lastAcceptPos;
                TokenType type = getTokenType(lastAccept);
                
                for (const char* p = lexer->start; p < lexer->current; p++) {
                    if (*p == '\n') lexer->line++;
                }
                
                return makeToken(lexer, type);
            }
            
            if (state == Q_START) {
                lexer->current++;
                return errorToken(lexer, "Unexpected character.");
            }
            if (state == Q_STRING_BODY) return errorToken(lexer, "Unterminated string literal.");
            if (state >= Q_CHAR_START && state <= Q_CHAR_ESCAPE_DONE) 
                return errorToken(lexer, "Unterminated char literal.");
            if (state == Q_COMMENT_BLOCK || state == Q_COMMENT_BLOCK_STAR)
                return errorToken(lexer, "Unterminated block comment.");
            if (state == Q_BANG) return errorToken(lexer, "Expected '=' after '!'.");
            if (state == Q_NUMBER_DOT) return errorToken(lexer, "Expected digit after decimal point.");
            
            return errorToken(lexer, "Invalid token.");
        }
        
        lexer->current++;
        state = next;
        
        if (isAcceptingState(state)) {
            lastAccept = state;
            lastAcceptPos = lexer->current;
        }
    }
    
    if (isAcceptingState(state)) {
        TokenType type = getTokenType(state);
        for (const char* p = lexer->start; p < lexer->current; p++) {
            if (*p == '\n') lexer->line++;
        }
        return makeToken(lexer, type);
    }
    
    if (lastAccept != Q_ERROR) {
        lexer->current = lastAcceptPos;
        TokenType type = getTokenType(lastAccept);
        for (const char* p = lexer->start; p < lexer->current; p++) {
            if (*p == '\n') lexer->line++;
        }
        return makeToken(lexer, type);
    }
    
    if (state == Q_STRING_BODY) return errorToken(lexer, "Unterminated string literal.");
    if (state >= Q_CHAR_START && state <= Q_CHAR_ESCAPE_DONE)
        return errorToken(lexer, "Unterminated char literal.");
    if (state == Q_COMMENT_BLOCK || state == Q_COMMENT_BLOCK_STAR)
        return errorToken(lexer, "Unterminated block comment.");
    
    return errorToken(lexer, "Unexpected end of input.");
}

Lexer* initLexer(const char* source) {
    if (source == NULL) return NULL;
    
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) return NULL;
    
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    
    // Initialize indentation tracking
    lexer->indentCapacity = 16;
    lexer->indentStack = (int*)malloc(lexer->indentCapacity * sizeof(int));
    if (lexer->indentStack == NULL) {
        free(lexer);
        return NULL;
    }
    lexer->indentCount = 1;
    lexer->indentStack[0] = 0; // Base indentation level
    lexer->pendingDedents = 0;
    lexer->atLineStart = true;
    
    return lexer;
}

/* Single getNextToken implementation (indentation-aware) */
Token getNextToken(Lexer* lexer) {
    if (lexer == NULL) {
        Token errorTok;
        errorTok.type = TOKEN_ERROR;
        errorTok.lexeme = "Lexer is NULL";
        errorTok.length = 13;
        errorTok.line = 0;
        return errorTok;
    }
    
    // Emit pending dedent tokens first
    if (lexer->pendingDedents > 0) {
        lexer->pendingDedents--;
        lexer->start = lexer->current;
        return makeToken(lexer, TOKEN_DEDENT);
    }
    
    // Handle indentation at the start of a line
    if (lexer->atLineStart) {
        lexer->atLineStart = false;
        
        // Skip empty lines and lines with only whitespace/comments
        while (true) {
            // Skip whitespace at start of line
            while (*lexer->current == ' ' || *lexer->current == '\t' || *lexer->current == '\r') {
                lexer->current++;
            }
            
            // Check for comment
            if (*lexer->current == '#') {
                // Skip comment line
                while (*lexer->current != '\n' && *lexer->current != '\0') {
                    lexer->current++;
                }
            }
            
            // Check for newline
            if (*lexer->current == '\n') {
                lexer->line++;
                lexer->current++;
                lexer->start = lexer->current;
                continue;  // Continue to next line
            }
            
            // Non-empty line found
            break;
        }
        
        // Check for EOF
        if (*lexer->current == '\0') {
            // Emit all remaining dedents before EOF
            if (lexer->indentCount > 1) {
                lexer->pendingDedents = lexer->indentCount - 1;
                lexer->indentCount = 1;  // Reset to base level
                return getNextToken(lexer);
            }
            return makeToken(lexer, TOKEN_EOF);
        }
        
        // Count indentation at the start of this non-empty line
        lexer->start = lexer->current;
        int indent = 0;
        const char* lineStart = lexer->current;
        
        // Go back to start of line to count indentation
        while (lineStart > lexer->source && *(lineStart - 1) != '\n') {
            lineStart--;
        }
        
        // Count spaces/tabs from line start
        const char* p = lineStart;
        while (*p == ' ' || *p == '\t') {
            if (*p == ' ') {
                indent++;
            } else if (*p == '\t') {
                indent += 4; // Treat tab as 4 spaces
            }
            p++;
        }
        
        int currentLevel = peekIndent(lexer);
        
        if (indent > currentLevel) {
            // Increased indentation - emit INDENT
            pushIndent(lexer, indent);
            lexer->start = lineStart;
            return makeToken(lexer, TOKEN_INDENT);
        } else if (indent < currentLevel) {
            // Decreased indentation - emit DEDENT(s)
            lexer->pendingDedents = 0;
            
            while (lexer->indentCount > 1 && peekIndent(lexer) > indent) {
                popIndent(lexer);
                lexer->pendingDedents++;
            }
            
            if (peekIndent(lexer) != indent) {
                return errorToken(lexer, "Indentation error: inconsistent indentation");
            }
            
            lexer->start = lineStart;
            return getNextToken(lexer);
        }
        
        // Same indentation - continue normally
    }
    
    Token token = scanToken(lexer);
    
    // Set flag for next line after newline
    if (token.type == TOKEN_NEWLINE) {
        lexer->atLineStart = true;
    }
    
    return token;
}

void freeLexer(Lexer* lexer) {
    if (lexer != NULL) {
        if (lexer->indentStack != NULL) {
            free(lexer->indentStack);
        }
        free(lexer);
    }
}
