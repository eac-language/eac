## **EaC (easy) Programming Language Specification**

This document provides the official specification for the EaC programming language. It details the language's philosophy and its core syntactic elements, from the basic character set to the structure of complex statements.

### **I. Introduction** 

#### **Language Description**

**EaC (easy)** is a modern, general-purpose, procedural programming language designed for simplicity, readability, and ease of use. It is a **dynamically-typed** language, meaning variable types are inferred at runtime, not through explicit mandatory declarations.

#### **Inspiration**

EaC is **C-based but Python-inspired**. It retains the robust and familiar operator set from C (e.g., \+, \*, \==) but adopts the clean, minimal syntax and developer-friendly features of Python. This blend aims to create a language that is easy for beginners to learn while being powerful enough for complex tasks.

#### **Core Functionality**

EaC provides developers with a streamlined set of tools for building applications, including:

* **Dynamic Variables:** Flexible variable creation using the flex keyword.  
* **Constants:** Immutable variable creation using the fixed keyword.  
* **Pythonic Control Flow:** Clean conditional logic using when and else, with indentation defining code blocks.  
* **Simple Output:** A built-in output() function for printing to the console.  
* **Familiar Operators:** A comprehensive set of arithmetic, relational, and logical operators based on C and Python.

### **II. Syntactic Elements of Language** 

This section defines the fundamental lexical and grammatical rules of EaC.

#### **1\. Character Set**

The character set includes all valid symbols recognized by the EaC compiler.

* **Alphabet (ALPHA)**:  
  * **Uppercase (CAP)**: ABCDEFGHIJKLMNOPQRSTUVWXYZ  
  * **Lowercase (SM)**: abcdefghijklmnopqrstuvwxyz  
* **Numerical Digits (DIGITS)**:  
  * 0123456789  
* **Symbols (SYMBOLS)**:  
  * **Operators**: \+, \-, \*, /, %, ^, |, \=, \<, \>  
  * **Grouping**: (, ), \[, \]  
  * **Punctuation**: ,, :, .  
  * **Miscellaneous**: \_, \#, "

#### **2\. Identifiers**

Identifiers are names used to reference variables and constants.

* **Rules for Naming**:  
    
  1. An identifier must start with a letter (a-z, A-Z) or an underscore (\_).  
  2. Subsequent characters can be letters, digits (0-9), or an underscore.  
  3. Identifiers are **case-sensitive** (my\_var is different from My\_Var).  
  4. Keywords cannot be used as identifiers.


* **Regular Expression**: \[a-zA-Z\_\]\[a-zA-Z0-9\_\]\*  
    
* **Machine**: An identifier is recognized by a state machine where:  
    
  * q0 (start state) transitions to q1 (accepting state) on any letter (\[a-zA-Z\]) or an underscore (\_).  
  * q1 (accepting state) transitions back to itself (q1) on any letter, digit (\[0-9\]), or underscore.


* **Recommended Style Guide**: Use snake\_case for all variable and constant names (e.g., my\_variable, FIXED\_RATE).

#### **3\. Operation Symbols**

EaC uses familiar operators for expressions.

* **Arithmetic Operators**   
    
  * \+ (Addition)  
    * **Machine**: q0 → \+ → q1 (final)  
  * \- (Subtraction)  
    * **Machine**: q0 → \- → q1 (final)  
  * \* (Multiplication)  
    * **Machine**: q0 → \* → q1 (final)  
  * / (Division)  
    * **Machine**: q0 → / → q1 (final)  
  * % (Modulo)  
    * **Machine**: q0 → % → q1 (final)  
  * ^ (Exponent)  
    * **Machine**: q0 → ^ → q1 (final)  
  * | (Absolute Value)  
    * **Machine**: q0 → | → q1 (final)


* **Relational & Equality Operators**   
    
  * \< (Less than)  
    * **Machine**: q0 → \< → q1 (final)  
  * \> (Greater than)  
    * **Machine**: q0 → \> → q1 (final)  
  * \== (Equal)  
    * **Machine**: q0 → \= → q1 → \= → q2 (final)  
  * \<= (Less than or Equal)  
    * **Machine**: q0 → \< → q1 → \= → q2 (final)  
  * \>= (Greater than or Equal)  
    * **Machine**: q0 → \> → q1 → \= → q2 (final)  
  * \!= (Not Equal)  
    * **Machine**: q0 → \! → q1 → \= → q2 (final)


* **Logical Operators**   
    
  * and (Logical AND)  
    * **Machine**: q0 → a → q1 → n → q2 → d → q3 (final)  
  * or (Logical OR)  
    * **Machine**: q0 → o → q1 → r → q2 (final)  
  * not (Logical NOT)  
    * **Machine**: q0 → n → q1 → o → q2 → t → q3 (final)


* **Assignment Operators**  
    
  * \= (Assignment)  
    * **Machine**: q0 → \= → q1 (final)  
  * \+= (Add and assign)  
    * **Machine**: q0 → \+ → q1 → \= → q2 (final)  
  * \-= (Subtract and assign)  
    * **Machine**: q0 → \- → q1 → \= → q2 (final)  
  * \*= (Multiply and assign)  
    * **Machine**: q0 → \* → q1 → \= → q2 (final)  
  * /= (Divide and assign)  
    * **Machine**: q0 → / → q1 → \= → q2 (final)  
  * %= (Modulo and assign)  
    * **Machine**: q0 → % → q1 → \= → q2 (final)

#### **4\. Keywords and Reserved Words**

Keywords are predefined words with special meanings.

* **4.a. Keywords**  
    
  * flex: Declares a flexible (mutable) variable.  
    * **Machine**: q0 → f → q1 → l → q2 → e → q3 → x → q4 (final)  
  * fixed: Declares a fixed (immutable) constant.  
    * **Machine**: q0 → f → q1 → i → q2 → x → q3 → e → q4 → d → q5 (final)  
  * when: Begins a conditional block (Python's if).  
    * **Machine**: q0 → w → q1 → h → q2 → e → q3 → n → q4 (final)  
  * else: Begins an alternate conditional block.  
    * **Machine**: q0 → e → q1 → l → q2 → s → q3 → e → q4 (final)  
  * output: Built-in function to print to the console.  
    * **Machine**: q0 → o → q1 → u → q2 → t → q3 → p → q4 → u → q5 → t → q6 (final)  
  * function: Declares a function block.  
    * **Machine**: q0 → f → q1 → u → q2 → n → q3 → c → q4 → t → q5 → i → q6 → o → q7 → n → q8 (final)  
  * import: Starts a module import declaration.  
    * **Machine**: q0 → i → q1 → m → q2 → p → q3 → o → q4 → r → q5 → t → q6 (final)  
  * from: Specifies the source in an import declaration.  
    * **Machine**: q0 → f → q1 → r → q2 → o → q3 → m → q4 (final)  
  * while: Begins a while-loop.  
    * **Machine**: q0 → w → q1 → h → q2 → i → q3 → l → q4 → e → q5 (final)  
  * for: Begins a for-loop (typically with in).  
    * **Machine**: q0 → f → q1 → o → q2 → r → q3 (final)  
  * in: Used in for loops for iteration.  
    * **Machine**: q0 → i → q1 → n → q2 (final)  
  * break: Exits the innermost loop.  
    * **Machine**: q0 → b → q1 → r → q2 → e → q3 → a → q4 → k → q5 (final)  
  * continue: Skips to the next iteration of the loop.  
    * **Machine**: q0 → c → q1 → o → q2 → n → q3 → t → q4 → i → q5 → n → q6 → u → q7 → e → q8 (final)  
  * return: Exits a function, optionally returning a value.  
    * **Machine**: q0 → r → q1 → e → q2 → t → q3 → u → q4 → r → q5 → n → q6 (final)  
  * true: Boolean true value.  
    * **Machine**: q0 → t → q1 → r → q2 → u → q3 → e → q4 (final)  
  * false: Boolean false value.  
    * **Machine**: q0 → f → q1 → a → q2 → l → q3 → s → q4 → e → q5 (final)  
  * and: Logical conjunction operator treated as a keyword.  
    * **Machine**: q0 → a → q1 → n → q2 → d → q3 (final)  
  * or: Logical disjunction operator treated as a keyword.  
    * **Machine**: q0 → o → q1 → r → q2 (final)  
  * not: Logical negation operator treated as a keyword.  
    * **Machine**: q0 → n → q1 → o → q2 → t → q3 (final)


* **4.b. Optional Type-Hint Keywords (Reserved Words)** These words are reserved for use as *optional* type hints in variable declarations. They do not affect the dynamic typing but improve code clarity.  
    
  * int:  
    * **Machine**: q0 → i → q1 → n → q2 → t → q3 (final)  
  * float:  
    * **Machine**: q0 → f → q1 → l → q2 → o → q3 → a → q4 → t → q5 (final)  
  * str:  
    * **Machine**: q0 → s → q1 → t → q2 → r → q3 (final)  
  * bool:  
    * **Machine**: q0 → b → q1 → o → q2 → o → q3 → l → q4 (final)
  * char:  
    * **Machine**: q0 → c → q1 → h → q2 → a → q3 → r → q4 (final)

#### **5. Noise Words**

EaC recognises a small set of “polite” noise words that are tokenised but have no effect on program semantics. These words are currently `please`, `kindly`, and `maybe`; they allow more conversational code without confusing the lexer.

#### **6\. Comments**

Comments are recognised by the lexer and emitted as tokens, allowing tooling to inspect or preserve them while later compilation phases may ignore them for execution.

* **Single-line comments**: Start with \# and extend to the end of the line.  
  * *Example*: flex x \= 10 \# This is a comment  
  * *Machine*: q0 → \# → q1 (final). State q1 loops on any character except a newline.  
* **Multi-line comments**: Start with /\* and end with \*/.  
  * *Example*: /\* This is a multi-line comment. \*/  
  * *Machine*: q0 → / → q1 → \* → q2. State q2 transitions to q3 on \*. State q3 transitions to the final state q4 on /. If q3 sees any char other than /, it returns to q2.

#### **7\. Blanks (Spaces)**

EaC is a Python-inspired language that uses **significant indentation**.

* **Indentation** (spaces or tabs at the beginning of a line) is used to define code blocks (e.g., inside a when statement or a while loop). Braces {} are not used.  
* **Whitespace** (spaces between tokens on the same line, e.g., x \= 10) is used as a delimiter and is ignored, allowing for flexible formatting.

#### **8\. Delimiters and Brackets**

* ( ) **Parentheses**: Used to group expressions and enclose function arguments.  
* \[ \] **Brackets**: Used for list/array declaration and element access.  
* : **Colon**: Used to terminate a statement that introduces a new indented code block (e.g., when a \> 5:, else:).  
* , **Comma**: Separates items in a list, such as function arguments.  
* **Newline**: A newline character terminates a statement. Semicolons (;) are **not** used.

#### **9\. Free- and Fixed-Field Formats**

EaC uses a **free-field** format. Statements are not required to be in specific columns. However, all statements must adhere to the **indentation rules** to define their block structure.

#### **10\. Expressions**

An expression is a combination of values, variables, and operators that evaluates to a single value. The order of operations (precedence) is as follows:

* **a. Mathematical/Arithmetic Expressions (Highest to Lowest)**  
    
  1. Literals (e.g., 18, "Juan") and Parentheses ()  
  2. Unary (e.g., a negative sign \-)  
  3. Power (Exponent ^)  
  4. Factor (Multiplication \*, Division /, Modulo %)  
  5. Term (Addition \+, Subtraction \-)  
  6. Absolute Value |


* **b. Boolean Expressions (Highest to Lowest)**  
    
  1. Unary (not)  
  2. Relational (\<, \>, \<=, \>=)  
  3. Equality (\==, \!=)  
  4. Conjunction (and)  
  5. Expression (or)

---

#### **11\. Statements**

Statements are the executable instructions of an EaC program.

* **Declaration Statement**:  
    
  \# Simple dynamic declaration  
    
  flex age  
    
  fixed first\_name  
    
  \# Declaration with initialization  
    
  flex age \= 18  
    
  fixed first\_name \= "Juan"  
    
  flex is\_student \= true  
    
  \# Declaration with optional type hint  
    
  flex age: int \= 18  
    
  fixed first\_name: str \= "Juan"  
    
* **Assignment Statement**:  
    
  age \= 19                \# Re-assigns the value of a 'flex' variable  
    
  age\_in\_months \= age \* 12  
    
* **Output Statement**:  
    
  output("Hello, World\!")  
    
  output(age)  
    
* **Conditional Statement**:  
    
  * **when (if)**:  
      
    when a \> 5:  
      
        output(a)  
      
  * **when-else (if-else)**:  
      
    when (gwa \>= 1.00) and (gwa \<= 3.00):  
      
        output("Passing Grade\!")  
      
    else:  
      
        output("Failing Grade\!")  
      
  * **when-else when-else (if-elif-else)**:  
      
    when grade \>= 90:  
      
        output("A")  
      
    else when grade \>= 80:  
      
        output("B")  
      
    else when grade \>= 70:  
      
        output("C")  
      
    else:  
      
        output("D or F")

    
* **Iterative Statements**:  
    
  * **While Loop**:  
      
    flex count \= 0  
      
    while count \< 5:  
      
        output(count)  
      
        count \= count \+ 1  \# or count \+= 1  
      
  * **For Loop**:  
      
    \# (Assuming a 'range' function or list/array support)  
      
    for i in range(5):  
      
        output(i)
