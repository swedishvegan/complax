#ifndef BYTECODEBLOCK_HPP
#define BYTECODEBLOCK_HPP

#include "./Instruction.hpp"
#include "./Address.hpp"
#include "./../util/vec.hpp"
#include "./../util/map.hpp"
#include "./../util/ptr.hpp"
#include "./../util/Printable.hpp"

// BytecodeBlocks are used as auxilliary objects in compilation to store compiled code for each instantiated code body

#define I(i) ((instruction)i)                   // Casts an integer type to an instruction
#define I_dirty_cast(x) (*(instruction*)(&(x))) // Converts any 8-byte primitive type to an instruction 

namespace Eval {

    struct BytecodeBlock : public Printable { // Block of instructions, to be linked with other blocks later

        managed_vec<instruction> instructions;

        int global_index = 0;     // Global index within complete program bytecode; computed towards the end of compilation for calculating jump indices

        BytecodeBlock();

        void reset(); // Self-explanatory

        void mergeWith(BytecodeBlock&); // Appends the instructions in the BytecodeBlock passed as an argument to the instructions in this block
        
        void call(void* ref, int num_args); // Wrapper around the "call" instruction that automatically inserts a hanging jump; ref is an AST::HeaderSymbol::InstantiationInfo object representing the specific instantiation of the function to be called

        template <typename... Args>               // I() and I_dirty_cast can be used to supply arguments with the proper type
        void addInstruction(instruction in, Args... args) { ibuf.push_back(I(in)); addInstruction(args...); }

        template <typename... Args>
        void addInstruction(Address addr, Args... args) { 
            
            ibuf.push_back(I(addr.access_type));
            ibuf.push_back(addr.args[0]);
            if (addr.num_args == 2) ibuf.push_back(addr.args[1]);

            addInstruction(args...);
            
        }

        void addInstruction(instruction in);

        void addInstruction(Address addr);
       
        struct HangingJump {     // The compiler doesn't know ahead of time where the jump locations will be in memory, so it holds on to these references and fills in the details later once every BytecodeBlock is compiled

            int offset = 0;            // Index of the instruction that needs to be modified relative to this block
            int jump_value_offset = 0; // Index of the actual value that needs to be modified relative to the instruction

            void* ref = nullptr;       // AST::HeaderSymbol::InstantiationInfo* object that the jump statement jumps to
                                       // If the jump is local (within this block) then ref is left as nullptr

            int jdest = 0;             // Index of the jump destination within this BytecodeBlock; only applies for local jumps (i.e., if ref is nullptr)
                                       // If jdest is -1 then the jump should be to the beginning of the current while loop (continue)
                                       // If jdest is -2 then the jump should escape the current while loop (break)

        };

        managed_vec<HangingJump> jumps;

        string toString(int alignment);

    protected:

        managed_vec<instruction> ibuf;    // Helper object

    };

    using ptr_BytecodeBlock = ptr<BytecodeBlock>;

}

#endif