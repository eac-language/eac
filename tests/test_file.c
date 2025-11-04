/* This is a C file (.c) - NOT an EaC file */
/* The lexical analyzer should ONLY accept .eac files */
/* This file tests that non-.eac files are rejected */

#include <stdio.h>

int main() {
    printf("This is C, not EaC\n");
    return 0;
}

