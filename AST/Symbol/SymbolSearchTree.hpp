#ifndef SYMBOLSEARCHTREE_HPP
#define SYMBOLSEARCHTREE_HPP

#include "./SymbolTableLinker.hpp"
#include "./SymbolSearchTreeBase.hpp"

// Used to make symbol search more efficient, base class for PatternMatchSearchTree and VariableSearchTree

#define AST_SEARCH_TREE_MAX_DEPTH 10
#define AST_INITIAL_CHILDREN_PER_SEARCH_NODE 8    // Must be a power of 2
#define AST_CHILDREN_PER_SEARCH_NODE_DECAY 2      // Must be a power of 2
                                                  // Top-level nodes have 8 children, next level have 4, etc. (but they will never have fewer than 2)
namespace AST {

    struct SymbolBundle  { // Bundle of Symbols that all have the same signature
        
        SymbolList syms;
        managed_string test_kw;

        bool needs_precedence_check = false;
        int conflict_first = -1, conflict_second = -1;

        inline managed_string* getSignature() { return (syms.size() == 0) ? nullptr : &syms[0]->getSignature(); }

        inline ptr_HeaderSymbol firstSym() { return syms[0].cast<HeaderSymbol>(); } // This is a dangerous function and should be used with care 
        
    };

    using ptr_SymbolBundle = ptr<SymbolBundle>;

    using SymbolSearchList = managed_vec<ptr_SymbolBundle>;

    struct SymbolSearchTree : public SymbolSearchTreeBase {
        
        int size = 0;        // Number of elements in the tree
        int depth;           // More depth = more efficiency, but more memory wasted
        bool bundle_symbols; // Determines whether symbols with the same signature are bundled together (true for PatternMatchSearchTree, false for VariableSearchTree)
        
        SymbolSearchTree(int depth, bool bundle_symbols);

        SymbolSearchTree(SymbolTableLinker, int depth, bool bundle_symbols);

        void add(ptr_Symbol);

        struct Iterator; friend struct Iterator;

        // The Iterator expects an array of depth or fewer characters
        // Note: If the array length is less than depth, a '}' should be appended (i.e. to find all potential matching patterns of "abc", you would enter "abc}" as an argument)
        // Note: Creating a second Iterator before a first Iterator is finished is undefined behavior

        Iterator iterate(const char* search_hint); // Example usage: auto it = tree.iterate("func}"); while (!it.finished()) { ... it.next(); }

        string toString(int alignment);

        struct Iterator {
            
            Iterator(const char* search_hint);

            inline const char* curKW() { return (cur_bundle != nullptr) ? cur_bundle->getSignature()->c_str() : nullptr; }

            inline void next();

            inline bool finished() { return cur_bundle == nullptr; }

            ptr_SymbolBundle cur_bundle;

        protected:

            SymbolSearchList* cur_search_list = nullptr;
            
            int ssl_idx = 0;
            int bdl_idx = 0;

            friend struct SymbolSearchTree;

        };

    protected:

        struct SymbolSearchNode;
        using ptr_SymbolSearchNode = ptr<SymbolSearchNode>;

        struct SymbolSearchNode  {

            SymbolSearchNode* children = nullptr;
            int num_children = 0;

            SymbolSearchList bundles;

            ~SymbolSearchNode();

        };

        ptr_SymbolSearchNode root;

        void buildTree(SymbolSearchNode*, int cur_depth, int cur_num_children);

        string printNode(SymbolSearchNode*, int);

        inline void findMatches(const char* letters);

        static SymbolSearchList* matches[AST_SEARCH_TREE_MAX_DEPTH];
        static int num_matches;

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

inline void AST::SymbolSearchTree::Iterator::next() {

    if (cur_bundle == nullptr) return;

    bdl_idx++;
    if (bdl_idx == cur_search_list->size()) {

        ssl_idx++;
        if (ssl_idx == num_matches) { cur_bundle = nullptr; return; }

        cur_search_list = matches[ssl_idx];
        bdl_idx = 0;

    }

    cur_bundle = cur_search_list->operator[](bdl_idx);

}

inline void AST::SymbolSearchTree::findMatches(const char* letters) {

    auto cur = root();
    num_matches = 0;

    for (int i = 0; i < depth; i++) {

        if (letters[i] == '}') break;

        cur = &cur->children[(int)letters[i] % cur->num_children];
        if (cur->bundles.size() > 0) { matches[num_matches] = &cur->bundles; num_matches++; }

    }

}

#endif