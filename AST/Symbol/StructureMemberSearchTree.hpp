#ifndef STRUCTUREMEMBERSEARCHTREE_HPP
#define STRUCTUREMEMBERSEARCHTREE_HPP

#include "./SymbolTableLinker.hpp"
#include "./SearchTree.hpp"

// SearchTree implementation for structure member lookup

namespace AST {

    struct StructureMemberInfo {

        managed_string name;
        ptr_HeaderSymbol owner;
        ptr_Symbol var;

    };

    using ptr_StructureMemberInfo = ptr<StructureMemberInfo>;

    struct StructureMemberInfoBundle : public Bundle<StructureMemberInfo> { managed_string* getSignature(); };

    using ptr_StructureMemberInfoBundle = ptr<StructureMemberInfoBundle>;

    struct StructureMemberSearchTree : public SearchTree<StructureMemberInfo, StructureMemberInfoBundle> {

        StructureMemberSearchTree(int depth);

        StructureMemberSearchTree(SymbolTableLinker, int depth);

        void addStructure(ptr_HeaderSymbol structure);

    protected:

    };

    using ptr_StructureMemberSearchTree = ptr<StructureMemberSearchTree>;
}

template <>
managed_string& AST::SearchTree<AST::StructureMemberInfo, AST::StructureMemberInfoBundle>::getSignature(ptr<StructureMemberInfo> sminfo);

template <>
managed_string& AST::SearchTree<AST::StructureMemberInfo, AST::StructureMemberInfoBundle>::getIdentifier(ptr<StructureMemberInfo> sminfo);

#endif