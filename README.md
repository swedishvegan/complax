# complax
## The official compiler and VM for the Lax programming language

## Table of contents
1. [Introduction](#pt1)
2. [Getting started](#pt2)
    1. [Pre-compiled binaries](#pt2.1)
    2. [Building the complax library from source](#pt2.2)
    3. [Using the Lax compiler and virtual machine](#pt2.3)
3. [Syntax](#pt3)
    1. [Basics](#pt3.1)
        1. [Hello world](#pt3.1.1)
        2. [Defining variables](#pt3.1.2)
        3. [Comments](#pt3.1.3)
        4. [Whitespace](#pt3.1.4)
        5. [Commas and colons](#pt3.1.5)
        6. [Data types](#pt3.1.6)
        7. [Arrays](#pt3.1.7)
    2. [Control flow: if, else, while](#pt3.2)
        1. [if/ else](#pt3.2.1)
        2. [while](#pt3.2.2)
        3. [Constant expression in the control flow condition](#pt3.2.3)
    3. [Built-in functions and operators](#pt3.3)
        1. [I/O and other system functions](#pt3.3.1)
        2. [Operators and comparators](#pt3.3.2)
        3. [Type casting and data type logic](#pt3.3.3)
        4. [Precedence table for all built-in patterns](#pt3.3.4)
    4. [Functions and pattern matching](#pt3.4)
        1. [Declaring and using functions](#pt3.4.1)
        2. [Argument types and return types](#pt3.4.2)
        3. [Precedence](#pt3.4.3)
        4. [Labels and forward declarations](#pt3.4.4)
        5. [Function overloading](#pt3.4.5)
    5. [Dealing with ambiguity](#pt3.5)
    6. [Imports and includes](#pt3.6)

## Introduction <a name="pt1"></a>

**UPDATE: As of 1/9/2023, the Lax virtual machine now supports JIT compilation. Massive shoutout to the incredible [SLJIT library](https://github.com/zherczeg/sljit) for making this possible. JIT support is currently experimental and hasn't yet been tested on all platforms, so the VM still runs in interpreted mode by default. See [Using the Lax compiler and virtual machine](#pt2.3) for info on how to use the JIT.**

**NOTE: Lax is currently still in development. I am making the project public in order to allow feedback, suggestions, and possibly outside contributions while the language is early in its development phase.**

Are you tired of other compilers telling you what to do all the time? Have you ever wanted to express your ideas in a particular way, but it just wouldn't quite conform to the syntax of the language you use? Ever wished you could just type "do the thing" and it would just magically work?

As a programmer, writing beautiful code is very important to me. I like to write code that is straightforward and concise, such that you can look at it and immediately tell what it does. As such, I often feel that my creativity is hampered by the syntactical rules of other languages.

This is why I created Lax. I felt that the way other languages parse and interpret code is unnecessarily strict, and I wanted to showcase how it is possible to create a language with a highly loose and flexible syntax without sacrificing the increased performance and control offered by lower-level languages.

Although modern programming languages have been trending towards being more human-readable, these apparent improvements are merely illusions. Using newlines instead of semicolons and tabs instead of curly brackets makes your code look prettier, but it doesn't fundamentally change the way code is parsed. __The way that Lax code is parsed is fundamentally different from other languages.__ Other languages parse your code based on some pre-defined grammer. Lax has an extremely minimal pre-defined grammar and high potential to alter the grammar by adding your own rules. In Lax, you can define your own syntactical "patterns" and the compiler uses a generalized pattern matching algorithm to detect instances of the patterns you create.

Sounds weird, right? Are you feeling confused? If so, please keep reading! After detailing how to install and run the Lax compiler and VM, I will provide detailed tutorials and examples to get you started coding in Lax.

### Quick facts about Lax:
 - __Lax is a statically typed language, but has dynamically typed syntax.__ This means that although all data types are known at compile time, you can write your code without ever thinking about them. Thus, Lax has the "feel" of a dynamically typed language, while avoiding the performance penalties and confusing runtime errors that come with other dynamically typed languages.
 - __Lax utilizes function templates and type inference automatically.__ Wondering how a language can be statically typed without even having types declared? This is achieved using function templating, combined with type inference. The types of variables are inferred from the types of the expressions that they are initialized to. Each time a function is called, the compiler generates a new instantiation of the templated function based on the data types of the arguments passed to it.
 - __Lax runs on a virtual machine and is "compile once, run anywhere."__ Lax code compiles to low-level bytecode, which is cross platform and runs on any 64-bit system that has the Lax virtual machine installed.
 - __Lax has a highly extensible syntax and grammar.__ There is almost no limit on what could be considered valid syntax in Lax. See [Syntax](#pt3) and specifically [Functions and pattern matching](#pt3.4) for more information on what makes the Lax syntax so flexible.
 - __Lax is JIT compiled to machine code.__ Despite its flexible syntax, Lax offers performance on par with ahead-of-time compiled code.

For now, take a look at this simple leap-year finding example program to get a feel for what Lax code looks like.

```
function { next leap year after {year} } wlabl { Find Next Leap Year }    // forward declaration

start program {

    output "Please enter a year: "

    current year = input as integer
    next leap year = next leap year after current year    // calling the function "next leap year after {year}"

    output "The next leap year is " + (next leap year as string) + "\n"

}

function { {year} is a leap year } {

    if year % 400 == 0, return true
    if year % 100 == 0, return false
    if year % 4 == 0, return true

    return false

}

function { {year} is not a leap year } { return not year is a leap year }

Find Next Leap Year {    // implementing the function "next leap year after {year}"

    while year is not a leap year, year = year + 1
    return year

}
```

## Getting started <a name="pt2"></a>

### Pre-compiled binaries <a name="pt2.1"></a>

Sorry, I don't have precompiled binaries yet for the current release of Lax.

### Building the complax library from source <a name="pt2.2"></a>

If there is no release for your particular coding environment, or if you're having any issues with the pre-compiled binaries, you can simply build the complax library from source. Building the complax library should be relatively straightforward as long as you have git, CMake, and a compiler that supports the C++11 standard. Here's what you need to do:

1. Navigate into the desired directory and clone the repository: &nbsp; ```git clone https://github.com/swedishvegan/complax.git```
2. Run the CMake script: &nbsp; ```cmake -S . -B .``` (if you have the CMake GUI program this is fine to use too).
    - __NOTE: If your compiler does not support C++11 by default, you may have to reconfigure your compiler or use a different compiler. In order to specify a compiler, add the argument__ ```-DCMAKE_CXX_COMPILER=[compiler binary name]```  __to the CMake command.__
    - __NOTE: The complax library has been tested and verified to work on the following compilers: GCC, MSVC, Clang. You are free to use a different compiler, but there is a risk that the library will fail to compile.__
3. Build the project.
    - On Windows with MSVC, simply open the newly generated ".sln" file with Visual Studio and build the project.
    - On Linux/MacOS with GCC or Clang, simply execute the newly generated Makefile by running ```make```.
    - __WARNING: I was unable to compile the project using AppleClang on MacOS because the default compiler did not support the C++11 standard. If you are having this issue, try using GCC instead:__ ```brew install gcc``` __and then running the CMake script with__```-DCMAKE_CXX_COMPILER=g++-13``` __(or whatever your version of g++ is). I believe it is also possible to configure AppleClang to use a different standard, but I had difficulties trying to do this.__

You should see two executables in the folder you chose to build binaries to: ```complax``` and ```lax```.

### Using the Lax compiler and virtual machine <a name="pt2.3"></a>

The repository comes with a few sample Lax programs to test out the compiler. If you're not building from source, you'll need to download these programs and move them into the same folder as the binaries.

To verify that the compiler and VM work, complete the following steps:

1. Navigate into the directory with the binaries: &nbsp; ```cd [path to binaries]```
2. Compile the "hello.lax" file: ```./complax hello.lax hello.l```
3. Run the Lax executable: ```./lax hello.l```

You should see the single line "Hello world" printed out to the terminal.

#### __Quick guide to the complax terminal interface:__
 - The ```complax``` compiler takes the following arguments: &nbsp; ```[input filename] [output filename] [optional flags (--emit_syntax_tree or --emit_bytecode)]```, where:
     - ```[input filename]``` is a valid filepath to any file containing Lax code. The ".lax" ending is just a convention and is not required.
     - ```[output filename]``` is a valid filepath that the Lax executable code will be saved to. The ".l" ending is just a convention and is not required.
     - ```--emit_syntax_tree``` outputs a visual representation of the syntax tree for each file that is parsed for the sake of debugging. For each file "filepath" parsed by the compiler, a second file "filepath.syntax_tree" is generated.
     - ```--emit_bytecode``` outputs a visual representation of the resulting Lax executable for the sake of debugging. Next to the Lax executable "output_filepath", a second file "output_filepath.bytecode" is generated.
 - The ```lax``` VM takes the following arguments: &nbsp; ```[optional flags (--jit or --interpret)] [input filename]```, where:
     - ```[input filename]``` is a valid filepath to the Lax executable to run.
     - ```--jit``` is fairly self-explanatory: it signals the VM to use the JIT compiler at runtime. As of right now, the VM uses interpreted mode by default and the JIT compiler must be explicitly enabled.
     - ```--interpret``` is the default option and does not need to be explicitly specified. If ```--jit``` and ```--interpret``` are both specified, an error will be generated.

There are two other example programs in the main directory of the repository: "pascal.lax" and "prime_numbers.lax". Feel free to try them out and look into the source code in order to get better acquainted with the language's syntax.

## Syntax <a name="pt3"></a>

### Basics <a name="pt3.1"></a>

#### Hello world <a name="pt3.1.1"></a>

Let's begin with a classic "Hello world" example program:

```
start program {

    output "Hello world\n"

}
```

The ```start program``` block tells the compiler where to start executing code. ```output``` is a built-in function for printing to the console (see [Built-in functions and operators](#pt3.3)).

If the compiler does not find a ```start program``` block, compilation will fail and an error like this will be generated:

```
 ERROR: Program has no starting point.
 Compilation finished in 0.000 seconds with 0 detected memory leak(s).
```

#### Defining variables <a name="pt3.1.2"></a>

Defining variables has a very intuitive syntax and looks like this:

```
global variable = 1

start program {

    local variable = global variable + 9
    
    output local variable
    output "\n"

} 
```

This simple program does nothing but print out the number "10" and then print a newline before exiting.

You can define variables in local scope (inside brackets) or global scope (outside brackets). Just know that variables will not be accessible outside the scope they were created in.

If you wish, you can also add the ```let``` keyword before a declaration. This can help disambiguate variable names, as explained in [Dealing with ambiguity](#pt3.5).

In Lax, you are not allowed to declare a local variable and then never use it. The following code:

```
start program {
    random variable = 100
}
```

will generate an error that looks like this:

```
 ERROR: Variable declared but never used:
 |  Declaration        (2.5-2.22):            randomvariable
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

See [Dealing with ambiguity](#pt3.5) for an explanation on why this safeguard is needed in Lax.

#### Comments <a name="pt3.1.3"></a>

Comments are just like in C/C++/Java:

```
start program {

    x = 3 // This is a single-line comment and will end at the next newline character

    /* 
       This
       is
       a
       multi-
       line
       comment.
    */

    output x - 3 // Outputs the number "0"
    output "\n"  // Outputs a newline

}
```

#### Whitespace <a name="pt3.1.4"></a>

Lax is unique from other languages in that whitespace does not have any effect on the way code is parsed, except for numeric literals or string literals.

This code:

```
start program { 

    important number = 42
    output important number

}
```

will compile exactly the same as this code:

```
startprogram{importantnumber=42outputimportantnumber}
```

or as this code:

```
st    ar t p    ro  g r a

m { 

    importa
    
    nt num b e r = 42
    o ut  pu t imp 
    ort
    ant numb
    er

}
```

If you look at the syntax tree generated for each of these, you will see that they are all identical (except for the line numbers, of course):

```
 GlobalScopeBuilder (1.1-14.1):          
 | Keyword_startpr... (1.1-3.2)
 | Body_Function      (3.3-13.2):            
 |  | Declaration:
 |  |  | Variable:
 |  |  |  | importantnumber
 |  | Assignment:
 |  |  | LH:
 |  |  |  | importantnumber
 |  |  | RH:
 |  |  |  | LiteralNode:    42
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | VariableNode:   importantnumber

```

Thus, you are completely free to use whitespace however you deem necessary in Lax. This freedom gives you the unique advantage of creating variable and function names that are multiple words long, and goes a long way in making your code look more readable and English-like.

An experienced programmer reading this might be alarmed by all the issues regarding ambiguity that could pop up as a result of this freedom. Luckily, Lax has plenty of tools and safeguards to help avoid such ambiguities, and it's surprisingly not nearly as much of an issue as you might think. See [Dealing with ambiguity](#pt3.5) for more details.

#### Commas and colons <a name="pt3.1.5"></a>

You can optionally place commas and colons after expressions, or after the ```else``` keyword (see [if/ else](#pt3.2.1)). This can help make your code more readable, and it can also help disambiguate your code as you will see later on in [Dealing with ambiguity](#pt3.5).

#### Data types <a name="pt3.1.6"></a>

Lax comes with five built-in "primitive" data types: &nbsp; ```integer```, ```decimal```, ```boolean```, ```ascii```, and ```string```.

 - ```integer``` values are for whole numbers, like 1, -1, 2, -2, etc.
 - ```decimal``` values are for any numeric value, either whole numbers or fractions, like 3.14, 1.618, etc.
 - ```boolean``` values are either true or false
 - ```ascii``` values are single 8-bit ascii characters and are delimited with single quotes, like 'A', '!', etc.
 - ```string``` values are arrays of ASCII text and are delimited with double quotes, like "Hello world", "What's your name?", etc.

Each data type is 64-bit. When you use a literal of any of these types in your code, the compiler will automatically detect its data type using the following heuristics:
1. If the literal is either ```true``` or ```false```, it is a boolean
2. Otherwise if the literal is delimited by single quotes, it is an ascii character
3. Otherwise if the literal is delimited by double quotes, it is a string
4. Otherwise if the literal is numeric and has a decimal point or an exponent (i.e. 1.23e10), it is a decimal
5. Otherwise, it is an integer

You can also create arrays, which are detailed in the next section, [Arrays](#pt3.1.7).

What if you want to explicitly declare the data type? Lax has a way to do this:

```
start program {

    a = integer // Variable "a" is assigned the value 0 with integer type
    b = decimal // Variable "b" is assigned the value 0 with decimal type
    c = boolean // Variable "c" is assigned the value "false" with boolean type
    d = ascii   // Variable "d" is assigned the ascii character with ID 0
    e = string  // Variable "e" is assigned an empty string

}
```

Lax also provides several mechanisms to convert from one type to another. See [Type casting and data type logic](#pt3.3.3) for more information on this.

What happens if you assign a variable an expression of the wrong type? Take a look at this example:

```
start program {

    x = true    // x is created with boolean type
    x = "false" // x is reassigned a value with string type

}
```

Luckily, if you make a mistake like this, the compiler will catch it for you and tell you what you did wrong:

```
 ERROR: Type mismatch in variable assignment.
 | Problematic expression:
 |  | Expression         (4.8-4.16):            
 |  |  | LiteralNode:    false
 | Expression type:
 |  | String
 | Expected type:
 |  | Boolean
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

#### Arrays <a name="pt3.1.7"></a>

You can create arrays containing any data type, including other arrays. Here are the different ways you can create an array:

```
start program {

    arr1 = { 1, 2, 3, 4 }           // Bracket initializer
    arr2 = string array of length 5 // Array of 5 empty strings

    output length of arr1           // Outputs '4'
    output "\n"

    output length of arr2           // Outputs '5'
    output "\n"

}
```

You can also reference the array type without actually creating an array, similar to using ```nullptr``` in C/C++. In Lax's lingo, this is called ```nothing```. Trying to dereference a variable that was assigned to ```nothing``` will cause a runtime error.

```
start program {

    arr = boolean array
    output arr[0]        // Don't do this!

}
```

This syntax is useful if you want to refer to the type of an array without actually creating an array object. However, you need to make sure that no arrays you reference in your code are assigned ```nothing```.

### Control flow: if, else, while <a name="pt3.2"></a>

#### if/ else <a name="pt3.2.1"></a>

```if``` statements are simply formed using the word ```if``` followed by a boolean expression. If the value of the expression is true, then the code immediately following the expression is executed. To execute multiple lines, you can use curly brackets.

After the ```if``` statement, you can have an ```else``` statement. The code following the ```else``` statement will only be executed if the value of the expression after the ```if``` statement is false.

Here's a simple example of an if/else statement in action:

```
start program {

    output "What's your name? "
    name = input

    if name == "swedishvegan", output "You're the creator of Lax!\n"
    else, output "You didn't create Lax, but I'm sure you're cool anyway!\n"

}
```

Here are two possible sample outputs, depending on the input you type in:

```
What's your name? Bob
You didn't create Lax, but I'm sure you're cool anyway!
```

Alternatively,

```
What's your name? swedishvegan
You're the creator of Lax!
```

If you want to run more than one statement after the ```if``` or ```else``` you can use curly brackets, like so:

```
start program {

    x=23    if x == 23 { output "You should see this message\n"    x = 10000    } else { output "You should not see this message\n" }
    
    output x    // Should output the number "10000"
    output "\n"

}
```

This code gives the following output:

```
You should see this message
10000
```

#### while <a name="pt3.2.2"></a>

To repeat the same instruction repeatedly, you can use a ```while``` loop. A ```while``` loop is the keyword ```while``` followed by a boolean expression. The code immediately following the expression will be run repeatedly in a loop as long as the expression evaluates to true.

For example, the following code simply counts to 10:

```
start program {

    x = 0
    while x < 10 { output "x = ", output x, output "\n", x = x + 1 } // x is incremented by 1 each time the loop is run.

}
```

This code has the following output:

```
x = 0
x = 1
x = 2
x = 3
x = 4
x = 5
x = 6
x = 7
x = 8
x = 9
```

#### Constant expression in the control flow condition

If the condition after an ```if``` or ```while``` keyword is a compile-time constant, the compiler can make the following optimizations:
 - For ```if``` statements, if the condition is true, the code after the condition will always be run, and the ```if``` is eliminated.
 - For ```if``` statements, if the condition is false and there is an ```else``` statement, the code after the ```if``` condition will be eliminated and the code after the ```else``` will always be run.
 - For ```while``` statements, if the condition is true, the code after the condition is simply turned into an infinite loop.
 - For ```while``` statements, if the condition is false, the code after the condition is eliminated.

These optimizations allow for useful functionality when utilizing Lax's templating abilities, such as having two different branches of a function body returning different data types depending on an ```if``` statement with a condition that is known at compile time.

### Built-in functions and operators <a name="pt3.3"></a>

#### I/O and other system functions <a name="pt3.3.1"></a>

__NOTE: I/O and other system functions will be expanded substantially in the near future with subsequent updates.__

Lax currently comes with four built-in system functions. Each of them are demonstrated in the below sample program:

```
start program {

    x = input    // Gets the next line of input from the user and stores it as a string

    output x + " is what you just said to me.\n"    // Outputs a string
    output 42    // Any non-string primitive data type will be automatically converted to a string
    output "\n"    // Newline

    start timer    // Built-in functionality for performance testing purposes

    i = 0
    while i < 10000000, i = i + 1

    time = timer value    // Gets the time (decimal in seconds) that has passed since the timer was started

    output "Took " + (time as string) + " seconds to count to 10000000.\n"

}
```

Here's a sample of the output from this code:
```
swedishvegan
swedishvegan is what you just said to me.
42
Took 0.040928 seconds to count to 10000000.
```

#### Operators and comparators <a name="pt3.3.2"></a>

Lax comes with all the standard operators and comparators that you're used to from other languages, with a few notable changes:
 - The ```^``` operator means exponentiation, not bitwise XOR.
 - Boolean operators are typed out using English words like in Python; i.e. ```&&```, ```||```, and ```!``` become ```and```, ```or```, and ```not``` respectively.
 - The bitwise NOT operator is still ! even though the boolean NOT operator is ```not```.

The order of operations for expressions depends on the precednece of every relevant operator in the expression. See [Precedence table for all built-in patterns](#pt3.3.4) for a complete list of these precedence values.

#### Type casting and data type logic <a name="pt3.3.3"></a>

Lax has a variety of tools to convert between types and query the types of existing variables and expressions.

To convert from one type to another, use the ```as``` keyword, like so:

```
start program {

    output "Please enter a number: "

    N = input as decimal    // Converts user input to a decimal so that we can do math on it
    N = N^2    // Squares N

    output "The square of that number is "
    output N
    output "\n"

}
```

You can also use the ```like``` keyword to cast one type to be the same type as another expression or variable:

```
start program {

    output "Please enter a number: "

    number two = 2    // number two is an integer
    N = input like number two    // Since number two is an integer, N will be initialized as an integer as well
    N = N^numbertwo

    output "The square of that number is "
    output N
    output "\n"

}
```

__NOTE:__ ```as``` __and__ ```like``` __actually do the exact same thing under the hood, and are completely interchangable. I just thought it was nice to provide both for the sake of greater readability.__

In order to query the type of a variable or expression, you can use the ```is``` or ```islike``` functions:

```
start program {

    example integer = 2
    example decimal = 2.2
    
    output example integer is integer    // Should output "true"
    output "\n"

    output example decimal is decimal    // Should output "true"
    output "\n"

    output (example integer is integer) is boolean    // Should output "true"
    output "\n"

    output example integer is like example decimal    // Should output "false" since the two variables have different types
    output "\n"

}
```

Indeed, here's what this program outputs:

```
true
true
true
false
```

__NOTE:__ ```is``` __and__ ```islike``` __actually do the exact same thing under the hood, and are completely interchangable. I just thought it was nice to provide both for the sake of greater readability.__

If you want to initialize a variable to be the same type as another variable, you can use the ```typeof``` keyword:

```
start program {

    x = 1.2345    // x is a decimal
    y = type of x    // y is a decimal initialized to zero, the default value for its type

    output y    // Prints out the number 0 in decimal form
    output "\n"

}
```

You can use the ```typeof``` keyword anywhere in your program, but I've found it to be mostly unnecessary for other situations.

__NOTE:__ ```is```__,__ ```is like```__, and__ ```typeof``` __all evaluate at compile-time, and thus generate compile-time constants.__

#### Precedence table for all built-in patterns <a name="pt3.3.4"></a>

Every built-in pattern has a precedence associated with, which determines the order of operations of expressions. In Lax, a precedence is a sequence of three decimal numbers, (p0, p2, p2). For any two precedences P = (p0, p1, p2) and Q = (q0, q1, q2), the rule for precedence is as follows:

1. If p0 > q0 then P > Q
2. Else if p0 < q0 then P < Q
3. Else if p1 > q1 then P > Q
4. Else if p1 < q1 then P < Q
5. Else if p2 > q2 then P > Q
6. Else if p2 < q2 then P < Q
7. Else P = Q

Patterns with a higher precedence always get evaluated first in an expression. If two precedences are equal, then by default the pattern on the right takes priority. For example, 1-2-3 would be parsed as 1-(2-3), not as (1-2)-3.

A pattern has both a left-hand precedence and a right-hand precedence, and the two are not necessarily equal.

Below is a table of precedences for every built-in pattern:


| Pattern       | LH Precedence    | RH Precedence    |
|---------------|------------------|------------------|
| {} + {}       | {1, 0, 0}        | {1, 0, 0}        |
| {} - {}       | {1, 0, 0}        | {1, 0, 0}        |
| {} * {}       | {1, 1, 0}        | {1, 1, 0}        |
| {} / {}       | {1, 1, 0}        | {1, 1, 0}        |
| {} ^ {}       | {1, 2, 0}        | {1, 2, 0}        |
| {} % {}       | {1, 3, 0}        | {1, 3, 0}        |
| {} & {}       | {1, 4, 0}        | {1, 4, 0}        |
| {} \| {}      | {1, 4, 0}        | {1, 4, 0}        |
| ! {}          | N/A              | {1, 5, 0}        |
| {} == {}      | {-1, 0, 0}       | {-1, 0, 0}       |
| {} != {}      | {-1, 0, 0}       | {-1, 0, 0}       |
| {} >= {}      | {-1, 0, 0}       | {-1, 0, 0}       |
| {} <= {}      | {-1, 0, 0}       | {-1, 0, 0}       |
| {} > {}       | {-1, 0, 0}       | {-1, 0, 0}       |
| {} < {}       | {-1, 0, 0}       | {-1, 0, 0}       |
| {} and {}     | {-3, 0, 0}       | {-3, 0, 0}       |
| {} or {}      | {-3, 0, 0}       | {-3, 0, 0}       |
| not {}        | N/A              | {-3, 1, 0}       |
| {} is {}      | {-2, 0, 0}       | {2, 0, 0}        |
| {} islike {}  | {-2, 0, 0}       | {2, 0, 0}        |
| {} as {}      | {-2, 0, 0}       | {2, 0, 0}        |
| {} like {}    | {-2, 0, 0}       | {2, 0, 0}        |
| typeof {}     | N/A              | {2, 0, 0}        |

### Functions and pattern matching <a name="pt3.4"></a>

#### Declaring and using functions <a name="pt3.4.1"></a>

The best way to illustrate how declaring a function works in Lax is with a simple example:

```
function { stuff with {a} and {b} } { return a + b }
```

Let's break this down:
1. The keyword ```function``` tells the compiler that what's about to follow is a function declaration.
2. The information ```{ stuff with {a} and {b} }``` inside the first set of curly braces tells the compiler the pattern that it should look for: (1) the characters "stuffwith" followed by (2) an expression followed by (3) the characters "and" followed by (4) an expression.
3. The code inside the second set of curly braces is the function body, or the code that actually runs when the function is called.

So for example,

```
function { stuff with {a} and {b} } { return a + b }

start program { 
    
    output stuff with 1 and 2    // Calls the function "stuff with {a} and {b}" with a=1 and b=2
    output "\n"    // Newline
    
}
```

When run, this program outputs the number 3, because it calls the function ```stuff with {a} and {b}```, with a = 1 and b = 2, and then returns the sum a + b.

You can also use function calls inside of other function calls:

```
function { stuff with {a} and {b} } { return a + b }

start program { 
    
    output stuff with stuff with 1 and 2 and stuff with 3 and 4  // Outputs 1+2+3+4 = 10
    output "\n"    // Newline
    
}
```

If you're an experienced programmer, there's probably alarm bells going off in your head right now thinking about all the problems that could arise from a language like this. Please bear with me for now, and in [Dealing with ambiguity](#pt3.5) I will explain how all of those problems are handled.

#### Argument types and return types <a name="pt3.4.2"></a>

I mentioned before that the types of arguments are automatically inferred from the types of expressions passed to function calls. However, sometimes it is advantageous to explicitly declare the type(s) that an argument is allowed to have. Lax lets you do so with the ```wrest``` qualifier.

Here's how you use the ```wrest``` qualifier. Let's say in our ```stuff with {a} and {b}``` function, we want to enforce that ```a``` is only allowed to be an integer, but ```b``` could be an integer or a decimal. The following code shows you how to do this:

```
function { stuff with {a} and {b}} wrest { a: integer, b: integer, decimal } { return a + b }

start program {

    output stuff with 1 and 2   // OK: 1 is an integer and 2 is an integer
    output stuff with 1 and 2.3 // OK: 1 is an integer and 2.3 is a decimal
    output stuff with 1.2 and 2 // ERROR: 1.2 not an integer
    output "\n"

}
```

When we try to compile this code, we get the following error:

```
 ERROR: Pattern match has no valid matches given the supplied arguments.
 | Source expression:
 |  | Expression         (6.32-7.32):           
 |  |  | Sys:
 |  |  |  | FillerNode:     output
 |  |  |  | PatternMatch:
 |  |  |  |  | FillerNode:     stuffwith
 |  |  |  |  | LiteralNode:    1.2
 |  |  |  |  | FillerNode:     and
 |  |  |  |  | LiteralNode:    2
 | Problematic pattern match:
 |  | stuffwith {} and {}
 | Argument types supplied:
 |  | TypeList:
 |  |  | Decimal
 |  |  | Integer
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

You can also use function calls or expressions as a type restriction:

```
function { dumb test } { return "dumb test" } // This function always returns a string

function { stuff with {a} and {b} } { return a + b } wrest { a: typeof dumb test, b: typeof dumb test }

start program {

    output stuff with "Hello " and "world.\n" // OK
    output stuff with 1 and 2 // ERROR: Arguments need to be string

}
```

This code also gives you an error message:

```
 ERROR: Pattern match has no valid matches given the supplied arguments.
 | Source expression:
 |  | Expression         (7.46-8.30):           
 |  |  | Sys:
 |  |  |  | FillerNode:     output
 |  |  |  | PatternMatch:
 |  |  |  |  | FillerNode:     stuffwith
 |  |  |  |  | LiteralNode:    1
 |  |  |  |  | FillerNode:     and
 |  |  |  |  | LiteralNode:    2
 | Problematic pattern match:
 |  | stuffwith {} and {}
 | Argument types supplied:
 |  | TypeList:
 |  |  | Integer
 |  |  | Integer
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

If you delete the problematic line, the program prints out "Hello world" like you would expect.

You also need to be careful not to include a function inside its own restrictions. Consider the following code:

```
function { stuff with {a} and {b} } wrest { a: typeof stuff with 1 and 2, b: integer } { return a + b }
start program { output stuff with 1 and 2, output "\n" }
```

If this were allowed, the compiler would get stuck in an infinite loop. To avoid this, it has been made explicitly illegal to reference a function inside its own restrictions. If you try to compile this code, you will get an error that looks like this:

```
 ERROR: Pattern match cannot refer to itself within its restrictions, either directly or indirectly.
 | Problematic pattern match:
 |  | Builder_Header_... (1.11-1.35)          
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

Here are a couple important notes on ```wrest``` syntax:
1. The ```wrest``` keyword and subsequent restrictions can come before or after the function body.
2. The colons and commas inside the restrictions body are technically optional like they always are. ```wrest { a integer b integer decimal }``` works just fine, although it is rather confusing to read.

You can also explicitly declare the return type of a function. Usually you don't need to do this because the compiler can infer the return type from the code inside the function body. However, in some special cases related to recursive function calls it is necessary to explicitly declare the return type.

Here's a simple example of a recursive function that calculates the factorial function:

```
function { {n} ! } wprec {10} /* Explicitly declaring precedence; this is explained later in tutorial */ {

    if n <= 1 return 1 like n  // Make sure return type is same as argument type
    else return n * (n-1)!

}

start program {

    output "Enter a number: "
    n = input as integer
    output "n" + "! = " + (n! as string) + "\n"

}
```

This generates an error, because the return type of the factorial function is based on an expression containing the factorial function itself. Thus there is insufficient information in this code to infer the return type of the function.

```
 ERROR: Recursive functions with the same type instantiation must have their return types explicitly declared, and the return type must not reference itself.
 | Source expression:
 |  | Expression         (4.16-4.27):           
 |  |  | Operator:
 |  |  |  | VariableNode:   n
 |  |  |  | FillerNode:     *
 |  |  |  | PatternMatch:
 |  |  |  |  | Operator:
 |  |  |  |  |  | VariableNode:   n
 |  |  |  |  |  | FillerNode:     -
 |  |  |  |  |  | LiteralNode:    1
 |  |  |  |  | FillerNode:     !
 | Problematic pattern match:
 |  | {} !
 | Argument types supplied:
 |  | TypeList:
 |  |  | Integer
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```
To get around this, you must explicitly declare the return type:

```
function { {n} ! } wprec {10} returns typeof n {

    if n <= 1 return 1 like n  // Make sure return type is same as argument type
    else return n * (n-1)!

}

start program {

    output "Enter a number: "
    n = input as integer
    output n as string + "! = " + (n! as string) + "\n"

}
```

The program now works as expected:

```
Enter a number: 5
5! = 120
```

Note that like with ```wrest```, the ```returns``` qualifier can come in any order with respect to the function body and the other qualifiers.

#### Precedence <a name="pt3.4.3"></a>

You already got a glimpse of this when we explicitly declared the precedence of the factorial function. The ```wprec``` keyword works like the other function qualifiers you've seen so far and can appear in any order with respect to everything else. You can use it to specify both a left-hand and right-hand precedence, but if the right-hand one is missing, the right-hand and left-hand precedences will automatically be set to the same value.

Here's how you can define custom precedences for functions:

```
function { F {x} } 
    wprec{1,2}  // Any numbers left out will be filled in as zeroes
    { return 2*x }

function { {x} G }
    wprec{1,3}
    { return x+1 }

function { {a} $$ {b} }
    wprec
        {-10} // LH precedence
        <->
        {10}  // RH precedence
    { return (a+b)/2 }

start program {

    output (F 3 G $$ 4 G)
    output "\n"

}
```

Please take a minute to convince yourself that based on the precedence rules defined, this expression will be parsed as ```( ( F (3 G) ) $$ 4 ) G```. If you look at the syntax tree created by the compiler and scroll down to this particular expression, you can see that it is being parsed correctly the way you would expect:

```
 | Body_Function      (16.15-21.2):          
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | PatternMatch:
 |  |  |  | PatternMatch:
 |  |  |  |  | PatternMatch:
 |  |  |  |  |  | FillerNode:     F
 |  |  |  |  |  | PatternMatch:
 |  |  |  |  |  |  | LiteralNode:    3
 |  |  |  |  |  |  | FillerNode:     G
 |  |  |  |  | FillerNode:     $$
 |  |  |  |  | LiteralNode:    4
 |  |  |  | FillerNode:     G
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | LiteralNode:    
```

On the other hand, what if we swap the LH and RH precedence of the last function, like so:

```
function { {a} $$ {b} } wprec {10} <-> {-10} { return (a+b)/2 }
```

Take another minute to convince yourself that the expression will be parsed as ```F ( (3 G) $$ (4 G) )```. The syntax tree once again confirms that this is how the expression is parsed:

```
 | Body_Function      (16.15-21.2):          
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | PatternMatch:
 |  |  |  | FillerNode:     F
 |  |  |  | PatternMatch:
 |  |  |  |  | PatternMatch:
 |  |  |  |  |  | LiteralNode:    3
 |  |  |  |  |  | FillerNode:     G
 |  |  |  |  | FillerNode:     $$
 |  |  |  |  | PatternMatch:
 |  |  |  |  |  | LiteralNode:    4
 |  |  |  |  |  | FillerNode:     G
```

One more thing to be aware of is that it is illegal to change the precedence of a function after it has already been used somewhere in the code. For example:

```
function { test: {a} } wlabl { TEST } // Forward declaration; see next subsection for info
function { test2: {a} } { return test: a }

start program { output test2: 300, output "\n" }
TEST wprec {100} /* Illegal redefinition of precedence */ { return 100 }
```

If you run this code you will get an error that looks like this:

```
 ERROR: Symbol precedence cannot be changed after the symbol has been used in expression parsing:
 | Body_Precedence    (5.12-5.17)
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

#### Labels and forward declarations <a name="pt3.4.4"></a>

Sometimes you need to call a function before the function itself is defined, most notably in mutually recursive functions. This can be done using the ```wlabl``` qualifier, which works like all the other qualifiers we've discussed. Here's how you can use labels to implement forward declarations:

```
function { hello world } wlabl { Hello world function here } // Forward declaration

start program { hello world }

Hello world function here: { output "Hello world\n" } // Implementing the function -- the colon here is optional as always
```

This code outputs "Hello world" like you would expect.

#### Function overloading <a name="pt3.4.5"></a>

Lax lets you define multiple functions with the same pattern, as long as the compiler is able to disambiguate which version of the function you're calling during type evaluation.

```
function { stuff with {a} and {b} } wrest { a: string, b: string } { return "This is the string version of the function\n" }
function { stuff with {a} and {b} } wrest { a: integer, b: integer } { return "This is the integer version of the function\n" }

start program {

    output stuff with "Hey" and "Wassup"  // Compiler knows you're calling the string version
    output stuff with 300 and 6789  // Compiler knows you're calling the integer version

}
```

Here's what this program prints out:

```
This is the string version of the function
This is the integer version of the function
```

If it is ambiguous which function you're trying to call, a compiler error is generated. If you try to compile this code:

```
function { test: {x} } wrest { x: string, boolean } { return x }
function { test: {x} } wrest { x: boolean } { return not x }

start program {

    output test: "HELLO"  // OK: There is only one pattern with the signature "test: {}" that takes a string as an argument
    output test: true     // ERROR: Both versions of "test: {}" allow a boolean, so the compiler doesn't know which one you mean

}
```

you will get an error that looks like this:

```
 ERROR: Ambiguous pattern match has multiple valid matches given the supplied arguments.
 | Source expression:
 |  | Expression         (6.25-7.22):           
 |  |  | Sys:
 |  |  |  | FillerNode:     output
 |  |  |  | PatternMatch:
 |  |  |  |  | FillerNode:     test:
 |  |  |  |  | LiteralNode:    true
 | Problematic pattern match:
 |  | test: {}
 | Argument types supplied:
 |  | TypeList:
 |  |  | Boolean
 | First valid match:
 |  | Builder_Header_... (1.11-1.22)          
 | Second valid match:
 |  | Builder_Header_... (2.11-2.22)          
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

You also need to be careful that all the overloaded functions have the same precedence. Otherwise, you get an error. For example, this code:

```
function { test: {x} } wrest { x: string } wprec{100} { return x }
function { test: {x} } wrest { x: boolean } wprec{200} { return not x }

start program { output test: "Hello\n" }
```

will give you this error message:

```
 ERROR: Two symbols with the same signature have conflicting precedences.
 | First symbol defined at:
 |  | Builder_Header_... (1.11-1.22)          
 | Second symbol defined at:
 |  | Builder_Header_... (2.11-2.22)          
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

Note that all the built-in patterns discussed in [Built-in functions and operators](#pt3.3) are also overloadable. You just need to make sure the precedence of the overloaded function matches the precedence of the built-in pattern.

### Dealing with ambiguity <a name="pt3.5"></a>

The keen reader might have realized the high potential for ambiguity in Lax. It's easily possible to construct code that seemingly has multiple interpretations. Take a look at this simple example:

```
function { F {arg} ... } { return 1 }
function { Function ... } { return 2 }

start program {

    unction = 100

    output Function... // Amgibuous expression
    output "\n"

}
```

There are two ways ```Function...``` could be interpreted: (1) ```F(unction)...``` (calling the function on line 1) and (2) ```Function...``` (calling the function on line 2).

This is no problem for the Lax compiler. __Lax evaluates all possible interpretations of an expression.__ If it finds multiple equally valid interpretations, a compiler error is generated.

In this particular example, here's what happens when you try to compile:

```
 ERROR: Ambiguous expression has two equally valid interpretations:
 | First interpretation (6.18-8.23):
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | PatternMatch:
 |  |  |  | FillerNode:     F
 |  |  |  | VariableNode:   unction
 |  |  |  | FillerNode:     ...
 | Second interpretation (6.18-8.23):
 |  | Sys:
 |  |  | FillerNode:     output
 |  |  | PatternMatch:
 |  |  |  | FillerNode:     Function...
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

If there are two possible valid expressions but one is longer than the other, the compiler always selects the longer one. For example, check out this program:

```
function { {a} @ {b} } { return a-b }
function { {a} @ } { return a }

start program { 

    output 2 @ 1
    output "\n"

}
```

The expression ```2 @ 1``` could be interpreted as ```{} @ ``` (function on line 2) or as ```{} @ {}``` (function on line 1). I have found that the overwhelming majority of the time, the longest possible expression is the one that you meant. It's very rare for this not to be the case. Thus, the compiler defaults to choosing ```{} @ {}``` as the valid interpretation, and this program compiles normally and outputs "1".

What about the rare situation where the shorter interpretation is the correct one? Take a look at this example:

```
function { {var} V } { return 0 }

start program {

    Variable 1 = 20
    Variable 1 = 30

    output Variable 1
    output "\n"

}
```

Looks harmless enough, but look more carefully. When ```Variable 1``` is declared, the compiler scans for the longest valid expression following the ```=``` sign. Since Lax doesn't care about whitespace, it finds the function call ```20 V``` and assumes that this must be what you meant. Thus, an unwanted variable called ```ariable 1``` is declared!

Please don't worry though! The compiler knows when you declared a variable you didn't mean to declare. Compiling this code, you'll get the following error message:

```
 ERROR: Variable declared but never used:
 |  Declaration        (6.6-6.17):            ariable1
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

(Note that this only works for local variables. Global variables can be declared without being used.)

But what if you were trying to create a second variable, called ```Variable 2```?

First of all, you could easily disambiguate the expression by adding a comma or a colon after ```Variable 1 = 20```. This tells the compiler to stop scanning for the current expression. For this reason I believe it's good practice to use commas in any place where there's any possibility of ambiguity. However, Lax won't force you to use them if you don't want to. That's the whole point of Lax -- to allow you to code the way you want to code with as few restrictions as possible.

But let's say you forgot to add a comma. What would happen if you ran this code?

```
function { {var} V } { return 0 }

start program {

    Variable 1 = 20
    Variable 2 = Variable 1

    output Variable 2
    output "\n"

}
```

You will get this somewhat vague error message:

```
 ERROR: Non-whitespace character(s) in builder (3.16-11.1) following final pattern:
 | Expression         (6.17-6.28)
 Compilation finished in 0.001 seconds with 0 detected memory leak(s).
```

This is because the compiler was trying to parse the expression ```output Variable 2``` but it failed because the variable called ```Variable 2``` was never created. If you got this error message, you would have to go look back in the relevant part of the code to figure out what happened, and it usually doesn't take long to figure it out. This is one of the unfortunate tradeoffs of using such a flexible language.

Alternatively, suppose the function ```V``` doesn't take any arguments:

```
function { V } { return 0 }

start program {

    Variable 1 = 20

    output Variable 1

}
```

This introduces a completely new problem. Since the function call ```V``` by itself is valid syntax (unlike in the last example), the compiler thinks you're trying to call the function ```V``` and then create a variable called ```ariable1```. Thus, you get another vague-looking error message:

```
 ERROR: Non-whitespace character(s) in builder (3.16-9.1) following final pattern:
 | Expression         (5.20-7.13)
 Compilation finished in 0.000 seconds with 0 detected memory leak(s).
```

How do you let the compiler know you're trying to create a variable named ```Variable 1```? The answer is actually very simple. Recall from earlier that you can precede any declaration with the ```let``` keyword. This lets the compiler know that whatever comes next is a declaration. Thus, this problem can be solved like so:

```
function { V } { return 0 }

start program {

    let Variable 1 = 20

    output Variable 1

}
```

This code compiles and runs exactly as you would expect.

Let's also briefly discuss the efficiency of expression parsing. This new method of parsing an expression might sound scary at first. Surely evaluating every possible interpretation must be horribly inefficient? Actually, in practice it is rarely an issue, because the overwhelming majority of branches that are created end up being garbage and are killed off immediately. Thus, the branching factor of the expression parser will essentially never be high enough to cause exponential explosion in practice.

### Imports and includes <a name="pt3.6"></a>

There are two ways to split your code across multiple files: ```import``` and ```include```. 

```import``` is the recommended way to use code from one file in another file. Let's say we have a file called "import.lax" with this code inside the file:

```
variable to import = 10
```

Then, in another file "main.lax" we can import our other Lax file like so:

```
import "import.lax"
start program { output variable to import as string + "\n" }
```

Unlike includes in C/C++, importing is not transitive -- if you import file A in file B, and import file B in file C, file A will NOT be imported into file C. This helps keep symbol tables small and avoid symbol pollution.

If you want that transitivity, you need to use ```include```. Including file A in file B will ensure that whenever file B is imported or included in some other file C, file A will also be brought with.

Please note that all imports and includes must be at the top of the file, before any other code is written.