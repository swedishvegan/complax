#ifndef MAP_HPP
#define MAP_HPP

#include <map>
//#include "./allocate.hpp"

// std::map specialization using custom memory allocator

template <typename K, typename V>
using managed_map = std::map<K, V>;
//using managed_map = std::map<K, V, std::less<K>, allocator<std::pair<K, V>>>;  // // There used to be a custom allocator but I got rid of it because I did not observe any benefits after switching to it

template <typename K, typename V>
using map = std::map<K, V>; // Regular std::map just in case it is ever needed

#endif