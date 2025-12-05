# **SYNTAX ANALYZER DOCUMENTATION**

# **EaC Programming Language**

## **I. INTRODUCTION**

This document provides the complete syntax analysis specification for the EaC (ēzē) programming language. EaC is a modern, general-purpose, procedural programming language designed for simplicity, readability, and ease of use. It is a dynamically-typed, Python-inspired language with C-based operators that uses significant indentation for block structure.

The syntax analyzer (parser) for EaC validates the grammatical structure of source code by checking whether token sequences conform to the language's production rules. This document defines the formal grammar rules, provides examples for each syntactic construct, and demonstrates correctness verification through derivations and parse trees.

## **II. SYNTACTIC ELEMENTS**

The EaC language comprises the following major syntactic categories:

1. **Declaration Statements** \- Variable and constant declarations  
2. **Assignment Statements** \- Value assignment to variables  
3. **Input Statements** \- Console input operation  
4. **Output Statements** \- Console output operation  
5. **Conditional Statements** \- Control flow based on conditions  
6. **Iterative Statements** \- Loop constructs  
7. **Expressions** \- Arithmetic, relational, and logical expressions

## **III. GRAMMAR RULES (BNF)**

Formal Grammar Definition: **G \= (V, T, P, S)**

*Where:*

* **V** \= Set of non-terminal symbols  
* **T** \= Set of terminal symbols (tokens)  
* **P** \= Set of production rules  
* **S** \= Start symbol

**Start Production Rule**  
\<PROGRAM\> \=\> \<STATEMENTS\>

\<STATEMENTS\> \=\> \<STATEMENT\>   
           | \<STATEMENT\> \<NEWLINE\> \<STATEMENTS\>

\<STATEMENT\> \=\> \<DECL\_STMT\>  
          | \<ASS\_STMT\>  
          | \<INPUT\_STMT\>	  
          | \<OUTPUT\_STMT\>  
          | \<COND\_STMT\>  
          | \<ITER\_STMT\>

1. **Declaration Statement Rules**

\<DEC\_STMT\> \=\> \<VAR\_TYPE\> \<id\>  
         | \<VAR\_TYPE\> \<id\> \<ASSIGN\> \<expr\>  
         | VAR\_TYPE id TYPE\_HINT  
         | VAR\_TYPE id TYPE\_HINT ASSIGN expr

\<VAR\_TYPE\> \=\> flex | fixed  
\<TYPE\_HINT\> \=\> : \<DATA\_TYPE\>  
\<DATA\_TYPE\> \=\> int | float | str | bool | char  
\<ASSIGN\> \=\> \= | to

*Examples:*

1. `flex age` (VAR\_TYPE id)  
2. `fixed first_name` (VAR\_TYPE id)  
3. `flex age = 18` (VAR\_TYPE id ASSIGN expr)  
4. `fixed first_name = "Juan"` (VAR\_TYPE id ASSIGN expr)  
5. `flex age int = 18` (VAR\_TYPE id TYPE\_HINT ASSIGN expr)  
6. `flex is_student = true` (VAR\_TYPE id ASSIGN expr)

**2\. Assignment Statement Rules**

\<ASS\_STMT\> \=\> \<id\> \<ASSIGN\_OP\> \<expr\>  
\<ASSIGN\_OP\> \=\> \= | \+= | \-= | \*= | /= | %=  
\<expr\> \=\> \<ARITH\_EXPR\> | \<BOOL\_EXPR\> |\< LITERAL\> | \<id\>

*Examples:*

1. `age = 19` (id ASSIGN\_OP expr)  
2. `age_in_months = age * 12` (id ASSIGN\_OP expr)  
3. `count += 1` (id ASSIGN\_OP expr)  
4. `result as float = x / y` (id ASSIGN\_OP expr NOISE\_WORD)

**3\. Input Statement Rules**

\<INPUT\_STMT\> \=\> \<id\> \= input ( )  
           | \<id\>= input ( \<STRING\_LITERAL\> )

*Examples:*

1. `name = input()` (id \= input ( ))  
2. `age = input("Enter your age: ")` (id \= input ( STRING\_LITERAL ))  
3. `user_input = input()` (id \= input ( ))

**4\. Output Statement Rules**

\<OUTPUT\_STMT\> \=\> output ( OUTPUT\_ARGS )  
\<OUTPUT\_ARGS\> \=\> \<expr\>  
            | \<expr\>, \<OUTPUT\_ARGS\>  
            | \<STRING\_LITERAL\>

*Examples:*

1. `output("Hello, World!")` (output ( STRING\_LITERAL ))  
2. `output(age)` (output ( id ))  
3. `output(a)` (output ( expr ))

**5\. Conditional Statement Rules**

\<COND\_STMT\> \=\> \<WHEN\_STMT\>  
          | \<WHEN\_ELSE\_STMT\>  
          | \<WHEN\_ELIF\_STMT\>  
\<WHEN\_STMT\> \=\> when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\>  
\<DEDENT\>

\<WHEN\_ELSE\_STMT\> \=\> when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\>   
\<DEDENT\>  
                  else : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>

\<WHEN\_ELWHEN\_STMT\> \=\>   
when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
                  \<ELWHEN\_CHAIN\>  
else : \<NEWLINE\> INDENT STATEMENTS DEDENT

\<ELWHEN\_CHAIN\> \=\>   
else when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
           | else when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\> \<ELIF\_CHAIN\>

\<CONDITION\> \=\> \<BOOL\_EXPR\>  
          | ( \<BOOL\_EXPR\> )

\<BOOL\_EXPR\> \=\> \<RELATIONAL\_EXPR\>  
          | \<LOGICAL\_EXPR\>  
          | not \<BOOL\_EXPR\>

*Examples:*

A. **Simple when statement:**

when a \> 5:  
    output(a)

(when CONDITION : STATEMENTS)

B. **when-else statement:**

when (gwa \>= 1.00) and (gwa \<= 3.00):  
    output("Passing Grade\!")  
else:  
    output("Failing Grade\!")

(when CONDITION : STATEMENTS else : STATEMENTS)

C. **when-else when-else statement:**

when grade \>= 90:  
    output("A")  
else when grade \>= 80:  
    output("B")  
else when grade \>= 70:  
    output("C")  
else:  
    output("D or F")  
(when CONDITION : STATEMENTS ELWHEN\_CHAIN else : STATEMENTS)

**6\. Iterative Statement Rules**

\<ITER\_STMT\> \=\> \<WHILE\_STMT\> | \<FOR\_STMT\>

\<WHILE\_STMT\> \=\> while \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\>   
\<DEDENT\>

\<FOR\_STMT\> \=\> for \<id\> in \<ITERABLE\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\>   
\<DEDENT\>  
         | for each \<id\> in \<ITERABLE\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>

\<ITERABLE\> \=\> \<id\> | range ( \<expr\> ) | \[ \<LIST\_ITEMS\> \]

\<LIST\_ITEMS\> \=\> \<expr\> | \<expr\> , \<LIST\_ITEMS\>

*Examples:*

A. **While Loop:**

flex count \= 0  
while count \< 5:  
    output(count)  
    count \= count \+ 1

(while CONDITION : STATEMENTS)

B. **For Loop:**

for i in range(5):  
    output(i)

(for id in ITERABLE : STATEMENTS)

C. **For Loop with 'each' noise word:**

for each item in list:  
    output(item)

(for each id in ITERABLE : STATEMENTS)

7**. Expression Rules**

**Arithmetic Expressions**

\<ARITH\_EXPR\> \=\> \<TERM\>  
           | \<ARITH\_EXPR\> \+ \<TERM\>  
           | \<ARITH\_EXPR\> \- \<TERM\>

\<TERM\> \=\> \<FACTOR\>  
     | \<TERM\> \* \<FACTOR\>  
     | \<TERM\> / \<FACTOR\>  
     | \<TERM\> % \<FACTOR\>  
     | \<TERM\> // \<FACTOR\>

\<FACTOR\> \=\> \<POWER\>  
       | \- \<FACTOR\>

\<POWER\> \=\> \<PRIMARY\>  
      | \<PRIMARY\> ^ \<POWER\>

\<PRIMARY\> \=\> \<LITERAL\>  
        | \<id\>  
        | ( \<ARITH\_EXPR\> )  
        | | \<ARITH\_EXPR\> |

*Examples:*

1. `age * 12` (TERM)  
2. `x + y` (ARITH\_EXPR \+ TERM)  
3. `result as float = x / y` (TERM / FACTOR)  
4. `2 ^ 3` (PRIMARY ^ POWER)

**Relational and Logical Expressions**

\<RELATIONAL\_EXPR\> \=\> \<ARITH\_EXPR\> \<REL\_OP\> \<ARITH\_EXPR\>

\<REL\_OP\> \=\> \< | \> | \<= | \>= | \== | \!= 

\<LOGICAL\_EXPR\> \=\> \<BOOL\_TERM\>  
             | \<LOGICAL\_EXPR\> or \<BOOL\_TERM\>

\<BOOL\_TERM\> \=\> \<BOOL\_FACTOR\>  
          | \<BOOL\_TERM\> and \<BOOL\_FACTOR\>

\<BOOL\_FACTOR\> \=\> \<RELATIONAL\_EXPR\>  
            | not \<BOOL\_FACTOR\>  
            | ( \<LOGICAL\_EXPR\> )  
            | true | false

**Examples:**

1. `a > 5` (ARITH\_EXPR REL\_OP ARITH\_EXPR)  
2. `(gwa >= 1.00) and (gwa <= 3.00)` (BOOL\_TERM and BOOL\_FACTOR)  
3. `grade >= 90` (RELATIONAL\_EXPR)  
4. `count < 5` (RELATIONAL\_EXPR)

**8\. Literal Rules**  
\<LITERAL\> \=\> \<NUMBER\> | \<STRING\_LITERAL\> | \<BOOLEAN\>  
\<NUMBER\> \=\> \<INTEGER\> | \<FLOAT\>  
\<INTEGER\> \=\> \<DIGIT+\>  
\<FLOAT\> \=\> \<DIGIT+\> . \<DIGIT+\>  
\<STRING\_LITERAL\> \=\> " \<CHAR\*\> " | ' \<CHAR\*\> '  
\<BOOLEAN\> \=\> true | false  
\<DIGIT\> \=\> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

**9. Import Statement Rules**

\<IMPORT\_STMT\> \=\> import \<id\>
             | from \<id\> import \<id\>

*Examples:*

1. `import math` (import id)
2. `from math import pi` (from id import id)

**10. Function Declaration Rules**

\<FUNCTION\_STMT\> \=\> function \<id\> ( \<PARAM\_LIST\> ) \<RET\_TYPE\_HINT\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>

\<PARAM\_LIST\> \=\> \<PARAM\>
            | \<PARAM\> , \<PARAM\_LIST\>
            | ε

\<PARAM\> \=\> \<id\> : \<DATA\_TYPE\>

\<RET\_TYPE\_HINT\> \=\> : \<DATA\_TYPE\>
               | ε

*Examples:*

1. `function add(a: int, b: int): int:`
2. `function greet(name: str):`

**11. Return Statement Rules**

\<RETURN\_STMT\> \=\> return \<expr\>
             | return

*Examples:*

1. `return result`
2. `return`

**12. Break Statement Rules**

\<BREAK\_STMT\> \=\> break

*Examples:*

1. `break`

## **IV. DERIVATION EXAMPLES**

*Example 1:* **Declaration Statement**

**Source code:** Flex age \= 18

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> DEC\_STMT  
\=\> VAR\_TYPE id ASSIGN expr  
\=\> flex id ASSIGN expr  
\=\> flex age ASSIGN expr  
\=\> flex age \= expr  
\=\> flex age \= ARITH\_EXPR  
\=\> flex age \= PRIMARY  
\=\> flex age \= LITERAL  
\=\> flex age \= NUMBER  
\=\> flex age \= INTEGER  
\=\> flex age \= 18

*Example 2:* **Assignment Statement**

**Source code:** count \+= 1

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> ASS\_STMT

\=\> id ASSIGN\_OP expr  
\=\> count \+= expr  
\=\> count \+= ARITH\_EXPR  
\=\> count \+= TERM  
\=\> count \+= FACTOR  
\=\> count \+= POWER  
\=\> count \+= PRIMARY  
\=\> count \+= LITERAL  
\=\> count \+= NUMBER  
\=\> count \+= INTEGER  
\=\> count \+= 1

*Example 3:* **Input Statement**

**Source code:** name \= input(“Enter: “)

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> INPUT\_STMT  
\=\> id \= input (STRING\_LITERAL)  
\=\> name \= input (STRING\_LITERAL)   
\=\> name \= input (“Enter: “)

*Example 4:* **Output Statement**

**Source code:** output(a)

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> OUTPUT\_STMT  
\=\> output(OUTPUT\_ARGS)  
\=\> output(expr)  
\=\> output(id)  
\=\> output(a)

*Example 5:* **Conditional Statement**

**Source Code:**

when a \> 5:  
    output(a)

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> COND\_STMT  
\=\> WHEN\_STMT  
\=\> when CONDITION : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when BOOL\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when RELATIONAL\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when ARITH\_EXPR REL\_OP ARITH\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when id REL\_OP ARITH\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a REL\_OP ARITH\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a \> ARITH\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a \> PRIMARY : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a \> LITERAL : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a \> 5 : NEWLINE INDENT STATEMENTS DEDENT  
\=\> when a \> 5 : NEWLINE INDENT STATEMENT DEDENT  
\=\> when a \> 5 : NEWLINE INDENT OUTPUT\_STMT DEDENT  
\=\> when a \> 5 : NEWLINE INDENT output ( OUTPUT\_ARGS ) DEDENT  
\=\> when a \> 5 : NEWLINE INDENT output ( expr ) DEDENT  
\=\> when a \> 5 : NEWLINE INDENT output ( id ) DEDENT  
\=\> when a \> 5 : NEWLINE INDENT output ( a ) DEDENT

*Example 6:* **While Loop**

**Source Code:**

while count \< 5:  
    output(count)

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT

\=\> ITER\_STMT  
\=\> WHILE\_STMT  
\=\> while CONDITION : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while BOOL\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while RELATIONAL\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while ARITH\_EXPR REL\_OP ARITH\_EXPR : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while id \< LITERAL : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while count \< 5 : NEWLINE INDENT STATEMENTS DEDENT  
\=\> while count \< 5 : NEWLINE INDENT STATEMENT DEDENT  
\=\> while count \< 5 : NEWLINE INDENT OUTPUT\_STMT DEDENT  
\=\> while count \< 5 : NEWLINE INDENT output ( id ) DEDENT  
\=\> while count \< 5 : NEWLINE INDENT output ( count ) DEDENT

*Example 7:* **For Loop**

**Source Code:**

for i in range(3):  
    output(i)

Leftmost Derivation:

PROGRAM  
\=\> STATEMENTS  
\=\> STATEMENT  
\=\> FOR\_STMT  
\=\> for id in ITERABLE : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in ITERABLE : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (expr) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (ARITH\_EXPR) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (TERM) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (PRIMARY) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (LITERAL) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (3) : NEWLINE INDENT STATEMENTS DEDENT  
\=\> for i in range (3) : NEWLINE INDENT STATEMENT DEDENT  
\=\> for i in range (3) : NEWLINE INDENT OUTPUT\_STMT DEDENT  
\=\> for i in range (3) : NEWLINE INDENT output (id) DEDENT  
\=\> for i in range (3) : NEWLINE INDENT output (i) DEDENT

## **V. INPUT PROCESSING**

**Input Method:** Tokens are read one by one from the symbol table (one token at a time).

The parser receives a stream of tokens from the lexical analyzer, processing them sequentially. Each token contains:

* Token type (keyword, identifier, operator, literal, etc.)  
* Lexeme (actual text)  
* Line and column information for error reporting

The parser maintains a current token pointer and advances through the token stream as it matches production rules.

## **VI. PARSING ALGORITHM IMPLEMENTATION**

**Algorithm:** Recursive Descent Parsing

Recursive Descent Parsing is a top-down parsing technique that uses a set of recursive procedures to process the input. Each non-terminal in the grammar has a corresponding parsing procedure.

Key Characteristics:

1. **One token lookahead** \- Parser examines the current token to decide which production to apply  
2. **Predictive** \- No backtracking required for LL(1) grammars  
3. **Easy to implement** \- Direct correspondence between grammar rules and code  
4. **Handles left-factored grammars** \- Requires grammar transformation for left recursion

Parsing Approach:

* Input is read one token at a time from the symbol table  
* Each parsing function corresponds to a non-terminal  
* Functions call each other recursively based on production rules  
* Error recovery is handled through synchronization tokens

## 

## **VI. TRANSITION DIAGRAM**

The parser uses state transitions to recognize syntactic patterns. Key transition diagrams include:

Declaration Statement Transition:  
![][image1]

Assignment Statement Transition:

Conditional Statement Transition:  
![][image2]

While Loop Transition:

## **![][image3]**

For Loop Transition:  
![][image4]

Input Statement Transition:  
![][image5]  
Output Statement Transition:  
![][image6]  
Expression Statement:  
![][image7]

## 

## **VII. ERROR RECOVERY**

**Method Used:** Panic Mode with Synchronization Tokens

Error Recovery Strategy:

1. **Detection** \- When parser encounters unexpected token  
2. **Reporting** \- Generate descriptive error message with line/column  
3. **Recovery** \- Skip tokens until synchronization point is found  
4. **Continuation** \- Resume parsing from synchronized state

Synchronization Tokens:

* `NEWLINE` \- Statement boundary  
* `DEDENT` \- Block end  
* Keywords: `flex`, `fixed`, `when`, `while`, `for`, `else`, `output`  
* Delimiters: `)`, `]`, `:`

Error Types Handled:

* Missing tokens (e.g., missing `:` after `when`)  
* Unexpected tokens (e.g., invalid operator)  
* Indentation errors (Python-style significant whitespace)  
* Mismatched delimiters  
* Invalid expressions

## **VIII. ERROR MESSAGES**

Sample Error Messages:  
**Missing Colon:**

 Error at line 5, column 15: Expected ':' after condition in 'when' statement  
when a \> 5  
          ^

1. 

**Invalid Indentation:**

 Error at line 8: Indentation error \- expected indent after 'when' statement  
when a \> 5:  
output(a)  
^

2. 

**Undeclared Variable:**

 Error at line 12: Assignment to undeclared variable 'count'  
count \= 0  
^^^^^

3. 

**Unexpected Token:**

 Error at line 6, column 10: Unexpected token ';' \- statements end with newline  
flex age \= 18;  
             ^

4. 

**Missing Expression:**

 Error at line 9: Expected expression after '=' in assignment statement  
age \=  
    ^

5. 

**Invalid Type Hint:**

 Error at line 3: Invalid type hint 'string' \- use 'str'  
flex name: string \= "Juan"  
           ^^^^^^

6. 

## **IX. PARSING TABLE (First and Follow Sets)**

First Sets:  
FIRST(PROGRAM) \= {flex, fixed, when, while, for, output, id, \#, /\*}  
FIRST(DEC\_STMT) \= {flex, fixed}  
FIRST(ASS\_STMT) \= {id}  
FIRST(OUTPUT\_STMT) \= {output}  
FIRST(COND\_STMT) \= {when}  
FIRST(ITER\_STMT) \= {while, for}  
FIRST(expr) \= {id, NUMBER, STRING, (, \-, not, true, false}

Follow Sets:  
FOLLOW(PROGRAM) \= {$}  
FOLLOW(STATEMENT) \= {NEWLINE, DEDENT, $}  
FOLLOW(expr) \= {NEWLINE, ), ,, :, \=, \+, \-, \*, /, and, or}  
FOLLOW(CONDITION) \= {:}

**Numbered Production Rules**

1. \<PROGRAM\> → \<STATEMENTS\>

Statement and Newline helper

2. \<STATEMENTS\> → \<STATEMENT\>  
3. \<STATEMENTS\> → \<STATEMENT\>\<NEWLINE\> \<STATEMENTS\>

Types of Statement

4. \<STATEMENT\> → \<DECL\_STMT\>  
5. \<STATEMENT\> → \<ASS\_STMT\>  
6. \<STATEMENT\> → \<INPUT\_STMT\>  
7. \<STATEMENT\> → \<OUTPUT\_STMT\>  
8. \<STATEMENT\> → \<COND\_STMT\>  
9. \<STATEMENT\> → \<ITER\_STMT\>

Declaration Statement

10. \<DECL\_STMT\> → \<VAR\_TYPE\> \<id\>  
11. \<DECL\_STMT\> → \<VAR\_TYPE\> \<id\> \<ASSIGN\> \<expr\>  
12. \<DECL\_STMT\> → \<VAR\_TYPE\> \<id\> \<TYPE\_HINT\>  
13. \<DECL\_STMT\> → \<VAR\_TYPE\> \<id\> \<TYPE\_HINT\> \<ASSIGN\> \<expr\>

Variable Types

14. \<VAR\_TYPE\> → flex  
15. \<VAR\_TYPE\> → fixed

Hint

16. \<TYPE\_HINT\> → : \<DATA\_TYPE\>

Data Types

17. \<DATA\_TYPE\> → int  
18. \<DATA\_TYPE\> → float  
19. \<DATA\_TYPE\> → str  
20. \<DATA\_TYPE\> → bool  
21. \<DATA\_TYPE\> → char

Assign

22. \<ASSIGN\> → \=  
23. \<ASSIGN\> → to

Assignment Statement

24. \<ASS\_STMT\> → \<id\> \<ASSIGN\_OP\> \<expr\>

Assignment Operations

25. \<ASSIGN\_OP\> → \=  
26. \<ASSIGN\_OP\> → \+=  
27. \<ASSIGN\_OP\> → \-=  
28. \<ASSIGN\_OP\> → \*=  
29. \<ASSIGN\_OP\> → /=  
30. \<ASSIGN\_OP\> → %=

Expressions

31. \<expr\> → \<ARITH\_EXPR\>  
32. \<expr\> → \<BOOL\_EXPR\>  
33. \<expr\> → \<LITERAL\>  
34. \<expr\> → \<id\>

Input Statements

35. \<INPUT\_STMT\> → \<id\> \= input ( )  
36. \<INPUT\_STMT\> → \<id\> \= input ( \<STRING\_LITERAL\> )

Output Statement

37. \<OUTPUT\_STMT\> → output ( \<OUTPUT\_ARGS\> )

Output Arguments

38. \<OUTPUT\_ARGS\> → \<expr\>  
39. \<OUTPUT\_ARGS\> → \<expr\> , \<OUTPUT\_ARGS\>  
40. \<OUTPUT\_ARGS\> → \<STRING\_LITERAL\>

Conditional Statements

41. \<COND\_STMT\> → \<WHEN\_STMT\>  
42. \<COND\_STMT\> → \<WHEN\_ELSE\_STMT\>  
43. \<COND\_STMT\> → \<WHEN\_ELWHEN\_STMT\>  
44. \<WHEN\_STMT\> → when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
45. \<WHEN\_ELSE\_STMT\> → when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
46.                         else : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
47. \<WHEN\_ELWHEN\_STMT\> → when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
48.                          \<ELWHEN\_CHAIN\>  
49.                          else : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
50. \<ELWHEN\_CHAIN\> → else when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
51. \<ELWHEN\_CHAIN\> → else when \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\> \<ELWHEN\_CHAIN\>

Conditional Logic

52. \<CONDITION\> → \<BOOL\_EXPR\>  
53. \<CONDITION\> → ( \<BOOL\_EXPR\> )  
54.   
55. \<BOOL\_EXPR\> → \<RELATIONAL\_EXPR\>  
56. \<BOOL\_EXPR\> → \<LOGICAL\_EXPR\>  
57. \<BOOL\_EXPR\> → not \<BOOL\_EXPR\>

Iterative Statement

58. \<ITER\_STMT\> → \<WHILE\_STMT\>  
59. \<ITER\_STMT\> → \<FOR\_STMT\>  
60. \<WHILE\_STMT\> → while \<CONDITION\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
61. \<FOR\_STMT\> → for \<id\> in \<ITERABLE\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>  
62. \<FOR\_STMT\> → for each \<id\> in \<ITERABLE\> : \<NEWLINE\> \<INDENT\> \<STATEMENTS\> \<DEDENT\>

63. \<ITERABLE\> → \<id\>  
64. \<ITERABLE\> → range ( \<expr\> )  
65. \<ITERABLE\> → \[ \<LIST\_ITEMS\> \]

66. \<LIST\_ITEMS\> → \<expr\>  
67. \<LIST\_ITEMS\> → \<expr\> , \<LIST\_ITEMS\>

68. \<ARITH\_EXPR\> → \<TERM\>  
69. \<ARITH\_EXPR\> → \<ARITH\_EXPR\> \+ \<TERM\>  
70. \<ARITH\_EXPR\> → \<ARITH\_EXPR\> \- \<TERM\>

71. \<TERM\> → \<FACTOR\>  
72. \<TERM\> → \<TERM\> \* \<FACTOR\>  
73. \<TERM\> → \<TERM\> / \<FACTOR\>  
74. \<TERM\> → \<TERM\> % \<FACTOR\>  
75. \<TERM\> → \<TERM\> // \<FACTOR\>

76. \<FACTOR\> → \<POWER\>  
77. \<FACTOR\> → \- \<FACTOR\>

78. \<POWER\> → \<PRIMARY\>  
79. \<POWER\> → \<PRIMARY\> ^ \<POWER\>

80. \<PRIMARY\> → \<LITERAL\>  
81. \<PRIMARY\> → \<id\>  
82. \<PRIMARY\> → ( \<ARITH\_EXPR\> )  
83. \<PRIMARY\> → | \<ARITH\_EXPR\> |

84. \<RELATIONAL\_EXPR\> → \<ARITH\_EXPR\> \<REL\_OP\> \<ARITH\_EXPR\>

85. \<REL\_OP\> →   
86. \<REL\_OP\> → \>  
87. \<REL\_OP\> → \<=  
88. \<REL\_OP\> → \>=  
89. 85\. \<REL\_OP\> → \==  
90. 86\. \<REL\_OP\> → \!=  
      
91. \<LOGICAL\_EXPR\> → \<BOOL\_TERM\>  
92. \<LOGICAL\_EXPR\> → \<LOGICAL\_EXPR\> or \<BOOL\_TERM\>  
      
93. 8\<BOOL\_TERM\> → \<BOOL\_FACTOR\>  
94. \<BOOL\_TERM\> → \<BOOL\_TERM\> and \<BOOL\_FACTOR\>  
      
95. \<BOOL\_FACTOR\> → \<RELATIONAL\_EXPR\>  
96. \<BOOL\_FACTOR\> → not \<BOOL\_FACTOR\>  
97. \<BOOL\_FACTOR\> → ( \<LOGICAL\_EXPR\> )  
98. \<BOOL\_FACTOR\> → true  
99. \<BOOL\_FACTOR\> → false  
      
100. \<LITERAL\> → \<NUMBER\>  
101. \<LITERAL\> → \<STRING\_LITERAL\>  
102. \<LITERAL\> → \<BOOLEAN\>  
       
103. \<NUMBER\> → \<INTEGER\>  
104. \<NUMBER\> → \<FLOAT\>  
       
105. \<INTEGER\> → \<DIGIT+\>  
106. \<FLOAT\> → \<DIGIT+\> . \<DIGIT+\>  
       
107. \<STRING\_LITERAL\> → " \<CHAR\*\> "  
108. \<STRING\_LITERAL\> → ' \<CHAR\*\> '

Boolean Operations

109. \<BOOLEAN\> → true  
110. \<BOOLEAN\> → false

Digits

111. \<DIGIT\> → 0  
112. \<DIGIT\> → 1  
113. \<DIGIT\> → 2  
114. \<DIGIT\> → 3  
115. \<DIGIT\> → 4  
116. \<DIGIT\> → 5  
117. \<DIGIT\> → 6  
118. \<DIGIT\> → 7  
119. \<DIGIT\> → 8  
120. \<DIGIT\> → 9

First and Follow Table

| Non-Terminal | First | Follow |
| :---- | :---- | :---- |
| \<PROGRAM\> | { flex, fixed, id, when, while, for, output } | { $ } |
| \<STATEMENTS\> | { flex, fixed, id, when, while, for, output, λ } | { DEDENT, $} |
| \<STATEMENT\> | { flex, fixed, id, when, while, for, output } | { NEWLINE, DEDENT, $} |
| \<DECL\_STMT\> | { flex, fixed } | { NEWLINE, DEDENT, $} |
| \<ASS\_STMT\> | { id } | { NEWLINE, DEDENT, $} |
| \<INPUT\_STMT\> | { id } | { NEWLINE, DEDENT, $} |
| \<OUTPUT\_STMT\> | { output } | { NEWLINE, DEDENT, $} |
| \<COND\_STMT\> | { when } | { NEWLINE, DEDENT, $} |
| \<ITER\_STMT\> | { while, for } | { NEWLINE, DEDENT, $} |
| \<VAR\_TYPE\> | { flex, fixed } | { id } |
| \<id\> | –  | { \=, to, \+=, \-=, \*=, /=, %=, :, NEWLINE, ), ,, \+, \-, \*, /, %, //, ^, and, or, \<, \>, \<=, \>=, \==, \!=, in, DEDENT, $ } |
| \<TYPE\_HINT\> | { : } | { \=, to, NEWLINE, DEDENT, $ } |
| \<DATA\_TYPE\> | { int, float, str, bool, char } | { \=, to, NEWLINE, DEDENT, $ } |
| \<ASSIGN\> | { \=, to } | { id, NUMBER, STRING, (, \-, not, true, false, | } |
| \<ASSIGN\_OP\> | { \=, \+=, \-=, \*=, /=, %= } | { id, NUMBER, STRING, (, \-, not, true, false, | } |
| \<expr\> | { id, NUMBER, STRING, (, \-, not, true, false, | } | { NEWLINE, ), ,, :, \+, \-, \*, /, %, //, ^, and, or, \<, \>, \<=, \>=, \==, \!=, DEDENT, $ } |
| \<CONDITION\> | { id, NUMBER, (, \-, not, true, false, | } | { : } |
| \<BOOL\_EXPR\> | { id, NUMBER, (, \-, not, true, false, | } | { :, ), and, or } |
| ,ARITH\_EXPR\> | { id, NUMBER, (, \-, | } | { NEWLINE, ), ,, :, \+, \-, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<TERM\> | { id, NUMBER, (, \-, | } | { NEWLINE, ), ,, :, \+, \-, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<FACTOR\> | { id, NUMBER, (, \-, | } | { NEWLINE, ), ,, :, \+, \-, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<POWER\> | { id, NUMBER, (, | } | { NEWLINE, ), ,, :, \+, \-, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<PRIMARY\> | { id, NUMBER, (, | } | { NEWLINE, ), ,, :, \+, \-, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<RELATIONAL\_EXPR\> | { id, NUMBER, (, \-, | } | { :, ), and, or } |
| \<REL\_OP\> | { \<, \>, \<=, \>=, \==, \!= } | { id, NUMBER, (, \-, | } |
| \<LOGICAL\_EXPR\> | { id, NUMBER, (, \-, not, true, false, | } | { :, ), or } |
| \<BOOL\_TERM\> | { id, NUMBER, (, \-, not, true, false, | } | { :, ), or, and } |
| \<BOOL\_FACTOR\> | { id, NUMBER, (, \-, not, true, false, | } | { :, ), and, or } |
| \<LITERAL\> | { NUMBER, STRING, true, false } | { NEWLINE, ), ,, :, \+, \-, \*, /, %, //, ^, \<, \>, \<=, \>=, \==, \!=, and, or, |, DEDENT, $ } |
| \<OUTPUT\_ARGS\> | – | { ) } |
| \< ITERABLES \> | – | { : } |
| \< LIST\_ITEMS \> | – | { \], , } |
| \<NUMBER\> | { DIGIT } | – |
| \<INTEGER\> | { DIGIT } | – |
| \<FLOAT\> | { DIGIT } | – |
| \<STRING\_LITERAL\> | { “, ‘ } | – |
| \<BOOLEAN\> | { true, false } | – |
| \<DIGIT\> | { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } | – |

