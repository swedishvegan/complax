#ifndef SYMBOLSEARCHTREEBASE_HPP
#define SYMBOLSEARCHTREEBASE_HPP

#include "./../../util/Printable.hpp"
#include "./../../util/ptr.hpp"

// Just a way to get around a really annoying issue with mutual inclusions

namespace AST {

    struct SymbolSearchTreeBase : public Printable { };
    using ptr_SymbolSearchTreeBase = ptr<SymbolSearchTreeBase>;

}

#endif