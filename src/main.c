/**
 * main.c - Complete EaC Lexer Implementation and Test Harness
 * 
 * This file contains:
 *   - Token type definitions (from token.h)
 *   - Lexer implementation with true character-by-character keyword recognition
 *   - Test harness that reads .eac files and outputs token tables
 * 
 * Keywords recognized while scanning (not after):
 *   break, continue, else, flex, fixed, for, from, function,
 *   in, import, when, while, return, output
 * 
 * Noise words: kindly, maybe
 * 
 * Logical operators (true, false, not, and, or) removed - treated as identifiers
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 #include <ctype.h>
 #include <sys/stat.h>
 #include <errno.h>
 
 // Ensure NULL is defined
 #ifndef NULL
 #define NULL ((void*)0)
 #endif
 
 // ===== TOKEN TYPE DEFINITIONS (from token.h) =====
 
 /**
  * TokenType - Enumeration of all token types in the EaC language
  */
 typedef enum {
     // Lifecycle tokens
     TOKEN_EOF,
     TOKEN_ERROR,
     
     // Whitespace & Structural
     TOKEN_NEWLINE,
     TOKEN_INDENT,
     TOKEN_DEDENT,
     
     // Literals
     TOKEN_IDENTIFIER,
     TOKEN_INTEGER,
     TOKEN_FLOAT,
     TOKEN_CHAR,
     TOKEN_STRING,
     TOKEN_COMMENT_LINE,
     TOKEN_COMMENT_BLOCK,
     
     // Primary Keywords
     TOKEN_FLEX,
     TOKEN_FIXED,
     TOKEN_WHEN,
     TOKEN_ELSE,
     TOKEN_OUTPUT,
     TOKEN_WHILE,
     TOKEN_FOR,
     TOKEN_IN,
     TOKEN_BREAK,
     TOKEN_CONTINUE,
     TOKEN_RETURN,
     TOKEN_FUNCTION,
     TOKEN_IMPORT,
     TOKEN_FROM,
     TOKEN_TRUE,      // Not recognized (kept for compatibility)
     TOKEN_FALSE,     // Not recognized (kept for compatibility)
     
     // Type Hint Keywords
     TOKEN_HINT_INT,
     TOKEN_HINT_FLOAT,
     TOKEN_HINT_STR,
     TOKEN_HINT_BOOL,
     TOKEN_HINT_CHAR,
     
     // Arithmetic Operators
     TOKEN_PLUS,
     TOKEN_MINUS,
     TOKEN_STAR,
     TOKEN_SLASH,
     TOKEN_PERCENT,
     TOKEN_CARET,
     TOKEN_VBAR,
     
     // Relational & Equality
     TOKEN_LESS,
     TOKEN_GREATER,
     TOKEN_EQUAL_EQUAL,
     TOKEN_LESS_EQUAL,
     TOKEN_GREATER_EQUAL,
     TOKEN_BANG_EQUAL,
     
     // Logical Operators
     TOKEN_AND,       // Not recognized (kept for compatibility)
     TOKEN_OR,        // Not recognized (kept for compatibility)
     TOKEN_NOT,       // Not recognized (kept for compatibility)
     
     // Noise Words
     TOKEN_NOISE,
     
     // Assignment Operators
     TOKEN_EQUAL,
     TOKEN_PLUS_EQUAL,
     TOKEN_MINUS_EQUAL,
     TOKEN_STAR_EQUAL,
     TOKEN_SLASH_EQUAL,
     TOKEN_PERCENT_EQUAL,
     
     // Delimiters
     TOKEN_LPAREN,
     TOKEN_RPAREN,
     TOKEN_LBRACKET,
     TOKEN_RBRACKET,
     TOKEN_COLON,
     TOKEN_COMMA,
     TOKEN_DOT
 } TokenType;
 
 /**
  * Token - Represents a single lexical token
  */
 typedef struct {
     TokenType type;
     const char* lexeme;
     int length;
     int line;
 } Token;
 
 // ===== LEXER IMPLEMENTATION =====
 
 #define MAX_INDENT_DEPTH 256
 
 /**
  * Lexer - Internal structure for lexer state
  */
 typedef struct {
     const char* source;
     const char* start;
     const char* current;
     int line;
     
     int indentStack[MAX_INDENT_DEPTH];
     int indentCount;
     int pendingDedents;
     
     bool atLineStart;
     bool needsNewline;
 } Lexer;
 
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
 
 // ===== True Character-by-Character Keyword Recognition =====
 
 static Token scanKeywordOrIdentifier(Lexer* lexer) {
     char first = lexer->start[0];
     
     typedef enum {
         KW_UNKNOWN, KW_IMPOSSIBLE,
         KW_BREAK, KW_CONTINUE, KW_ELSE, KW_FLEX, KW_FIXED,
         KW_FOR, KW_FROM, KW_FUNCTION, KW_IN, KW_IMPORT,
         KW_KINDLY, KW_MAYBE, KW_RETURN, KW_WHEN, KW_WHILE,
         KW_OUTPUT
     } KeywordState;
     
     KeywordState state = KW_UNKNOWN;
     
     switch (first) {
         case 'b': state = KW_BREAK; break;
         case 'c': state = KW_CONTINUE; break;
         case 'e': state = KW_ELSE; break;
         case 'f': state = KW_UNKNOWN; break;
         case 'i': state = KW_UNKNOWN; break;
         case 'k': state = KW_KINDLY; break;
         case 'm': state = KW_MAYBE; break;
         case 'o': state = KW_OUTPUT; break;
         case 'r': state = KW_RETURN; break;
         case 'w': state = KW_UNKNOWN; break;
         default:  state = KW_IMPOSSIBLE; break;
     }
     
     int position = 1;
     
     while (isAlphaNumeric(peek(lexer))) {
         char c = advance(lexer);
         
         switch (state) {
             case KW_BREAK:
                 if (position == 1 && c == 'r') break;
                 else if (position == 2 && c == 'e') break;
                 else if (position == 3 && c == 'a') break;
                 else if (position == 4 && c == 'k') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_CONTINUE:
                 if (position == 1 && c == 'o') break;
                 else if (position == 2 && c == 'n') break;
                 else if (position == 3 && c == 't') break;
                 else if (position == 4 && c == 'i') break;
                 else if (position == 5 && c == 'n') break;
                 else if (position == 6 && c == 'u') break;
                 else if (position == 7 && c == 'e') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_ELSE:
                 if (position == 1 && c == 'l') break;
                 else if (position == 2 && c == 's') break;
                 else if (position == 3 && c == 'e') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_FLEX:
                 if (position == 1 && c == 'l') break;
                 else if (position == 2 && c == 'e') break;
                 else if (position == 3 && c == 'x') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_FIXED:
                 if (position == 1 && c == 'i') break;
                 else if (position == 2 && c == 'x') break;
                 else if (position == 3 && c == 'e') break;
                 else if (position == 4 && c == 'd') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_FOR:
                 if (position == 1 && c == 'o') break;
                 else if (position == 2 && c == 'r') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_FROM:
                 if (position == 1 && c == 'r') break;
                 else if (position == 2 && c == 'o') break;
                 else if (position == 3 && c == 'm') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_FUNCTION:
                 if (position == 1 && c == 'u') break;
                 else if (position == 2 && c == 'n') break;
                 else if (position == 3 && c == 'c') break;
                 else if (position == 4 && c == 't') break;
                 else if (position == 5 && c == 'i') break;
                 else if (position == 6 && c == 'o') break;
                 else if (position == 7 && c == 'n') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_IN:
                 if (position == 1 && c == 'n') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_IMPORT:
                 if (position == 1 && c == 'm') break;
                 else if (position == 2 && c == 'p') break;
                 else if (position == 3 && c == 'o') break;
                 else if (position == 4 && c == 'r') break;
                 else if (position == 5 && c == 't') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_KINDLY:
                 if (position == 1 && c == 'i') break;
                 else if (position == 2 && c == 'n') break;
                 else if (position == 3 && c == 'd') break;
                 else if (position == 4 && c == 'l') break;
                 else if (position == 5 && c == 'y') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_MAYBE:
                 if (position == 1 && c == 'a') break;
                 else if (position == 2 && c == 'y') break;
                 else if (position == 3 && c == 'b') break;
                 else if (position == 4 && c == 'e') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_OUTPUT:
                 if (position == 1 && c == 'u') break;
                 else if (position == 2 && c == 't') break;
                 else if (position == 3 && c == 'p') break;
                 else if (position == 4 && c == 'u') break;
                 else if (position == 5 && c == 't') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_RETURN:
                 if (position == 1 && c == 'e') break;
                 else if (position == 2 && c == 't') break;
                 else if (position == 3 && c == 'u') break;
                 else if (position == 4 && c == 'r') break;
                 else if (position == 5 && c == 'n') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_WHEN:
                 if (position == 1 && c == 'h') break;
                 else if (position == 2 && c == 'e') break;
                 else if (position == 3 && c == 'n') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_WHILE:
                 if (position == 1 && c == 'h') break;
                 else if (position == 2 && c == 'i') break;
                 else if (position == 3 && c == 'l') break;
                 else if (position == 4 && c == 'e') break;
                 else state = KW_IMPOSSIBLE;
                 break;
             case KW_UNKNOWN:
                 if (first == 'f' && position == 1) {
                     if (c == 'l') state = KW_FLEX;
                     else if (c == 'i') state = KW_FIXED;
                     else if (c == 'o') state = KW_FOR;
                     else if (c == 'r') state = KW_FROM;
                     else if (c == 'u') state = KW_FUNCTION;
                     else state = KW_IMPOSSIBLE;
                 } else if (first == 'i' && position == 1) {
                     if (c == 'n') state = KW_IN;
                     else if (c == 'm') state = KW_IMPORT;
                     else state = KW_IMPOSSIBLE;
                 } else if (first == 'w') {
                     if (position == 1 && c == 'h') state = KW_UNKNOWN;
                     else if (position == 2 && c == 'e') state = KW_WHEN;
                     else if (position == 2 && c == 'i') state = KW_WHILE;
                     else state = KW_IMPOSSIBLE;
                 }
                 break;
             case KW_IMPOSSIBLE:
                 break;
         }
         
         position++;
         
         if (state == KW_IMPOSSIBLE) {
             while (isAlphaNumeric(peek(lexer))) {
                 advance(lexer);
             }
             return makeToken(lexer, TOKEN_IDENTIFIER);
         }
     }
     
     int length = (int)(lexer->current - lexer->start);
     
     switch (state) {
         case KW_BREAK:     return (length == 5) ? makeToken(lexer, TOKEN_BREAK) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_CONTINUE:  return (length == 8) ? makeToken(lexer, TOKEN_CONTINUE) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_ELSE:      return (length == 4) ? makeToken(lexer, TOKEN_ELSE) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_FLEX:      return (length == 4) ? makeToken(lexer, TOKEN_FLEX) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_FIXED:     return (length == 5) ? makeToken(lexer, TOKEN_FIXED) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_FOR:       return (length == 3) ? makeToken(lexer, TOKEN_FOR) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_FROM:      return (length == 4) ? makeToken(lexer, TOKEN_FROM) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_FUNCTION:  return (length == 8) ? makeToken(lexer, TOKEN_FUNCTION) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_IN:        return (length == 2) ? makeToken(lexer, TOKEN_IN) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_IMPORT:    return (length == 6) ? makeToken(lexer, TOKEN_IMPORT) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_KINDLY:    return (length == 6) ? makeToken(lexer, TOKEN_NOISE) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_MAYBE:     return (length == 5) ? makeToken(lexer, TOKEN_NOISE) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_OUTPUT:    return (length == 6) ? makeToken(lexer, TOKEN_OUTPUT) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_RETURN:    return (length == 6) ? makeToken(lexer, TOKEN_RETURN) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_WHEN:      return (length == 4) ? makeToken(lexer, TOKEN_WHEN) : makeToken(lexer, TOKEN_IDENTIFIER);
         case KW_WHILE:     return (length == 5) ? makeToken(lexer, TOKEN_WHILE) : makeToken(lexer, TOKEN_IDENTIFIER);
         default:           return makeToken(lexer, TOKEN_IDENTIFIER);
     }
 }
 
 // ===== Token Scanning Functions =====
 
 static Token scanString(Lexer* lexer) {
     while (peek(lexer) != '"' && !isAtEnd(lexer)) {
         if (peek(lexer) == '\n') {
             lexer->line++;
         }
         advance(lexer);
     }
     
     if (isAtEnd(lexer)) {
         return errorToken(lexer, "Unterminated string literal.");
     }
     
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
         advance(lexer);
     }
 
     if (peek(lexer) != '\'') {
         return errorToken(lexer, "Char literal must contain exactly one character.");
     }
 
     advance(lexer);
     return makeToken(lexer, TOKEN_CHAR);
 }
 
 static Token scanNumber(Lexer* lexer) {
     while (isDigit(peek(lexer))) {
         advance(lexer);
     }
     
     if (peek(lexer) == '.' && isDigit(peekNext(lexer))) {
         advance(lexer);
         
         while (isDigit(peek(lexer))) {
             advance(lexer);
         }
         
         return makeToken(lexer, TOKEN_FLOAT);
     }
     
     return makeToken(lexer, TOKEN_INTEGER);
 }
 
 // ===== Indentation Handling =====
 
 static int measureIndentation(Lexer* lexer) {
     int indent = 0;
     const char* ptr = lexer->current;
     
     while (*ptr == ' ' || *ptr == '\t') {
         if (*ptr == '\t') {
             indent += 4;
         } else {
             indent += 1;
         }
         ptr++;
     }
     
     return indent;
 }
 
 static Token handleIndentation(Lexer* lexer) {
     int newIndent = measureIndentation(lexer);
     
     while (peek(lexer) == ' ' || peek(lexer) == '\t') {
         advance(lexer);
     }
     
     if (peek(lexer) == '\n') {
         return syntheticToken(lexer, TOKEN_EOF);
     }
     
     int currentIndent = lexer->indentStack[lexer->indentCount - 1];
     
     if (newIndent > currentIndent) {
         if (lexer->indentCount >= MAX_INDENT_DEPTH) {
             return errorToken(lexer, "Maximum indentation depth exceeded.");
         }
         lexer->indentStack[lexer->indentCount++] = newIndent;
         lexer->start = lexer->current;
         return syntheticToken(lexer, TOKEN_INDENT);
     } else if (newIndent < currentIndent) {
         while (lexer->indentCount > 1 && 
                lexer->indentStack[lexer->indentCount - 1] > newIndent) {
             lexer->indentCount--;
             lexer->pendingDedents++;
         }
         
         if (lexer->indentStack[lexer->indentCount - 1] != newIndent) {
             return errorToken(lexer, "Indentation error: dedent to invalid level.");
         }
         
         lexer->pendingDedents--;
         lexer->start = lexer->current;
         return syntheticToken(lexer, TOKEN_DEDENT);
     }
     
     return syntheticToken(lexer, TOKEN_EOF);
 }
 
 // ===== Main Token Scanner =====
 
 static Token scanToken(Lexer* lexer) {
     if (lexer->pendingDedents > 0) {
         lexer->pendingDedents--;
         return syntheticToken(lexer, TOKEN_DEDENT);
     }
     
     if (lexer->needsNewline) {
         lexer->needsNewline = false;
         lexer->atLineStart = true;
         return syntheticToken(lexer, TOKEN_NEWLINE);
     }
     
     if (lexer->atLineStart && !isAtEnd(lexer)) {
         lexer->atLineStart = false;
         
         while (peek(lexer) == '\n' || peek(lexer) == ' ' || 
                peek(lexer) == '\t' || peek(lexer) == '\r') {
             if (peek(lexer) == '\n') {
                 lexer->line++;
                 advance(lexer);
             } else {
                 Token indentToken = handleIndentation(lexer);
                 if (indentToken.type == TOKEN_INDENT || 
                     indentToken.type == TOKEN_DEDENT ||
                     indentToken.type == TOKEN_ERROR) {
                     return indentToken;
                 }
                 break;
             }
         }
     }
     
     skipWhitespace(lexer);
     lexer->start = lexer->current;
     
     if (isAtEnd(lexer)) {
         if (lexer->indentCount > 1) {
             lexer->indentCount--;
             return syntheticToken(lexer, TOKEN_DEDENT);
         }
         return makeToken(lexer, TOKEN_EOF);
     }
     
     char c = advance(lexer);
     
     if (isAlpha(c)) {
         return scanKeywordOrIdentifier(lexer);
     }
     
     if (isDigit(c)) {
         return scanNumber(lexer);
     }
     
     switch (c) {
         case '\n':
             lexer->line++;
             lexer->atLineStart = true;
             return syntheticToken(lexer, TOKEN_NEWLINE);
         case '"':  return scanString(lexer);
         case '\'': return scanChar(lexer);
         case '(':  return makeToken(lexer, TOKEN_LPAREN);
         case ')':  return makeToken(lexer, TOKEN_RPAREN);
         case '[':  return makeToken(lexer, TOKEN_LBRACKET);
         case ']':  return makeToken(lexer, TOKEN_RBRACKET);
         case ':':  return makeToken(lexer, TOKEN_COLON);
         case ',':  return makeToken(lexer, TOKEN_COMMA);
         case '.':  return makeToken(lexer, TOKEN_DOT);
         case '+':  return makeToken(lexer, match(lexer, '=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
         case '-':  return makeToken(lexer, match(lexer, '=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
         case '*':  return makeToken(lexer, match(lexer, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
         case '%':  return makeToken(lexer, match(lexer, '=') ? TOKEN_PERCENT_EQUAL : TOKEN_PERCENT);
         case '/':
             if (match(lexer, '=')) {
                 return makeToken(lexer, TOKEN_SLASH_EQUAL);
             } else if (match(lexer, '*')) {
                 return scanBlockComment(lexer);
             }
             return makeToken(lexer, TOKEN_SLASH);
         case '^':  return makeToken(lexer, TOKEN_CARET);
         case '|':  return makeToken(lexer, TOKEN_VBAR);
         case '=':  return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
         case '<':  return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
         case '>':  return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
         case '!':
             if (match(lexer, '=')) {
                 return makeToken(lexer, TOKEN_BANG_EQUAL);
             }
             return errorToken(lexer, "Unexpected character '!'.");
         case '#':  return scanLineComment(lexer);
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
     
     lexer->indentStack[0] = 0;
     lexer->indentCount = 1;
     lexer->pendingDedents = 0;
     
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
 
 // ===== TEST HARNESS - Token Type Name Mapping =====
 
 const char* getTokenTypeName(TokenType type) {
     switch (type) {
         case TOKEN_EOF:              return "EOF";
         case TOKEN_ERROR:            return "ERROR";
         case TOKEN_NEWLINE:          return "NEWLINE";
         case TOKEN_INDENT:           return "INDENT";
         case TOKEN_DEDENT:           return "DEDENT";
         case TOKEN_IDENTIFIER:       return "IDENTIFIER";
         case TOKEN_INTEGER:          return "INTEGER";
         case TOKEN_FLOAT:            return "FLOAT";
         case TOKEN_CHAR:             return "CHAR";
         case TOKEN_STRING:           return "STRING";
         case TOKEN_COMMENT_LINE:     return "COMMENT";
         case TOKEN_COMMENT_BLOCK:    return "COMMENT";
         case TOKEN_FLEX:             return "KEYWORD";
         case TOKEN_FIXED:            return "KEYWORD";
         case TOKEN_WHEN:             return "KEYWORD";
         case TOKEN_ELSE:             return "KEYWORD";
         case TOKEN_OUTPUT:           return "KEYWORD";
         case TOKEN_WHILE:            return "KEYWORD";
         case TOKEN_FOR:              return "KEYWORD";
         case TOKEN_IN:               return "KEYWORD";
         case TOKEN_BREAK:            return "KEYWORD";
         case TOKEN_CONTINUE:         return "KEYWORD";
         case TOKEN_RETURN:           return "KEYWORD";
         case TOKEN_FUNCTION:         return "KEYWORD";
         case TOKEN_IMPORT:           return "KEYWORD";
         case TOKEN_FROM:             return "KEYWORD";
         case TOKEN_TRUE:             return "KEYWORD";
         case TOKEN_FALSE:            return "KEYWORD";
         case TOKEN_HINT_INT:         return "HINT_KEYWORD";
         case TOKEN_HINT_FLOAT:       return "HINT_KEYWORD";
         case TOKEN_HINT_STR:         return "HINT_KEYWORD";
         case TOKEN_HINT_BOOL:        return "HINT_KEYWORD";
         case TOKEN_HINT_CHAR:        return "HINT_KEYWORD";
         case TOKEN_PLUS:             return "ARITHMETIC";
         case TOKEN_MINUS:            return "ARITHMETIC";
         case TOKEN_STAR:             return "ARITHMETIC";
         case TOKEN_SLASH:            return "ARITHMETIC";
         case TOKEN_PERCENT:          return "ARITHMETIC";
         case TOKEN_CARET:            return "ARITHMETIC";
         case TOKEN_VBAR:             return "ARITHMETIC";
         case TOKEN_LESS:             return "RELATIONAL";
         case TOKEN_GREATER:          return "RELATIONAL";
         case TOKEN_EQUAL_EQUAL:      return "RELATIONAL";
         case TOKEN_LESS_EQUAL:       return "RELATIONAL";
         case TOKEN_GREATER_EQUAL:    return "RELATIONAL";
         case TOKEN_BANG_EQUAL:       return "RELATIONAL";
         case TOKEN_AND:              return "LOGICAL";
         case TOKEN_OR:               return "LOGICAL";
         case TOKEN_NOT:              return "LOGICAL";
         case TOKEN_NOISE:            return "NOISE";
         case TOKEN_EQUAL:            return "ASSIGNMENT";
         case TOKEN_PLUS_EQUAL:       return "ASSIGNMENT";
         case TOKEN_MINUS_EQUAL:      return "ASSIGNMENT";
         case TOKEN_STAR_EQUAL:       return "ASSIGNMENT";
         case TOKEN_SLASH_EQUAL:      return "ASSIGNMENT";
         case TOKEN_PERCENT_EQUAL:    return "ASSIGNMENT";
         case TOKEN_LPAREN:           return "DELIMITER";
         case TOKEN_RPAREN:           return "DELIMITER";
         case TOKEN_LBRACKET:         return "DELIMITER";
         case TOKEN_RBRACKET:         return "DELIMITER";
         case TOKEN_COLON:            return "DELIMITER";
         case TOKEN_COMMA:            return "DELIMITER";
         case TOKEN_DOT:              return "DELIMITER";
         default:                     return "UNKNOWN";
     }
 }
 
 const char* getTokenSpecial(TokenType type) {
     switch (type) {
         case TOKEN_EOF:              return "EOF";
         case TOKEN_ERROR:            return "ERROR";
         case TOKEN_NEWLINE:          return "NEWLINE";
         case TOKEN_INDENT:           return "INDENT";
         case TOKEN_DEDENT:           return "DEDENT";
         case TOKEN_IDENTIFIER:       return "IDENTIFIER";
         case TOKEN_INTEGER:          return "INTEGER";
         case TOKEN_FLOAT:            return "FLOAT";
         case TOKEN_STRING:           return "STRING";
         case TOKEN_CHAR:             return "CHAR";
         case TOKEN_COMMENT_LINE:     return "COMMENT_LINE";
         case TOKEN_COMMENT_BLOCK:    return "COMMENT_BLOCK";
         case TOKEN_FLEX:             return "FLEX";
         case TOKEN_FIXED:            return "FIXED";
         case TOKEN_WHEN:             return "WHEN";
         case TOKEN_ELSE:             return "ELSE";
         case TOKEN_OUTPUT:           return "OUTPUT";
         case TOKEN_WHILE:            return "WHILE";
         case TOKEN_FOR:              return "FOR";
         case TOKEN_IN:               return "IN";
         case TOKEN_BREAK:            return "BREAK";
         case TOKEN_CONTINUE:         return "CONTINUE";
         case TOKEN_RETURN:           return "RETURN";
         case TOKEN_FUNCTION:         return "FUNCTION";
         case TOKEN_IMPORT:           return "IMPORT";
         case TOKEN_FROM:             return "FROM";
         case TOKEN_HINT_INT:         return "HINT_INT";
         case TOKEN_HINT_FLOAT:       return "HINT_FLOAT";
         case TOKEN_HINT_STR:         return "HINT_STR";
         case TOKEN_HINT_BOOL:        return "HINT_BOOL";
         case TOKEN_HINT_CHAR:        return "HINT_CHAR";
         case TOKEN_PLUS:             return "PLUS";
         case TOKEN_MINUS:            return "MINUS";
         case TOKEN_STAR:             return "STAR";
         case TOKEN_SLASH:            return "SLASH";
         case TOKEN_PERCENT:          return "PERCENT";
         case TOKEN_CARET:            return "CARET";
         case TOKEN_VBAR:             return "VBAR";
         case TOKEN_LESS:             return "LESS";
         case TOKEN_GREATER:          return "GREATER";
         case TOKEN_EQUAL_EQUAL:      return "EQUAL_EQUAL";
         case TOKEN_LESS_EQUAL:       return "LESS_EQUAL";
         case TOKEN_GREATER_EQUAL:    return "GREATER_EQUAL";
         case TOKEN_BANG_EQUAL:       return "BANG_EQUAL";
         case TOKEN_AND:              return "AND";
         case TOKEN_OR:               return "OR";
         case TOKEN_NOT:              return "NOT";
         case TOKEN_NOISE:            return "NOISE";
         case TOKEN_EQUAL:            return "EQUAL";
         case TOKEN_PLUS_EQUAL:       return "PLUS_EQUAL";
         case TOKEN_MINUS_EQUAL:      return "MINUS_EQUAL";
         case TOKEN_STAR_EQUAL:       return "STAR_EQUAL";
         case TOKEN_SLASH_EQUAL:      return "SLASH_EQUAL";
         case TOKEN_PERCENT_EQUAL:    return "PERCENT_EQUAL";
         case TOKEN_LPAREN:           return "LPAREN";
         case TOKEN_RPAREN:           return "RPAREN";
         case TOKEN_LBRACKET:         return "LBRACKET";
         case TOKEN_RBRACKET:         return "RBRACKET";
         case TOKEN_COLON:            return "COLON";
         case TOKEN_COMMA:            return "COMMA";
         case TOKEN_DOT:              return "DOT";
         default:                     return "UNKNOWN";
     }
 }
 
 // ===== File Utilities =====
 
 static bool hasEacExtension(const char* path) {
     size_t len = strlen(path);
     if (len < 4) {
         return false;
     }
     const char* ext = path + len - 4;
     return ext[0] == '.' &&
            tolower((unsigned char)ext[1]) == 'e' &&
            tolower((unsigned char)ext[2]) == 'a' &&
            tolower((unsigned char)ext[3]) == 'c';
 }
 
 char* readFile(const char* path) {
     FILE* file = fopen(path, "rb");
     if (file == NULL) {
         fprintf(stderr, "Error: Could not open file '%s'.\n", path);
         return NULL;
     }
     
     fseek(file, 0, SEEK_END);
     long fileSize = ftell(file);
     rewind(file);
     
     char* buffer = (char*)malloc(fileSize + 1);
     if (buffer == NULL) {
         fprintf(stderr, "Error: Not enough memory to read '%s'.\n", path);
         fclose(file);
         return NULL;
     }
     
     size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
     if (bytesRead < (size_t)fileSize) {
         fprintf(stderr, "Error: Could not read file '%s'.\n", path);
         free(buffer);
         fclose(file);
         return NULL;
     }
     
     buffer[bytesRead] = '\0';
     fclose(file);
     return buffer;
 }
 
 bool createDirectory(const char* path) {
 #ifdef _WIN32
     if (mkdir(path) != 0 && errno != EEXIST) {
 #else
     if (mkdir(path, 0755) != 0 && errno != EEXIST) {
 #endif
         fprintf(stderr, "Error: Could not create directory '%s'.\n", path);
         return false;
     }
     return true;
 }
 
 const char* extractFilename(const char* path) {
     const char* filename = strrchr(path, '/');
     if (filename == NULL) {
         filename = strrchr(path, '\\');
     }
     return filename ? filename + 1 : path;
 }
 
 char* generateOutputFilename(const char* inputPath) {
     const char* filename = extractFilename(inputPath);
     size_t len = strlen(filename);
     
     if (!createDirectory("output")) {
         return NULL;
     }
     
     const char* prefix = "output/symbol_table_";
     size_t prefixLen = strlen(prefix);
     const char* suffix = ".txt";
     size_t suffixLen = strlen(suffix);
 
     char* output;
     if (len > 4 && strcmp(filename + len - 4, ".eac") == 0) {
         size_t stemLen = len - 4;
         output = (char*)malloc(prefixLen + stemLen + suffixLen + 1);
         if (output == NULL) {
             fprintf(stderr, "Error: Memory allocation failed.\n");
             return NULL;
         }
         memcpy(output, prefix, prefixLen);
         memcpy(output + prefixLen, filename, stemLen);
         memcpy(output + prefixLen + stemLen, suffix, suffixLen);
         output[prefixLen + stemLen + suffixLen] = '\0';
     } else {
         output = (char*)malloc(prefixLen + len + suffixLen + 1);
         if (output == NULL) {
             fprintf(stderr, "Error: Memory allocation failed.\n");
             return NULL;
         }
         memcpy(output, prefix, prefixLen);
         memcpy(output + prefixLen, filename, len);
         memcpy(output + prefixLen + len, suffix, suffixLen);
         output[prefixLen + len + suffixLen] = '\0';
     }
 
     return output;
 }
 
 // ===== Token Printer =====
 
 void printToken(FILE* outFile, Token token) {
     fprintf(outFile, "%-6d", token.line);
 
     char lexeme[128] = {0};
     
     if (token.type == TOKEN_NEWLINE) {
         snprintf(lexeme, sizeof(lexeme), "\\n");
     } else if (token.type == TOKEN_INDENT) {
         snprintf(lexeme, sizeof(lexeme), ">>INDENT");
     } else if (token.type == TOKEN_DEDENT) {
         snprintf(lexeme, sizeof(lexeme), "<<DEDENT");
     } else if ((token.type == TOKEN_COMMENT_LINE || token.type == TOKEN_COMMENT_BLOCK) &&
                token.length > 0) {
         int maxCopy = token.length < (int)sizeof(lexeme) - 1 ? token.length : (int)sizeof(lexeme) - 1;
         int j = 0;
         for (int i = 0; i < maxCopy; i++) {
             char ch = token.lexeme[i];
             if (ch == '\r' || ch == '\n' || ch == '\t') {
                 ch = ' ';
             }
             lexeme[j++] = ch;
         }
         lexeme[j] = '\0';
     } else if (token.length > 0 && token.length < 127) {
         snprintf(lexeme, sizeof(lexeme), "%.*s", token.length, token.lexeme);
     }
     
     fprintf(outFile, "%-20s", lexeme);
     fprintf(outFile, "%-20s", getTokenTypeName(token.type));
     
     const char* special = getTokenSpecial(token.type);
     fprintf(outFile, "%s\n", special);
 }
 
 // ===== Main Program =====
 
 int main(int argc, char* argv[]) {
     if (argc < 2) {
         fprintf(stderr, "Usage: %s <source-file.eac>\n", argv[0]);
         return 1;
     }
     
     const char* sourcePath = argv[1];
 
     if (!hasEacExtension(sourcePath)) {
         fprintf(stderr, "Error: Source file '%s' must have a .eac extension.\n", sourcePath);
         return 1;
     }
     
     if (!createDirectory("output")) {
         return 1;
     }
     
     char* outputPath = generateOutputFilename(sourcePath);
     if (outputPath == NULL) {
         return 1;
     }
     
     char* source = readFile(sourcePath);
     if (source == NULL) {
         free(outputPath);
         return 1;
     }
     
     Lexer* lexer = initLexer(source);
     if (lexer == NULL) {
         fprintf(stderr, "Error: Failed to initialize lexer.\n");
         free(source);
         free(outputPath);
         return 1;
     }
     
     FILE* outFile = fopen(outputPath, "w");
     if (outFile == NULL) {
         fprintf(stderr, "Error: Could not create output file '%s'.\n", outputPath);
         freeLexer(lexer);
         free(source);
         free(outputPath);
         return 1;
     }
     
     fprintf(outFile, "Line   Lexeme              Token               Token Special\n");
     fprintf(outFile, "==========================================================================\n");
     fprintf(outFile, "\n");
     
     int tokenCount = 0;
     bool hasErrors = false;
     
     for (;;) {
         Token token = getNextToken(lexer);
         
         if (token.type == TOKEN_ERROR) {
             hasErrors = true;
             break;
         }
         
         if (token.type == TOKEN_EOF) {
             break;
         }
         
         printToken(outFile, token);
         tokenCount++;
     }
     
     printf("Tokenization complete: %s -> %s\n", sourcePath, outputPath);
     printf("Total tokens: %d\n", tokenCount);
     
     if (hasErrors) {
         printf("Status: ERRORS DETECTED\n");
     } else {
         printf("Status: SUCCESS\n");
     }
     
     fclose(outFile);
     freeLexer(lexer);
     free(source);
     free(outputPath);
     
     return hasErrors ? 1 : 0;
 }