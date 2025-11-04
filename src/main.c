/**
 * main.c - Test Harness for EaC Lexer
 * 
 * This program reads an EaC source file and writes all tokens
 * to an output text file in a clean table format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#include "common/token.h"
#include "lexer/lexer.h"

// ===== Token Type Name Mapping =====

/**
 * getTokenTypeName - Returns the token type name (middle column)
 */
const char* getTokenTypeName(TokenType type) {
    switch (type) {
        // Lifecycle
        case TOKEN_EOF:              return "EOF";
        case TOKEN_ERROR:            return "ERROR";
        
        // Whitespace & Structural
        case TOKEN_NEWLINE:          return "NEWLINE";
        case TOKEN_INDENT:           return "INDENT";
        case TOKEN_DEDENT:           return "DEDENT";
        
        // Literals
        case TOKEN_IDENTIFIER:       return "IDENTIFIER";
        case TOKEN_INTEGER:          return "INTEGER";
        case TOKEN_FLOAT:            return "FLOAT";
        case TOKEN_CHAR:             return "CHAR";
        case TOKEN_STRING:           return "STRING";
        case TOKEN_COMMENT_LINE:     return "COMMENT";
        case TOKEN_COMMENT_BLOCK:    return "COMMENT";
        
        // Primary Keywords
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
        
        // Type Hint Keywords
        case TOKEN_HINT_INT:         return "HINT_KEYWORD";
        case TOKEN_HINT_FLOAT:       return "HINT_KEYWORD";
        case TOKEN_HINT_STR:         return "HINT_KEYWORD";
        case TOKEN_HINT_BOOL:        return "HINT_KEYWORD";
        case TOKEN_HINT_CHAR:        return "HINT_KEYWORD";
        
        // Arithmetic Operators
        case TOKEN_PLUS:             return "ARITHMETIC";
        case TOKEN_MINUS:            return "ARITHMETIC";
        case TOKEN_STAR:             return "ARITHMETIC";
        case TOKEN_SLASH:            return "ARITHMETIC";
        case TOKEN_PERCENT:          return "ARITHMETIC";
        case TOKEN_CARET:            return "ARITHMETIC";
        case TOKEN_VBAR:             return "ARITHMETIC";
        
        // Relational & Equality
        case TOKEN_LESS:             return "RELATIONAL";
        case TOKEN_GREATER:          return "RELATIONAL";
        case TOKEN_EQUAL_EQUAL:      return "RELATIONAL";
        case TOKEN_LESS_EQUAL:       return "RELATIONAL";
        case TOKEN_GREATER_EQUAL:    return "RELATIONAL";
        case TOKEN_BANG_EQUAL:       return "RELATIONAL";
        
        // Logical Operators
        case TOKEN_AND:              return "LOGICAL";
        case TOKEN_OR:               return "LOGICAL";
        case TOKEN_NOT:              return "LOGICAL";

        // Noise Words
        case TOKEN_NOISE:            return "NOISE";
        
        // Assignment Operators
        case TOKEN_EQUAL:            return "ASSIGNMENT";
        case TOKEN_PLUS_EQUAL:       return "ASSIGNMENT";
        case TOKEN_MINUS_EQUAL:      return "ASSIGNMENT";
        case TOKEN_STAR_EQUAL:       return "ASSIGNMENT";
        case TOKEN_SLASH_EQUAL:      return "ASSIGNMENT";
        case TOKEN_PERCENT_EQUAL:    return "ASSIGNMENT";
        
        // Delimiters
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

/**
 * getTokenSpecial - Returns the token special name (right column)
 */
const char* getTokenSpecial(TokenType type) {
    switch (type) {
        // Lifecycle
        case TOKEN_EOF:              return "EOF";
        case TOKEN_ERROR:            return "ERROR";
        
        // Whitespace & Structural
        case TOKEN_NEWLINE:          return "NEWLINE";
        case TOKEN_INDENT:           return "INDENT";
        case TOKEN_DEDENT:           return "DEDENT";
        
        // Literals
        case TOKEN_IDENTIFIER:       return "IDENTIFIER";
        case TOKEN_INTEGER:          return "INTEGER";
        case TOKEN_FLOAT:            return "FLOAT";
        case TOKEN_STRING:           return "STRING";
        case TOKEN_CHAR:             return "CHAR";
        case TOKEN_COMMENT_LINE:     return "COMMENT_LINE";
        case TOKEN_COMMENT_BLOCK:    return "COMMENT_BLOCK";
        
        // Primary Keywords
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
        case TOKEN_TRUE:             return "TRUE";
        case TOKEN_FALSE:            return "FALSE";
        
        // Type Hint Keywords
        case TOKEN_HINT_INT:         return "HINT_INT";
        case TOKEN_HINT_FLOAT:       return "HINT_FLOAT";
        case TOKEN_HINT_STR:         return "HINT_STR";
        case TOKEN_HINT_BOOL:        return "HINT_BOOL";
        case TOKEN_HINT_CHAR:        return "HINT_CHAR";
        
        // Arithmetic Operators
        case TOKEN_PLUS:             return "PLUS";
        case TOKEN_MINUS:            return "MINUS";
        case TOKEN_STAR:             return "STAR";
        case TOKEN_SLASH:            return "SLASH";
        case TOKEN_PERCENT:          return "PERCENT";
        case TOKEN_CARET:            return "CARET";
        case TOKEN_VBAR:             return "VBAR";
        
        // Relational & Equality
        case TOKEN_LESS:             return "LESS";
        case TOKEN_GREATER:          return "GREATER";
        case TOKEN_EQUAL_EQUAL:      return "EQUAL_EQUAL";
        case TOKEN_LESS_EQUAL:       return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL:    return "GREATER_EQUAL";
        case TOKEN_BANG_EQUAL:       return "BANG_EQUAL";
        
        // Logical Operators
        case TOKEN_AND:              return "AND";
        case TOKEN_OR:               return "OR";
        case TOKEN_NOT:              return "NOT";

        // Noise Words
        case TOKEN_NOISE:            return "NOISE";
        
        // Assignment Operators
        case TOKEN_EQUAL:            return "EQUAL";
        case TOKEN_PLUS_EQUAL:       return "PLUS_EQUAL";
        case TOKEN_MINUS_EQUAL:      return "MINUS_EQUAL";
        case TOKEN_STAR_EQUAL:       return "STAR_EQUAL";
        case TOKEN_SLASH_EQUAL:      return "SLASH_EQUAL";
        case TOKEN_PERCENT_EQUAL:    return "PERCENT_EQUAL";
        
        // Delimiters
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

// ===== File Reading Utility =====

/**
 * readFile - Reads entire file contents into a dynamically allocated string
 */
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

// ===== Directory Utility =====

/**
 * createDirectory - Creates a directory if it doesn't exist
 */
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

/**
 * extractFilename - Extracts filename from full path
 */
const char* extractFilename(const char* path) {
    const char* filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = strrchr(path, '\\');
    }
    return filename ? filename + 1 : path;
}

// ===== Output Filename Generator =====

/**
 * generateOutputFilename - Creates output filename in output/ directory
 */
char* generateOutputFilename(const char* inputPath) {
    const char* filename = extractFilename(inputPath);
    size_t len = strlen(filename);
    
    // Create output directory
    if (!createDirectory("output")) {
        return NULL;
    }
    
    // Build output path
    char* output;
    if (len > 4 && strcmp(filename + len - 4, ".eac") == 0) {
        output = (char*)malloc(strlen("output/") + len + 1);
        if (output == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        strcpy(output, "output/");
        strcat(output, filename);
        strcpy(output + strlen("output/") + len - 4, ".txt");
    } else {
        output = (char*)malloc(strlen("output/") + len + 5);
        if (output == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        strcpy(output, "output/");
        strcat(output, filename);
        strcat(output, ".txt");
    }
    
    return output;
}

// ===== Token Printer =====

/**
 * printToken - Prints a token in clean table format
 */
void printToken(FILE* outFile, Token token) {
    // Column 1: Line number (padded to 5 chars)
    fprintf(outFile, "%-6d", token.line);

    // Column 2: Lexeme (20 chars wide, left-aligned)
    char lexeme[128] = {0};
    
    // Special handling for structural tokens
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
    
    // Column 2: Token type (20 chars wide, left-aligned)
    fprintf(outFile, "%-20s", getTokenTypeName(token.type));
    
    // Column 3: Token special
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
    
    // Create output directory
    if (!createDirectory("output")) {
        return 1;
    }
    
    // Set output filename
    const char* outputPath = "output/symbol_table.txt";
    
    // Read source file
    char* source = readFile(sourcePath);
    if (source == NULL) {
        return 1;
    }
    
    // Initialize lexer
    Lexer* lexer = initLexer(source);
    if (lexer == NULL) {
        fprintf(stderr, "Error: Failed to initialize lexer.\n");
        free(source);
        return 1;
    }
    
    // Open output file
    FILE* outFile = fopen(outputPath, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error: Could not create output file '%s'.\n", outputPath);
        freeLexer(lexer);
        free(source);
        return 1;
    }
    
    // Write header
    fprintf(outFile, "Line   Lexeme              Token               Token Special\n");
    fprintf(outFile, "==========================================================================\n");
    fprintf(outFile, "\n");
    
    // Scan and print all tokens
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
    
    // Console summary
    printf("Tokenization complete: %s -> %s\n", sourcePath, outputPath);
    printf("Total tokens: %d\n", tokenCount);
    
    if (hasErrors) {
        printf("Status: ERRORS DETECTED\n");
    } else {
        printf("Status: SUCCESS\n");
    }
    
    // Cleanup
    fclose(outFile);
    freeLexer(lexer);
    free(source);
    
    return hasErrors ? 1 : 0;
}