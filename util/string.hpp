#ifndef STRING_HPP
#define STRING_HPP

#include <string>
//#include "./allocate.hpp"

// Memory-managed strings

using managed_string = std::string;
//using managed_string = std::basic_string<char, std::char_traits<char>, allocator<char>>; // There used to be a custom allocator but I got rid of it because I did not observe any benefits after switching to it

using string = std::string; // Regular string

#endif