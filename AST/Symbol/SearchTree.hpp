#ifndef SEARCHTREE_HPP
#define SEARCHTREE_HPP

#include "./../../util/vec.hpp"
#include "./../../util/Printable.hpp"
#include "./SearchTreeBase.hpp"

// Used to make lookups during expression parsing more efficient, base class for PatternMatchSearchTree and VariableSearchTree

#define AST_SEARCH_TREE_MAX_DEPTH 10
#define AST_INITIAL_CHILDREN_PER_SEARCH_NODE 8    // Must be a power of 2
#define AST_CHILDREN_PER_SEARCH_NODE_DECAY 2      // Must be a power of 2
                                                  // Top-level nodes have 8 children, next level have 4, etc. (but they will never have fewer than 2)
namespace AST {

    template <typename T>
    struct Bundle { // Used to keep track of symbols that all have the same "signature" (the meaning of signature varies depending on the Bundle implementation)
        
        managed_vec<ptr<T>> elements;
        managed_string test_kw; // Keyword that is used to match the code against this Bundle

        bool has_unacknowledged_updates = false; // Used by some specializations during expression parsing

        virtual managed_string* getSignature() = 0;
        
    };

    template <typename BundleType>
    using SearchList = managed_vec<ptr<BundleType>>;

    template <typename T, typename BundleType>
    struct SearchTree : public SearchTreeBase {
        
        int size = 0;         // Number of elements in the tree
        int depth;            // More depth = more efficiency, but more memory wasted
        bool bundle_elements; // Determines whether elements with the same signature are bundled together; if false then bundles are trivial and each only contain a single element
        
        SearchTree(SearchTreeID, int depth, bool bundle_elements);

        void add(ptr<T>);

        static managed_string& getSignature(ptr<T> element);  // Templated function must be specialized for implementation classes

        static managed_string& getIdentifier(ptr<T> element); // Above requirement also applies here

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

            ptr<BundleType> cur_bundle;

        protected:

            SearchList<BundleType>* cur_search_list = nullptr;
            
            int ssl_idx = 0;
            int bdl_idx = 0;

            friend struct SearchTree<T, BundleType>;

        };

    protected:

        struct SearchNode  {

            SearchNode* children = nullptr;
            int num_children = 0;

            SearchList<BundleType> bundles;

            ~SearchNode();

        };

        ptr<SearchNode> root;

        void buildTree(SearchNode*, int cur_depth, int cur_num_children);

        string printNode(SearchNode*, int);

        inline void findMatches(const char* letters);

        static SearchList<BundleType>* matches[AST_SEARCH_TREE_MAX_DEPTH];
        static int num_matches;

    };

}

template <typename T, typename BundleType>
AST::SearchList<BundleType>* AST::SearchTree<T, BundleType>::matches[AST_SEARCH_TREE_MAX_DEPTH];

template <typename T, typename BundleType>
int AST::SearchTree<T, BundleType>::num_matches;

template <typename T, typename BundleType>
AST::SearchTree<T, BundleType>::SearchTree(SearchTreeID ID, int depth, bool bundle_elements) 
    : SearchTreeBase(ID), depth(depth > AST_SEARCH_TREE_MAX_DEPTH ? AST_SEARCH_TREE_MAX_DEPTH : depth), bundle_elements(bundle_elements) { 
    
    root = new SearchNode{ };
    buildTree(root(), 0, AST_INITIAL_CHILDREN_PER_SEARCH_NODE); 
    
}

template <typename T, typename BundleType>
void AST::SearchTree<T, BundleType>::add(ptr<T> element) {
    
    auto& name = getIdentifier(element);
    auto node = root();

    for (int i = 0; i < depth; i++) {
        
        if (i == (int)name.size()) break;
        node = &node->children[(int)name[i] % node->num_children];
        
    }

    ptr<BundleType> match;

    if (bundle_elements)
        for (auto bundle : node->bundles)
            if (bundle->elements.size() > 0)
                if (getSignature(element) == *bundle->getSignature()) { match = bundle; break; }

    if (match != nullptr) {

        match->elements.push_back(element);
        match->has_unacknowledged_updates = true;

    }
    else {

        int bundle_idx = node->bundles.size();
        
        auto new_bundle = new BundleType{ };

        new_bundle->elements.push_back(element);
        new_bundle->test_kw = name;

        node->bundles.push_back(new_bundle);

    }

    size++;

}

template <typename T, typename BundleType>
managed_string& AST::SearchTree<T, BundleType>::getSignature(ptr<T> element) { static managed_string _dummy; return _dummy; }

template <typename T, typename BundleType>
managed_string& AST::SearchTree<T, BundleType>::getIdentifier(ptr<T> element) { static managed_string _dummy; return _dummy; }

template <typename T, typename BundleType>
typename AST::SearchTree<T, BundleType>::Iterator AST::SearchTree<T, BundleType>::iterate(const char* search_hint) { 

    findMatches(search_hint);    
    return Iterator(search_hint); 
    
}

template <typename T, typename BundleType>
string AST::SearchTree<T, BundleType>::toString(int alignment) {

    if (size == 0) return "SearchTree (empty)";

    string s = "SearchTree:\n";
    s += printNode(root(), alignment + 1);

    return s.substr(0, s.size() - 1);

}

template <typename T, typename BundleType>     
AST::SearchTree<T, BundleType>::Iterator::Iterator(const char* search_hint) {

    if (num_matches > 0) {

        cur_search_list = matches[0];
        cur_bundle = cur_search_list->operator[](0);

    }

}

template <typename T, typename BundleType>
inline void AST::SearchTree<T, BundleType>::Iterator::next() {

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

template <typename T, typename BundleType>
AST::SearchTree<T, BundleType>::SearchNode::~SearchNode() { if (children) delete[] children; }

template <typename T, typename BundleType>
void AST::SearchTree<T, BundleType>::buildTree(SearchNode* node, int cur_depth, int cur_num_children) {

    if (cur_depth == depth) return;

    if (cur_num_children < 2) cur_num_children = 2;

    node->num_children = cur_num_children;
    node->children = new SearchNode[cur_num_children];

    for (int i = 0; i < cur_num_children; i++) buildTree(node->children + i, cur_depth + 1, cur_num_children / 2);
    
}

template <typename T, typename BundleType>
string AST::SearchTree<T, BundleType>::printNode(SearchNode* node, int alignment) {

    string s;

    for (auto& b : node->bundles) s += indent(alignment) + ((b->elements.size() == 0) ? "Bundle (empty)" : ("Bundle (size " + std::to_string(b->elements.size()) + "): " + getSignature(b->elements[0]).c_str())) + "\n";
    for (int i = 0; i < node->num_children; i++) s += printNode(node->children + i, alignment);

    return s;

}

template <typename T, typename BundleType>
inline void AST::SearchTree<T, BundleType>::findMatches(const char* letters) {

    auto cur = root();
    num_matches = 0;

    for (int i = 0; i < depth; i++) {

        if (letters[i] == '}') break;

        cur = &cur->children[(int)letters[i] % cur->num_children];
        if (cur->bundles.size() > 0) { matches[num_matches] = &cur->bundles; num_matches++; }

    }

}

#endif