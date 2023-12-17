#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cstdint>

// This file defines the instruction set used by the VM

using instruction = int64_t;

namespace Eval {

    enum inst: instruction {

        exit,     // Arguments: 0,    Purpose: exits the program with code 0
        sass,     // Arguments: 1,    Purpose: asserts that growing the stack by arg0 does not exceed the maximum stack size; exits the program with code 1 if it does
        j,        // Arguments: 1,    Purpose: jumps to instruction arg0
        jc,       // Arguments: 2,    Purpose: jumps to instruction arg0 if SP + arg1 is true
        call,     // Arguments: 2,    Purpose: advances the SP by arg0, jumps to arg1, writes the old SP in SP, and writes the old IP in SP + 1
        ret,      // Arguments: 0,    Purpose: restores the IP to SP + 1 and the SP to SP
        inp,      // Arguments: 1,    Purpose: gets user input and allocates a new string for the input; stores the address of the string in SP + arg0
        out,      // Arguments: 1,    Purpose: outputs the value in SP + arg0
        tmrs,     // Arguments: 0,    Purpose: starts a timer
        tmrr,     // Arguments: 1,    Purpose: reads the value of the timer into SP + arg0
        rand,     // Arguments: 1,    Purpose: writes a random integer into SP + arg0
        gc,       // Arguments: 1+,   Purpose: checks whether garbage collection is necessary; if it is, each object SP + arg_{2K - 1} with type SP + arg_{2K} is registered as a root object in the garbage collector for each K from 1 to arg0; then the gc() function is called
        halloc,   // Arguments: 2,    Purpose: allocates SP + arg0 bytes on the heap and stores the resulting pointer in SP + arg1
        hinit,    // Arguments: 2,    Purpose: same as halloc, but zeroes the newly created memory
        lhcpy,    // Arguments: 3,    Purpose: copies SP + arg0 values starting at SP + arg1 to the heap location pointed to by SP + arg2
        hhcpy,    // Arguments: 3,    Purpose: copies SP + arg0 values from the heap location pointed to by SP + arg1 to the heap location pointed to by SP + arg2
        movll,    // Arguments: 2,    Purpose: moves the 8-byte value at SP + arg0 to SP + arg1
        movpl,    // Arguments: 2,    Purpose: moves the 8-byte value arg0 to SP + arg1
        movxl,    // Arguments: 2,    Purpose: moves the 8-byte value at arg0 to SP + arg1
        movhl,    // Arguments: 3,    Purpose: moves element SP + arg0 of the heap value pointed to by SP + arg1 to SP + arg2
        movlx,    // Arguments: 2,    Purpose: moves the 8-byte value at SP + arg0 to arg1
        movpx,    // Arguments: 2,    Purpose: moves the 8-byte value arg0 to arg1
        movxx,    // Arguments: 2,    Purpose: moves the 8-byte value at arg0 to arg1
        movhx,    // Arguments: 3,    Purpose: moves element SP + arg0 of the heap value pointed to by SP + arg1 to arg2
        movlh,    // Arguments: 3,    Purpose: moves the 8-byte value at SP + arg0 to element SP + arg1 of the heap value pointed to by SP + arg2
        movph,    // Arguments: 3,    Purpose: moves the 8-byte value arg0 to element SP + arg1 of the heap value pointed to by SP + arg2
        movxh,    // Arguments: 3,    Purpose: moves the 8-byte value at arg0 to element SP + arg1 of the heap value pointed to by SP + arg2
        movhh,    // Arguments: 4,    Purpose: moves element SP + arg0 of the heap value pointed to by SP + arg1 to element SP + arg2 of the heap value pointed to by SP + arg3
        strlen,   // Arguments: 2,    Purpose: gets the length of the string SP + arg0 and stores the value in SP + arg1
        arrlen,   // Arguments: 2,    Purpose: gets the length of the heap array pointed to by SP + arg0 and stores the value in SP + arg1
        arrcat,   // Arguments: 3,    Purpose: concatenates the array SP + arg0 with the array SP + arg1 and stores the result in SP + arg2
        castid,   // Arguments: 2,    Purpose: casts SP + arg0 to a decimal and stores its value in SP + arg1
        castdi,   // Arguments: 2,    Purpose: casts SP + arg0 to an integer and stores its value in SP + arg1
        castib,   // Arguments: 2,    Purpose: casts SP + arg0 to a bool and stores its value in SP + arg1
        castbi,   // Arguments: 2,    Purpose: casts SP + arg0 to an integer and stores its value in SP + arg1
        castdb,   // Arguments: 2,    Purpose: casts SP + arg0 to a bool and stores its value in SP + arg1
        castbd,   // Arguments: 2,    Purpose: casts SP + arg0 to a double and stores its value in SP + arg1
        castsi,   // Arguments: 2,    Purpose: casts SP + arg0 to an integer and stores its value in SP + arg1
        castis,   // Arguments: 2,    Purpose: casts SP + arg0 to a string and stores its value in SP + arg1
        castsd,   // Arguments: 2,    Purpose: casts SP + arg0 to a decimal and stores its value in SP + arg1
        castds,   // Arguments: 2,    Purpose: casts SP + arg0 to a string and stores its value in SP + arg1
        castsb,   // Arguments: 2,    Purpose: casts SP + arg0 to a bool and stores its value in SP + arg1
        castbs,   // Arguments: 2,    Purpose: casts SP + arg0 to a string and stores its value in SP + arg1
        castas,   // Arguments: 2,    Purpose: casts SP + arg0 to a string and stores its value in SP + arg1
        castai,   // Arguments: 2,    Purpose: casts SP + arg0 to an integer and stores its value in SP + arg1
        castia,   // Arguments: 2,    Purpose: casts SP + arg0 to ascii and stores its value in SP + arg1
        addi,     // Arguments: 3,    Purpose: adds SP + arg0 to SP + arg1 and stores the result in SP + arg2
        addd,     // Arguments: 3,    Purpose: adds SP + arg0 to SP + arg1 and stores the result in SP + arg2
        adda,     // Arguments: 3,    Purpose: adds the ascii characters SP + arg0 and SP + arg1 and stores the result in SP + arg2
        subi,     // Arguments: 3,    Purpose: subtracts SP + arg0 and SP + arg1 and stores the result in SP + arg2
        subd,     // Arguments: 3,    Purpose: subtracts SP + arg0 and SP + arg1 and stores the result in SP + arg2
        suba,     // Arguments: 3,    Purpose: subtracts the ascii characters SP + arg0 and SP + arg1 and stores the result in SP + arg2
        muli,     // Arguments: 3,    Purpose: multiplies SP + arg0 by SP + arg1 and stores the result in SP + arg2
        muld,     // Arguments: 3,    Purpose: multiplies SP + arg0 by SP + arg1 and stores the result in SP + arg2
        divi,     // Arguments: 3,    Purpose: divides SP + arg0 by SP + arg1 and stores the result in SP + arg2
        divd,     // Arguments: 3,    Purpose: divides SP + arg0 by SP + arg1 and stores the result in SP + arg2
        andi,     // Arguments: 3,    Purpose: performs logical & with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        andb,     // Arguments: 3,    Purpose: performs logical & with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        ori,      // Arguments: 3,    Purpose: performs logical | with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        orb,      // Arguments: 3,    Purpose: performs logical | with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        modi,     // Arguments: 3,    Purpose: performs % operation with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        powi,     // Arguments: 3,    Purpose: performs integer exponentiation with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        powd,     // Arguments: 3,    Purpose: performs decimal exponentiation with SP + arg0 and SP + arg1 and stores the result in SP + arg2
        adds,     // Arguments: 3,    Purpose: concatenates the strings SP + arg0 and SP + arg1 and stores the result in SP + arg2
        eqi,      // Arguments: 3,    Purpose: evaluates SP + arg0 == SP + arg1 and stores the result in SP + arg2
        eqd,      // Arguments: 3,    Purpose: evaluates SP + arg0 == SP + arg1 and stores the result in SP + arg2
        eqb,      // Arguments: 3,    Purpose: evaluates SP + arg0 == SP + arg1 and stores the result in SP + arg2
        eqa,      // Arguments: 3,    Purpose: evaluates SP + arg0 == SP + arg1 and stores the result in SP + arg2
        neqi,     // Arguments: 3,    Purpose: evaluates SP + arg0 != SP + arg1 and stores the result in SP + arg2
        neqd,     // Arguments: 3,    Purpose: evaluates SP + arg0 != SP + arg1 and stores the result in SP + arg2
        neqb,     // Arguments: 3,    Purpose: evaluates SP + arg0 != SP + arg1 and stores the result in SP + arg2
        neqa,     // Arguments: 3,    Purpose: evaluates SP + arg0 != SP + arg1 and stores the result in SP + arg2
        gti,      // Arguments: 3,    Purpose: evaluates SP + arg0 > SP + arg1 and stores the result in SP + arg2
        gtd,      // Arguments: 3,    Purpose: evaluates SP + arg0 > SP + arg1 and stores the result in SP + arg2
        gta,      // Arguments: 3,    Purpose: evaluates SP + arg0 > SP + arg1 and stores the result in SP + arg2
        gtei,     // Arguments: 3,    Purpose: evaluates SP + arg0 >= SP + arg1 and stores the result in SP + arg2
        gted,     // Arguments: 3,    Purpose: evaluates SP + arg0 >= SP + arg1 and stores the result in SP + arg2
        gtea,     // Arguments: 3,    Purpose: evaluates SP + arg0 >= SP + arg1 and stores the result in SP + arg2
        lti,      // Arguments: 3,    Purpose: evaluates SP + arg0 < SP + arg1 and stores the result in SP + arg2
        ltd,      // Arguments: 3,    Purpose: evaluates SP + arg0 < SP + arg1 and stores the result in SP + arg2
        lta,      // Arguments: 3,    Purpose: evaluates SP + arg0 < SP + arg1 and stores the result in SP + arg2
        ltei,     // Arguments: 3,    Purpose: evaluates SP + arg0 <= SP + arg1 and stores the result in SP + arg2
        lted,     // Arguments: 3,    Purpose: evaluates SP + arg0 <= SP + arg1 and stores the result in SP + arg2
        ltea,     // Arguments: 3,    Purpose: evaluates SP + arg0 <= SP + arg1 and stores the result in SP + arg2
        eqs,      // Arguments: 3,    Purpose: evaluates SP + arg0 == SP + arg1 and stores the result in SP + arg2
        neqs,     // Arguments: 3,    Purpose: evaluates SP + arg0 != SP + arg1 and stores the result in SP + arg2
        gts,      // Arguments: 3,    Purpose: evaluates SP + arg0 > SP + arg1 and stores the result in SP + arg2
        gtes,     // Arguments: 3,    Purpose: evaluates SP + arg0 >= SP + arg1 and stores the result in SP + arg2
        lts,      // Arguments: 3,    Purpose: evaluates SP + arg0 < SP + arg1 and stores the result in SP + arg2
        ltes,     // Arguments: 3,    Purpose: evaluates SP + arg0 <= SP + arg1 and stores the result in SP + arg2
        noti,     // Arguments: 2,    Purpose: performs logical !  with SP + arg0 and stores the result in SP + arg1
        notb,     // Arguments: 2,    Purpose: performs logical !  with SP + arg0 and stores the result in SP + arg1
        _size,    // Total number of instructions

    };

}

#endif