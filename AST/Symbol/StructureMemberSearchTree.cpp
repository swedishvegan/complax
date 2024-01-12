
#include "./StructureMemberSearchTree.hpp"
#include "./../Builder/Builder.hpp"
#include "./../Pattern/implementations/Declaration.hpp"

managed_string* AST::StructureMemberInfoBundle::getSignature() { return (elements.size() == 0) ? nullptr : &elements[0]->name; }

AST::StructureMemberSearchTree::StructureMemberSearchTree(int depth)
    : SearchTree<StructureMemberInfo, StructureMemberInfoBundle>(SearchTreeID::StructureMember, depth, true) { }

AST::StructureMemberSearchTree::StructureMemberSearchTree(SymbolTableLinker symbols, int depth) 
    : SearchTree<StructureMemberInfo, StructureMemberInfoBundle>(SearchTreeID::StructureMember, depth, true) {

    auto it = symbols.iterate(SymbolID::Structure);

	while (!it.finished()) {

        auto header_sym = it().cast<HeaderSymbol>();
		
		addStructure(header_sym);
		it = it.next();

	}

}

void AST::StructureMemberSearchTree::addStructure(ptr_HeaderSymbol structure) {

	auto& structure_varnames = ((Builder*)(structure->body))->patterns;

	for (auto& varname : structure_varnames) {

		if (varname->ID != PatternID::Declaration) continue;

		auto decl = varname.cast<Declaration>();
		auto var = decl->sym;
		auto& name = var->operator[](0).name;

		ptr_StructureMemberInfo sminfo = new StructureMemberInfo{ name, structure, var };

		add(sminfo);

	}

}

template <>
managed_string& AST::SearchTree<AST::StructureMemberInfo, AST::StructureMemberInfoBundle>::getSignature(ptr<StructureMemberInfo> sminfo) { return sminfo->name; }

template <>
managed_string& AST::SearchTree<AST::StructureMemberInfo, AST::StructureMemberInfoBundle>::getIdentifier(ptr<StructureMemberInfo> sminfo) { return sminfo->name; }
