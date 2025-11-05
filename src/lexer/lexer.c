/**
 * lexer.c - Lexical Analyzer Implementation for EaC Compiler
 * 
 * This file implements the complete lexical analysis phase,
 * including Python-style significant whitespace/indentation handling.
 * 
 * Modified to use TRUE character-by-character keyword recognition.
 * Keywords are recognized while scanning, not after.
 * Removed logical operators: true, false, not, and, or
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
 
 /**
  * scanKeywordOrIdentifier - Scans character-by-character, detecting keywords in real-time
  * 
  * This function maintains a "possible keywords" state as it reads each character.
  * Once we know it's definitely not a keyword, we continue as a regular identifier.
  */
 static Token scanKeywordOrIdentifier(Lexer* lexer) {
     // We've already consumed the first character (in scanToken)
     char first = lexer->start[0];
     
     // Track which keywords are still possible
     typedef enum {
         KW_UNKNOWN,      // Haven't determined yet
         KW_IMPOSSIBLE,   // Definitely not a keyword
         // Specific keyword possibilities
         KW_BREAK, KW_CONTINUE, KW_ELSE, KW_FLEX, KW_FIXED,
         KW_FOR, KW_FROM, KW_FUNCTION, KW_IN, KW_IMPORT,
         KW_KINDLY, KW_MAYBE, KW_RETURN, KW_WHEN, KW_WHILE,
         KW_OUTPUT
     } KeywordState;
     
     KeywordState state = KW_UNKNOWN;
     
     // Initialize state based on first character
     switch (first) {
         case 'b': state = KW_BREAK; break;
         case 'c': state = KW_CONTINUE; break;
         case 'e': state = KW_ELSE; break;
         case 'f': state = KW_UNKNOWN; break;  // Multiple possibilities
         case 'i': state = KW_UNKNOWN; break;  // Multiple possibilities
         case 'k': state = KW_KINDLY; break;
         case 'm': state = KW_MAYBE; break;
         case 'o': state = KW_OUTPUT; break;
         case 'r': state = KW_RETURN; break;
         case 'w': state = KW_UNKNOWN; break;  // Multiple possibilities
         default:  state = KW_IMPOSSIBLE; break;
     }
     
     int position = 1;
     
     // Read character by character and refine state
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
                 // Disambiguate based on position
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
     
     // Determine final token type
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
     
     // Identifiers and keywords (character-by-character recognition)
     if (isAlpha(c)) {
         return scanKeywordOrIdentifier(lexer);
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