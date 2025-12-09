## **EaC (ēzē) Programming Language Specification**

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

* Character \= {Letters, Digits, Symbols}   
* Letters \= {Uppercase, Lowercase}   
  * Uppercase (CAP) \= {A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X Y, Z}    
  * Lowercase (SM) \= {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z}     
* Digits \= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}   
* Special Symbols \= {+, \-, \*, %, /, \=, \>, \<, \!, ^, \_, (, ), \[, \], :, ,, .,} 


#### **2\. Identifiers**

Identifiers are names used to reference variables and constants.

* **Rules for Naming**:  
    
  1. An identifier must start with a letter (SM, CAP) or an underscore (\_).  
  2. After the first character, an identifier may contain any combination of letters (SM, CAP), digits (Digits), or underscores (\_).  
  3. An identifier must contain at least one character.  
  4. An identifier is case-sensitive; uppercase and lowercase letters are treated as distinct.  
  5. An identifier cannot be a reserved word (keyword).  
  6. An identifier has no maximum length restriction.


* **Regular Expression**: (Letters, \_)(Letters, Digits, \_)\*  
    
* **Machine**: An identifier is recognized by a state machine where:  
    
  * q0 (start state) transitions to q1 (accepting state) on any letter (CAP, SM) or an underscore (\_).  
  * q1 (accepting state) transitions back to itself (q1) on any letter, digit (Digits), or underscore.  
    ![][image1]


* **Recommended Style Guide**: Use snake\_case for all variable and constant names (e.g., my\_variable, FIXED\_RATE).

#### **3\. Operation Symbols**

EaC uses familiar operators for expressions.

* **Arithmetic Operators**  
    
  * \+ (Addition)

	**	![][image2]**

* \- (Subtraction)

	**	![][image3]**

* \* (Multiplication)

	**	![][image4]**

* / (Division)

	**	![][image5]**

* % (Modulo)

		**![][image6]**

* ^ (Exponent)  
  ![][image7]  
  * // (Floor Division)

	**	![][image8]**

* **Relational & Equality Operators**  
    
  * \< (Less than)  
    ![][image9]  
  * \> (Greater than)

	**	![][image10]**

* \== (Equal)

	**	![][image11]**

* \<= (Less than or Equal)  
  **![][image12]**  
  * \>= (Greater than or Equal)  
    **![][image13]**  
  * \!= (Not Equal)

	**	![][image14]**

* **Logical Operators**  
    
  * and (Logical AND)  
    ![][image15]  
      
      
  * or (Logical OR)

	**	![][image16]**

* not (Logical NOT)  
  ![][image17]

    
* **Assignment Operators**  
    
  * \= (Assignment)  
    ![][image18]  
  * \+= (Add and assign)  
    ![][image19]  
  * \-= (Subtract and assign)

	**	![][image20]**

* \*= (Multiply and assign)

	**	![][image21]**

* /= (Divide and assign)

	**	![][image22]**

* %= (Modulo and assign)

	**	![][image23]**

#### **4\. Keywords and Reserved Words**

Keywords are predefined words with special meanings.

* **4.a. Keywords**  
    
  * flex: Declares a flexible (mutable) variable.

![][image24]

* fixed: Declares a fixed (immutable) constant.

**![][image25]**

* when: Begins a conditional block (Python's if)

**![][image26]**

* else: Begins an alternate conditional block.

**![][image27]**

* output: Built-in function to print to the console.

![][image28]

* function: Declares a function block.

**![][image29]**

* import: Starts a module import declaration.

**![][image30]**

* from: Specifies the source in an import declaration.

**![][image31]**

* while: Begins a while-loop.

**![][image32]**

* for: Begins a for-loop (typically with in).

**![][image33]**

* in: Used in for loops for iteration.

**![][image34]**

* break: Exits the innermost loop.

**![][image35]**

* continue: Skips to the next iteration of the loop.

**![][image36]**

* return: Exits a function, optionally returning a value.

**![][image37]**

* input: Built-in function for user input

![][image38]

* **4.b. Optional Type-Hint Keywords (Reserved Words)** These words are reserved for use as *optional* type hints in variable declarations. They do not affect the dynamic typing but improve code clarity.  
  * true: Boolean true value.

![][image39]

* false: Boolean false value.

![][image40]

* int:

![][image41]

* float:

![][image42]

* str:

![][image43]

* bool:

![][image44]

* char:

![][image45]

#### **5\. Noise Words**

**Definition:** Noise words are tokens that improve code readability by making syntax more natural, while being ignored by the parser for semantic analysis.

| Noise Word | Purpose | Syntax Example | Natural Reading |
| :---- | :---- | :---- | :---- |
| **to** | Connect variable to value | flex x to 10 | "flex x to 10" |
| **of** | Link variable to type | flex age of int \= 25 | "age of type int" |
| **then** | Connect condition to action | when x \> 5 then: | "when x \> 5, then..." |
| **each** | Make loops more natural | for each item in list: | "for each item in list" |
| **as** | Type conversion/casting | result as float \= x / y | "result as float" |

to:  
![][image46]  
of:  
![][image47]

then:  
![][image48]

each:  
![][image49]

as:  
![][image50]

#### **6\. Comments**

Comments are recognised by the lexer and emitted as tokens, allowing tooling to inspect or preserve them while later compilation phases may ignore them for execution.

* **Single-line comments**: Start with \# and extend to the end of the line.  
  * *Example*: flex x \= 10 \# This is a comment  
  * *Machine*: q0 → \# → q1 (final). State q1 loops on any character except a newline.  
    ![][image51]  
* **Multi-line comments**: Start with /\* and end with \*/.  
  * *Example*: /\* This is a multi-line comment. \*/  
  * *Machine*: q0 → / → q1 → \* → q2. State q2 transitions to q3 on \*. State q3 transitions to the final state q4 on /. If q3 sees any char other than /, it returns to q2.  
    ![][image52]

#### **7\. Blanks (Spaces)**

EaC is a Python-inspired language that uses **significant indentation**.

* **Indentation** (spaces or tabs at the beginning of a line) is used to define code blocks (e.g., inside a when statement or a while loop). Braces {} are not used.  
* **Whitespace** (spaces between tokens on the same line, e.g., x \= 10\) is used as a delimiter and is ignored, allowing for flexible formatting.  
* **EOF** is used to determine if the text file reached the end or no more tokens left to be read.  
* **Error**   
* **Newline** (new line indention \\n) is not seen in the code however it is read as token \\n.

#### **8\. Delimiters and Brackets**

* ( ) **Parentheses**: Used to group expressions and enclose function arguments.  
  ![][image53]  
  ![][image54]  
    
* \[ \] **Brackets**: Used for list/array declaration and element access.  
  ![][image55]  
  ![][image56]

* : **Colon**: Used to terminate a statement that introduces a new indented code block (e.g., when a \> 5:, else:).  
  ![][image57]  
* , **Comma**: Separates items in a list, such as function arguments.  
  ![][image58]  
    
* **. Dot**: Indicates decimal.  
  ![][image59]

* **Newline**: A newline character terminates a statement. Semicolons (;) are **not** used.  
  ![][image60]

#### **9\. Free- and Fixed-Field Formats**

EaC uses a **fixed-field** format. Statements are required to be in specific columns. All statements must adhere to the **indentation rules** to define their block structure.

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
  3. Equality (==, \!=)  
  4. Conjunction (and)  
  5. Expression (or)

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
      
    output("Passing Grade\\\!")    
      
    else:  
      
    output("Failing Grade\\\!")    
      
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
      
    count \\= count \\+ 1  \\\# or count \\+= 1    
      
  * **For Loop**:  
      
    \# (Assuming a 'range' function or list/array support)  
      
    for i in range(5):  
      
    output(i)

