#ifndef PROGRAMDATATABLE_HPP
#define PROGRAMDATATABLE_HPP

#include <cstdint>
#include "./../util/map.hpp"
#include "./../util/vec.hpp"
#include "./../util/string.hpp"
#include "./../util/Printable.hpp"

using pointer = int64_t;

// Builds the data section of the executable, keeps track of global variables and built-in data (like strings and arrays)

namespace Eval {

    struct ProgramDataTable : public Printable {

        ProgramDataTable();

        pointer getStackPosition(void*, managed_string&); // Imbeds the given string from the given LiteralNode within the table; returns stack position where the newly added string is stored

        pointer getStackPosition(void*); // Returns the stack position of the given global variable pointer

        pointer size();

        char* data(); // Built-in strings and arrays; put at the beginning of the heap when the program starts

        managed_vec<pointer> initial_stack; // Global variables with their initial values at the bottom of the stack

        std::string toString(int alignment);

    protected:

        managed_vec<char> the_data;
        pointer count = 0; // Size of the_data
        pointer offset;    // Index of the first data element

        pointer next_avail_index = 0;

        struct StackType { int AST_Type = 0; bool is_variable = false; };
        managed_vec<StackType> initial_stack_types; // This is really only used for the toString() function

        managed_map<void*, pointer> global_variable_indices; // Maps every relevant global variable to an index within the stack

    };

}

#endif