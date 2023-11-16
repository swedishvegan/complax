#include "./SymbolSearchTree.hpp"

AST::SymbolSearchList* AST::SymbolSearchTree::matches[AST_SEARCH_TREE_MAX_DEPTH];
int AST::SymbolSearchTree::num_matches;

AST::SymbolSearchTree::SymbolSearchTree(int depth, bool bundle_symbols) 
    : depth(depth > AST_SEARCH_TREE_MAX_DEPTH ? AST_SEARCH_TREE_MAX_DEPTH : depth), bundle_symbols(bundle_symbols) { 
    
    root = new SymbolSearchNode{ };
    buildTree(root(), 0, AST_INITIAL_CHILDREN_PER_SEARCH_NODE); 
    
}

AST::SymbolSearchTree::SymbolSearchTree(SymbolTableLinker symbols, int depth, bool bundle_symbols) 
    : depth(depth > AST_SEARCH_TREE_MAX_DEPTH ? AST_SEARCH_TREE_MAX_DEPTH : depth), bundle_symbols(bundle_symbols) {
    
    root = new SymbolSearchNode{ };
    buildTree(root(), 0, AST_INITIAL_CHILDREN_PER_SEARCH_NODE);

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

void AST::SymbolSearchTree::add(ptr_Symbol sym) {
    
    auto& name = bundle_symbols ? sym->operator[](sym.cast<HeaderSymbol>()->filler_indices[0]).name : sym->operator[](0).name;
    auto node = root();

    for (int i = 0; i < depth; i++) {
        
        if (i == (int)name.size()) break;
        node = &node->children[(int)name[i] % node->num_children];
        
    }

    ptr_SymbolBundle match;

    if (bundle_symbols)
        for (auto bundle : node->bundles)
            if (bundle->syms.size() > 0)
                if (sym->getSignature() == *bundle->getSignature()) { match = bundle; break; }

    if (match != nullptr) {
        
        match->syms.push_back(sym);
        match->needs_precedence_check = true;

    }
    else {

        int bundle_idx = node->bundles.size();
        
        auto new_bundle = new SymbolBundle{ };

        new_bundle->syms.push_back(sym);
        new_bundle->test_kw = name;

        node->bundles.push_back(new_bundle);

    }

    size++;

}

AST::SymbolSearchTree::Iterator AST::SymbolSearchTree::iterate(const char* search_hint) { 

    findMatches(search_hint);    
    return Iterator(search_hint); 
    
}

AST::SymbolSearchTree::SymbolSearchNode::~SymbolSearchNode() { if (children) delete[] children; }

string AST::SymbolSearchTree::toString(int alignment) {

    if (size == 0) return "SymbolSearchTree (empty)";

    string s = "SymbolSearchTree:\n";
    s += printNode(root(), alignment + 1);

    return s.substr(0, s.size() - 1);

}
            
AST::SymbolSearchTree::Iterator::Iterator(const char* search_hint) {

    if (num_matches > 0) {

        cur_search_list = matches[0];
        cur_bundle = cur_search_list->operator[](0);

    }

}

void AST::SymbolSearchTree::buildTree(SymbolSearchNode* node, int cur_depth, int cur_num_children) {

    if (cur_depth == depth) return;

    if (cur_num_children < 2) cur_num_children = 2;

    node->num_children = cur_num_children;
    node->children = new SymbolSearchNode[cur_num_children];

    for (int i = 0; i < cur_num_children; i++) buildTree(node->children + i, cur_depth + 1, cur_num_children / 2);
    
}

string AST::SymbolSearchTree::printNode(SymbolSearchNode* node, int alignment) {

    string s;

    for (auto& b : node->bundles) s += indent(alignment) + ((b->syms.size() == 0) ? "Bundle (empty)" : ("Bundle (size " + std::to_string(b->syms.size()) + "): " + b->firstSym()->getSignature().c_str())) + "\n";
    for (int i = 0; i < node->num_children; i++) s += printNode(node->children + i, alignment);

    return s;

}

AST::PatternMatchSearchTree::PatternMatchSearchTree(int depth) : SymbolSearchTree(depth, true) { }

AST::PatternMatchSearchTree::PatternMatchSearchTree(SymbolTableLinker symbols, int depth) : SymbolSearchTree(symbols, depth, true) { }

AST::VariableSearchTree::VariableSearchTree(int depth) : SymbolSearchTree(depth, false) { }

AST::VariableSearchTree::VariableSearchTree(SymbolTableLinker symbols, int depth) : SymbolSearchTree(symbols, depth, false) { }