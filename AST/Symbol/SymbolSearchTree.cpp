#include "./SymbolSearchTree.hpp"

managed_string* AST::SymbolBundle::getSignature() { return (elements.size() == 0) ? nullptr : &elements[0]->getSignature(); }

AST::SymbolSearchTree::SymbolSearchTree(SearchTreeID ID, int depth, bool bundle_symbols)
    : SearchTree<Symbol, SymbolBundle>(ID, depth, bundle_symbols) { }

AST::SymbolSearchTree::SymbolSearchTree(SearchTreeID ID, SymbolTableLinker symbols, int depth, bool bundle_symbols) 
    : SearchTree<Symbol, SymbolBundle>(ID, depth, bundle_symbols) {

#define _addSymbolTypes(SID) \
\
auto it_##SID = symbols.iterate(SymbolID::SID); \
\
while (!it_##SID.finished()) { \
    \
    add(it_##SID()); \
    it_##SID = it_##SID.next(); \
    \
}

    if (bundle_symbols) {

        _addSymbolTypes(Function);
        _addSymbolTypes(Structure);

    }
    else { _addSymbolTypes(Variable); }

}

template <>
managed_string& AST::SearchTree<AST::Symbol, AST::SymbolBundle>::getSignature(ptr<Symbol> sym) { return sym->getSignature(); }

template <>
managed_string& AST::SearchTree<AST::Symbol, AST::SymbolBundle>::getIdentifier(ptr<Symbol> sym) { return (sym->size() > 1) ? sym->operator[](sym.cast<HeaderSymbol>()->filler_indices[0]).name : sym->operator[](0).name; }

AST::PatternMatchSearchTree::PatternMatchSearchTree(int depth) : SymbolSearchTree(SearchTreeID::PatternMatch, depth, true) { }

AST::PatternMatchSearchTree::PatternMatchSearchTree(SymbolTableLinker symbols, int depth) : SymbolSearchTree(SearchTreeID::PatternMatch, symbols, depth, true) { }

AST::VariableSearchTree::VariableSearchTree(int depth) : SymbolSearchTree(SearchTreeID::Variable, depth, false) { }

AST::VariableSearchTree::VariableSearchTree(SymbolTableLinker symbols, int depth) : SymbolSearchTree(SearchTreeID::Variable, symbols, depth, false) { }