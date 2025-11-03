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
        case TOKEN_EOF:              return "eof";
        case TOKEN_ERROR:            return "error";
        
        // Whitespace & Structural
        case TOKEN_NEWLINE:          return "newline";
        case TOKEN_INDENT:           return "indent";
        case TOKEN_DEDENT:           return "dedent";
        
        // Literals
        case TOKEN_IDENTIFIER:       return "identifier";
        case TOKEN_INTEGER:          return "integer";
        case TOKEN_FLOAT:            return "float";
        case TOKEN_STRING:           return "string";
        
        // Primary Keywords
        case TOKEN_FLEX:             return "keyword";
        case TOKEN_FIXED:            return "keyword";
        case TOKEN_WHEN:             return "keyword";
        case TOKEN_ELSE:             return "keyword";
        case TOKEN_OUTPUT:           return "keyword";
        case TOKEN_WHILE:            return "keyword";
        case TOKEN_FOR:              return "keyword";
        case TOKEN_IN:               return "keyword";
        case TOKEN_BREAK:            return "keyword";
        case TOKEN_CONTINUE:         return "keyword";
        case TOKEN_RETURN:           return "keyword";
        case TOKEN_TRUE:             return "keyword";
        case TOKEN_FALSE:            return "keyword";
        
        // Type Hint Keywords
        case TOKEN_HINT_INT:         return "keyword";
        case TOKEN_HINT_FLOAT:       return "keyword";
        case TOKEN_HINT_STR:         return "keyword";
        case TOKEN_HINT_BOOL:        return "keyword";
        
        // Arithmetic Operators
        case TOKEN_PLUS:             return "arithmetic";
        case TOKEN_MINUS:            return "arithmetic";
        case TOKEN_STAR:             return "arithmetic";
        case TOKEN_SLASH:            return "arithmetic";
        case TOKEN_PERCENT:          return "arithmetic";
        case TOKEN_CARET:            return "arithmetic";
        case TOKEN_VBAR:             return "arithmetic";
        
        // Relational & Equality
        case TOKEN_LESS:             return "relational";
        case TOKEN_GREATER:          return "relational";
        case TOKEN_EQUAL_EQUAL:      return "relational";
        case TOKEN_LESS_EQUAL:       return "relational";
        case TOKEN_GREATER_EQUAL:    return "relational";
        case TOKEN_BANG_EQUAL:       return "relational";
        
        // Logical Operators
        case TOKEN_AND:              return "logical";
        case TOKEN_OR:               return "logical";
        case TOKEN_NOT:              return "logical";
        
        // Assignment Operators
        case TOKEN_EQUAL:            return "arithmetic";
        case TOKEN_PLUS_EQUAL:       return "arithmetic";
        case TOKEN_MINUS_EQUAL:      return "arithmetic";
        case TOKEN_STAR_EQUAL:       return "arithmetic";
        case TOKEN_SLASH_EQUAL:      return "arithmetic";
        case TOKEN_PERCENT_EQUAL:    return "arithmetic";
        
        // Delimiters
        case TOKEN_LPAREN:           return "delimiter";
        case TOKEN_RPAREN:           return "delimiter";
        case TOKEN_LBRACKET:         return "delimiter";
        case TOKEN_RBRACKET:         return "delimiter";
        case TOKEN_COLON:            return "delimiter";
        case TOKEN_COMMA:            return "delimiter";
        case TOKEN_DOT:              return "delimiter";
        
        default:                     return "unknown";
    }
}

/**
 * getTokenSpecial - Returns the token special name (right column)
 */
const char* getTokenSpecial(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER:       return "identifier";
        case TOKEN_INTEGER:          return "integer";
        case TOKEN_FLOAT:            return "float";
        case TOKEN_STRING:           return "string";
        
        // Keywords
        case TOKEN_FLEX:             return "flex";
        case TOKEN_FIXED:            return "fixed";
        case TOKEN_WHEN:             return "when";
        case TOKEN_ELSE:             return "else";
        case TOKEN_OUTPUT:           return "output";
        case TOKEN_WHILE:            return "while";
        case TOKEN_FOR:              return "for";
        case TOKEN_IN:               return "in";
        case TOKEN_BREAK:            return "break";
        case TOKEN_CONTINUE:         return "continue";
        case TOKEN_RETURN:           return "return";
        case TOKEN_TRUE:             return "true";
        case TOKEN_FALSE:            return "false";
        
        // Type hints
        case TOKEN_HINT_INT:         return "int";
        case TOKEN_HINT_FLOAT:       return "float";
        case TOKEN_HINT_STR:         return "str";
        case TOKEN_HINT_BOOL:        return "bool";
        
        // Operators
        case TOKEN_PLUS:             return "add";
        case TOKEN_MINUS:            return "subtract";
        case TOKEN_STAR:             return "multiply";
        case TOKEN_SLASH:            return "divide";
        case TOKEN_PERCENT:          return "modulo";
        case TOKEN_CARET:            return "power";
        case TOKEN_VBAR:             return "pipe";
        
        case TOKEN_LESS:             return "less_than";
        case TOKEN_GREATER:          return "greater_than";
        case TOKEN_EQUAL_EQUAL:      return "equal";
        case TOKEN_LESS_EQUAL:       return "less_equal";
        case TOKEN_GREATER_EQUAL:    return "greater_equal";
        case TOKEN_BANG_EQUAL:       return "not_equal";
        
        case TOKEN_AND:              return "and";
        case TOKEN_OR:               return "or";
        case TOKEN_NOT:              return "not";
        
        case TOKEN_EQUAL:            return "assignment";
        case TOKEN_PLUS_EQUAL:       return "add_assign";
        case TOKEN_MINUS_EQUAL:      return "sub_assign";
        case TOKEN_STAR_EQUAL:       return "mul_assign";
        case TOKEN_SLASH_EQUAL:      return "div_assign";
        case TOKEN_PERCENT_EQUAL:    return "mod_assign";
        
        case TOKEN_LPAREN:           return "left_paren";
        case TOKEN_RPAREN:           return "right_paren";
        case TOKEN_LBRACKET:         return "left_bracket";
        case TOKEN_RBRACKET:         return "right_bracket";
        case TOKEN_COLON:            return "colon";
        case TOKEN_COMMA:            return "comma";
        case TOKEN_DOT:              return "dot";
        
        case TOKEN_NEWLINE:          return "newline";
        case TOKEN_INDENT:           return "indent";
        case TOKEN_DEDENT:           return "dedent";
        
        default:                     return "";
    }
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
    // Column 1: Lexeme (20 chars wide, left-aligned)
    char lexeme[128] = {0};
    
    // Special handling for structural tokens
    if (token.type == TOKEN_NEWLINE) {
        snprintf(lexeme, sizeof(lexeme), "\\n");
    } else if (token.type == TOKEN_INDENT) {
        snprintf(lexeme, sizeof(lexeme), ">>INDENT");
    } else if (token.type == TOKEN_DEDENT) {
        snprintf(lexeme, sizeof(lexeme), "<<DEDENT");
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
    
    // Generate output filename
    char* outputPath = generateOutputFilename(sourcePath);
    if (outputPath == NULL) {
        return 1;
    }
    
    // Read source file
    char* source = readFile(sourcePath);
    if (source == NULL) {
        free(outputPath);
        return 1;
    }
    
    // Initialize lexer
    Lexer* lexer = initLexer(source);
    if (lexer == NULL) {
        fprintf(stderr, "Error: Failed to initialize lexer.\n");
        free(source);
        free(outputPath);
        return 1;
    }
    
    // Open output file
    FILE* outFile = fopen(outputPath, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error: Could not create output file '%s'.\n", outputPath);
        freeLexer(lexer);
        free(source);
        free(outputPath);
        return 1;
    }
    
    // Write header
    fprintf(outFile, "Lexeme              Token               Token Special\n");
    fprintf(outFile, "====================================================================\n");
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
    free(outputPath);
    
    return hasErrors ? 1 : 0;
}