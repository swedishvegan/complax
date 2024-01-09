#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cstdint>

// This file defines the instruction set used by the VM

using instruction = int64_t;

namespace Eval {

    /*
    
    Address format: [address code] [address args...]
    
    Thus, addresses have variable length, but always have length at least two.
    
    */

    enum access: instruction {

        none,     // Special value used to indicate that access type is 
        local,    // Arguments: 1,    Address = stack + SP + arg1
        global,   // Arguments: 1,    Address = stack + arg1
        imm,      // Arguments: 1,    Address = arg1
        heap1ll,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + (stack + SP + arg2)
        heap1gl,  // Arguments: 2,    Address = heap + (stack + arg1) + (stack + SP + arg2)
        heap1lg,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + (stack + arg2)
        heap1gg,  // Arguments: 2,    Address = heap + (stack + arg1) + (stack + arg2)
        heap1li,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + arg2
        heap1gi,  // Arguments: 2,    Address = heap + (stack + arg1) + arg2
        heap8ll,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + 8 * (stack + SP + arg2)
        heap8gl,  // Arguments: 2,    Address = heap + (stack + arg1) + 8 * (stack + SP + arg2)
        heap8lg,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + 8 * (stack + arg2)
        heap8gg,  // Arguments: 2,    Address = heap + (stack + arg1) + 8 * (stack + arg2)
        heap8li,  // Arguments: 2,    Address = heap + (stack + SP + arg1) + 8 * arg2
        heap8gi   // Arguments: 2,    Address = heap + (stack + arg1) + 8 * arg2

    };

    /*
    
    Within this documentation:

        * arg(k) = the kth argument of the instruction
        * addr(k) = also the kth argument, but in the format of an address, 
          where each address follows one of the above formats
        
    As mentioned above, most instructions have variable length.

    */
    enum inst: instruction {

        exit,     // Arguments: 0,    Purpose: exits the program with code 0
        begfnc,   // Arguments: 1,    Purpose: marks the beginning of a function; arg1 is used by the JIT
        j,        // Arguments: 1,    Purpose: jumps to instruction arg1
        jc,       // Arguments: 2,    Purpose: jumps to instruction arg1 if addr2 is false
        call,     // Arguments: 5,    Purpose: advances the SP by arg1, jumps to arg2, writes the old SP in SP, and writes the old IP in SP + 1; args 3-5 are used by the JIT
        ret,      // Arguments: 0,    Purpose: restores the IP to SP + 1 and the SP to SP
        scpbeg,   // Arguments: 1,    Purpose: marks the beginning of a new scope for use by the JIT -- arg1 is the number of local variables visible in the scope
        scpend,   // Arguments: 0,    Purpose: marks the end of a new scope for use by the JIT
        inp,      // Arguments: 1,    Purpose: gets user input and allocates a new string for the input; stores the address of the string in addr1
        out,      // Arguments: 1,    Purpose: outputs the value in addr1
        tmrs,     // Arguments: 0,    Purpose: starts a timer
        tmrr,     // Arguments: 1,    Purpose: reads the value of the timer into addr1
        rand,     // Arguments: 1,    Purpose: writes a random integer into addr1
        //gc,       // Arguments: 1+,   Purpose: checks whether garbage collection is necessary; if it is, each object SP + arg_{2K - 1} with type SP + arg_{2K} is registered as a root object in the garbage collector for each K from 1 to arg0; then the gc() function is called
        halloc,   // Arguments: 2,    Purpose: allocates addr1 bytes on the heap and stores the resulting pointer in addr2
        hinit,    // Arguments: 2,    Purpose: allocates 8 * addr1 bytes on the heap and stores the resulting pointer in addr2; zeroes the newly created memory
        copy,     // Arguments: 3,    Purpose: copies arg1 bytes from the memory at addr2 to the memory pointed to by addr3
        mov,      // Arguments: 2,    Purpose: moves the value at addr1 to addr2
        strlen,   // Arguments: 2,    Purpose: gets the length of the string at addr1 and stores the value in addr2
        arrlen,   // Arguments: 2,    Purpose: gets the length of the heap array pointed to by addr1 and stores the value in addr2
        arrcat,   // Arguments: 3,    Purpose: concatenates the array at addr1 with the array at addr2 and stores the result in addr3
        castid,   // Arguments: 2,    Purpose: casts addr1 to a decimal and stores its value in addr2
        castdi,   // Arguments: 2,    Purpose: casts addr1 to an integer and stores its value in addr2
        castsi,   // Arguments: 2,    Purpose: casts addr1 to an integer and stores its value in addr2
        castis,   // Arguments: 2,    Purpose: casts addr1 to a string and stores its value in addr2
        castsd,   // Arguments: 2,    Purpose: casts addr1 to a decimal and stores its value in addr2
        castds,   // Arguments: 2,    Purpose: casts addr1 to a string and stores its value in addr2
        castas,   // Arguments: 2,    Purpose: casts addr1 to a string and stores its value in addr2
        castai,   // Arguments: 2,    Purpose: casts addr1 to an integer and stores its value in addr2
        castia,   // Arguments: 2,    Purpose: casts addr1 to ascii and stores its value in addr2
        addi,     // Arguments: 3,    Purpose: adds addr1 to addr2 and stores the result in addr3
        addd,     // Arguments: 3,    Purpose: adds addr1 to addr2 and stores the result in addr3
        adda,     // Arguments: 3,    Purpose: adds the ascii characters addr1 and addr2 and stores the result in addr3
        subi,     // Arguments: 3,    Purpose: subtracts addr1 and addr2 and stores the result in addr3
        subd,     // Arguments: 3,    Purpose: subtracts addr1 and addr2 and stores the result in addr3
        suba,     // Arguments: 3,    Purpose: subtracts the ascii characters addr1 and addr2 and stores the result in addr3
        muli,     // Arguments: 3,    Purpose: multiplies addr1 by addr2 and stores the result in addr3
        muld,     // Arguments: 3,    Purpose: multiplies addr1 by addr2 and stores the result in addr3
        divi,     // Arguments: 3,    Purpose: divides addr1 by addr2 and stores the result in addr3
        divd,     // Arguments: 3,    Purpose: divides addr1 by addr2 and stores the result in addr3
        andi,     // Arguments: 3,    Purpose: performs logical & with addr1 and addr2 and stores the result in addr3
        andb,     // Arguments: 3,    Purpose: performs logical & with addr1 and addr2 and stores the result in addr3
        ori,      // Arguments: 3,    Purpose: performs logical | with addr1 and addr2 and stores the result in addr3
        orb,      // Arguments: 3,    Purpose: performs logical | with addr1 and addr2 and stores the result in addr3
        modi,     // Arguments: 3,    Purpose: performs % operation with addr1 and addr2 and stores the result in addr3
        powi,     // Arguments: 3,    Purpose: performs integer exponentiation with addr1 and addr2 and stores the result in addr3
        powd,     // Arguments: 3,    Purpose: performs decimal exponentiation with addr1 and addr2 and stores the result in addr3
        adds,     // Arguments: 3,    Purpose: concatenates the strings addr1 and addr2 and stores the result in addr3
        eqi,      // Arguments: 3,    Purpose: evaluates addr1 == addr2 and stores the result in addr3
        eqd,      // Arguments: 3,    Purpose: evaluates addr1 == addr2 and stores the result in addr3
        eqb,      // Arguments: 3,    Purpose: evaluates addr1 == addr2 and stores the result in addr3
        eqa,      // Arguments: 3,    Purpose: evaluates addr1 == addr2 and stores the result in addr3
        neqi,     // Arguments: 3,    Purpose: evaluates addr1 != addr2 and stores the result in addr3
        neqd,     // Arguments: 3,    Purpose: evaluates addr1 != addr2 and stores the result in addr3
        neqb,     // Arguments: 3,    Purpose: evaluates addr1 != addr2 and stores the result in addr3
        neqa,     // Arguments: 3,    Purpose: evaluates addr1 != addr2 and stores the result in addr3
        gti,      // Arguments: 3,    Purpose: evaluates addr1 > addr2 and stores the result in addr3
        gtd,      // Arguments: 3,    Purpose: evaluates addr1 > addr2 and stores the result in addr3
        gta,      // Arguments: 3,    Purpose: evaluates addr1 > addr2 and stores the result in addr3
        gtei,     // Arguments: 3,    Purpose: evaluates addr1 >= addr2 and stores the result in addr3
        gted,     // Arguments: 3,    Purpose: evaluates addr1 >= addr2 and stores the result in addr3
        gtea,     // Arguments: 3,    Purpose: evaluates addr1 >= addr2 and stores the result in addr3
        lti,      // Arguments: 3,    Purpose: evaluates addr1 < addr2 and stores the result in addr3
        ltd,      // Arguments: 3,    Purpose: evaluates addr1 < addr2 and stores the result in addr3
        lta,      // Arguments: 3,    Purpose: evaluates addr1 < addr2 and stores the result in addr3
        ltei,     // Arguments: 3,    Purpose: evaluates addr1 <= addr2 and stores the result in addr3
        lted,     // Arguments: 3,    Purpose: evaluates addr1 <= addr2 and stores the result in addr3
        ltea,     // Arguments: 3,    Purpose: evaluates addr1 <= addr2 and stores the result in addr3
        eqs,      // Arguments: 3,    Purpose: evaluates addr1 == addr2 and stores the result in addr3
        neqs,     // Arguments: 3,    Purpose: evaluates addr1 != addr2 and stores the result in addr3
        gts,      // Arguments: 3,    Purpose: evaluates addr1 > addr2 and stores the result in addr3
        gtes,     // Arguments: 3,    Purpose: evaluates addr1 >= addr2 and stores the result in addr3
        lts,      // Arguments: 3,    Purpose: evaluates addr1 < addr2 and stores the result in addr3
        ltes,     // Arguments: 3,    Purpose: evaluates addr1 <= addr2 and stores the result in addr3
        noti,     // Arguments: 2,    Purpose: performs logical !  with addr1 and stores the result in addr2
        notb,     // Arguments: 2,    Purpose: performs logical !  with addr1 and stores the result in addr2
        _size     // Total number of instructions

    };

    extern const int inst_lengths[];
    extern const int inst_info[];
    extern int inst_info_offsets[];
    extern bool inst_initialized;
    extern const char* inst_names[];
    extern const char* access_names[];
    extern void inst_init();

}

#endif