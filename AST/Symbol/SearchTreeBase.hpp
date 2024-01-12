#ifndef SEARCHTREEBASE_HPP
#define SEARCHTREEBASE_HPP

#include "./../../util/Printable.hpp"
#include "./../../util/ptr.hpp"

// Sometimes I really hate C++

namespace AST {

    enum class SearchTreeID {

        PatternMatch,
        Variable,
        StructureMember

    };

    struct SearchTreeBase : public Printable { 
        
        SearchTreeID ID;

        inline SearchTreeBase(SearchTreeID ID) : ID(ID) { }
        
    };

    using ptr_SearchTreeBase = ptr<SearchTreeBase>;

}

#endif