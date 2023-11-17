# complax
## The official compiler and VM for the Lax programming language

## Table of contents
1. [Introduction](#pt1)
2. [Getting started](#pt2)
    1. [Building the complax library](#pt2.1)
    2. [Using the Lax compiler and virtual machine](#pt2.2)

## Introduction <a name="pt1"></a>

**NOTE: Lax is currently still in development. I am making the project public in order to allow feedback, suggestions, and possibly outside contributions while the language is early in its development phase. The following features are still in progress and will be released in the near future, more or less in this order:**

 - **Arrays**
 - **User-defined structures**
 - **Garbage collection**
 - **More detailed error messages**
 - **File I/O, math functions, and other standard functionality**
 - **Function pointers**
 - **JIT (and possibly AOT) support**
 - **Multithreading support**
 - **C++ interface for compiling custom dynamic library-based modules**

Are you tired of other compilers telling you what to do all the time? Have you ever wanted to express your ideas in a particular way, but it just wouldn't quite conform to the syntax of the language you use? Ever wished you could just type "do the thing" and it would just magically work?

As a programmer, writing beautiful code is very important to me. I like to write code that is straightforward and concise, such that you can look at it and immediately tell what it does. As such, I often feel that my creativity is hampered by the syntactical rules of other languages.

This is why I created Lax. I felt that the way other languages parse and interpret code is unnecessarily strict, and I wanted to showcase how it is possible to create a language with a highly loose and flexible syntax without sacrificing the increased performance and control offered by lower-level languages.

Although modern languages have been trending towards increased simplicity and readability, these apparent improvements are merely illusions. Using newlines instead of semicolons and tabs instead of curly brackets makes your code look prettier, but it doesn't fundamentally change the way code is parsed. __The way that Lax code is parsed is fundamentally different from other languages.__ Other languages parse your code based on some pre-defined grammer. Lax has an extremely minimal pre-defined grammar and high potential to alter the grammar by adding your own rules. In Lax, you can define your own syntactical "patterns" and the compiler uses a generalized pattern matching algorithm to detect instances of the patterns you create.

Sounds weird, right? Are you feeling confused? If so, please keep reading! After detailing how to install and run the Lax compiler and VM, I will provide detailed tutorials and examples to get you started coding in Lax.

### Quick facts about Lax:
 - __Lax is a statically typed language, but has dynamically typed semantics.__ This means that although all data types are known at compile time, you can write your code without ever thinking about them. Thus, Lax has the "feel" of a dynamically typed language, while avoiding the performance penalties and confusing runtime errors that come with other dynamically typed languages.
 - __Lax utilizes function templates and type inference automatically.__ Wondering how a language can be statically typed without even having types declared? This is achieved using function templating, combined with type inference. The types of variables are inferred from the types of the expressions that they are initialized to. Each time a function is called, the compiler generates a new instantiation of the templated function based on the data types of the arguments passed to it.
 - __Lax supports compile-time constant evaluation and type evaluation.__ Constant expressions are automatically evaluated at compile-time, which allows for some useful tricks that are detailed later in the tutorial.

For now, take a look at this simple leap-year finding example program to get a feel for what Lax code looks like.

```
function { next leap year after {year} } wlabl { Find Next Leap Year }

start program {

    output 'Please enter a year: '

    current year = input as integer
    next leap year = next leap year after current year

    output 'The next leap year is ' + "next leap year" + '\n'

}

function { {year} is a leap year } {

    if year % 400 == 0, return true
    if year % 100 == 0, return false
    if year % 4 == 0, return true

    return false

}

function { {year} is not a leap year } { return not year is a leap year }

Find Next Leap Year {

    while year is not a leap year, year = year + 1
    return year

}
```

## Getting started <a name="pt2"></a>

### Building the complax library <a name="pt2.1"></a>

Building the complax library should be relatively straightforward as long as you have git, CMake, and a compiler that supports the C++11 standard. Here's what you need to do:

1. Navigate into the desired directory and clone the repository: &nbsp; ```git clone https://github.com/swedishvegan/complax.git```
2. Run the CMake script: &nbsp; ```cmake -S . -B ./[path to build binaries]``` (if you have the CMake GUI program this is fine to use too).
    - __NOTE: If your compiler does not support C++11 by default, you may have to reconfigure your compiler or use a different compiler. In order to specify a compiler, add the argument__ &nbsp; ```-DCMAKE_CXX_COMPILER=[compiler binary name]```  __to the CMake command.__
    - __NOTE: The complax library has been tested and verified to work on the following compilers: GCC, MSVC, Clang, MinGW. You are free to use a different compiler, but there is a risk that the library will fail to compile.__
3. Build the project.
    - On Windows with MSVC, simply open the newly generated ".sln" file with Visual Studio and build the project.
    - On Linux/MacOS with GCC or Clang, simply execute the newly generated Makefile by running &nbsp; ```make```.
    - __WARNING: I was unable to compile the project using AppleClang on MacOS because the default compiler did not support the C++11 standard. If you are having this issue, try using GCC instead:__ &nbsp; ```brew install gcc``` __and then running the CMake script with__ &nbsp; ```-DCMAKE_CXX_COMPILER=g++-13``` __(or whatever your version of g++ is). I believe it is also possible to configure AppleClang to use a different standard, but I had difficulties trying to do this.__

You should see two executables in the folder you chose to build binaries to: "complax" and "lax".

### Using the Lax compiler and virtual machine <a name="pt2.2"></a>

The repository comes with a few sample Lax programs to test out the compiler. To verify that the compilar and VM work, complete the following steps:

1. Navigate into the directory with the binaries: &nbsp; ```cd [path to binaries]```
2. Compile the "hello.lax" file:
    - On Windows: &nbsp; ```complax hello.lax hello.l```
    - On Linux/MacOS: &nbsp; ```./complax hello.lax hello.l```
3. Run the Lax executable:
    - On Windows: &nbsp; ```lax hello.l```
    - On Linux/MacOS: &nbsp; ```./lax hello.l```

You should see the single line "Hello world" printed out to the terminal.

#### __Quick guide to the complax terminal interface:__
 - The "complax" compiler takes the following arguments: &nbsp; ```[input filename] [output filename] [optional flags (--emit_syntax_tree or --emit_bytecode)]```, where:
     - ```[input filename]``` is a valid filepath to any file containing Lax code. The ".lax" ending is just a convention and is not required.
     - ```[output filename]``` is a valid filepath that the Lax executable code will be saved to. The ".l" ending is just a convention and is not required.
     - ```--emit_syntax_tree``` outputs a visual representation of the syntax tree for each file that is parsed for the sake of debugging. For each file "filepath" parsed by the compiler, a second file "filepath.syntax_tree" is generated.
     - ```--emit_bytecode``` outputs a visual representation of the resulting Lax executable for the sake of debugging. Next to the Lax executable "output_filepath", a second file "output_filepath.bytecode" is generated.
 - The "lax" virtual machine takes only one argument, which is the name of the Lax executable to be run. (Terminal/ command-line arguments have not been implemented yet.)

There are two other example programs in the main directory of the repository: "pascal.lax" and "prime_numbers.lax". Feel free to try them out and look into the source code in order to get better acquainted with the language's syntax.

