#ifndef SYMBOLSEARCHTREE_HPP
#define SYMBOLSEARCHTREE_HPP

#include "./SymbolTableLinker.hpp"
#include "./SearchTree.hpp"

// SearchTree implementations for symbol lookup

namespace AST {

    struct SymbolBundle : public Bundle<Symbol> {

        int conflict_first = -1, conflict_second = -1; // Used elsewhere for detecting precedence conflicts
        
        managed_string* getSignature();

    };

    using ptr_SymbolBundle = ptr<SymbolBundle>;

    struct SymbolSearchTree : public SearchTree<Symbol, SymbolBundle> {

        SymbolSearchTree(SearchTreeID, int depth, bool bundle_symbols);

        SymbolSearchTree(SearchTreeID, SymbolTableLinker, int depth, bool bundle_symbols);

    protected:

    };

    using ptr_SymbolSearchTree = ptr<SymbolSearchTree>;

    struct PatternMatchSearchTree : public SymbolSearchTree {

        PatternMatchSearchTree(int depth);

        PatternMatchSearchTree(SymbolTableLinker, int depth);

    };

    using ptr_PatternMatchSearchTree = ptr<PatternMatchSearchTree>;

    struct VariableSearchTree : public SymbolSearchTree {

        SymbolTableLinker local_symbols; // Set within Builder implementations

        VariableSearchTree(int depth);

        VariableSearchTree(SymbolTableLinker, int depth);

    };

    using ptr_VariableSearchTree = ptr<VariableSearchTree>;

}

template <>
managed_string& AST::SearchTree<AST::Symbol, AST::SymbolBundle>::getSignature(ptr<Symbol> sym);

template <>
managed_string& AST::SearchTree<AST::Symbol, AST::SymbolBundle>::getIdentifier(ptr<Symbol> sym);

#endif