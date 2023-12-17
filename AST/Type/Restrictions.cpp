#include "Restrictions.hpp"
#include "./../Symbol/SymbolTable.hpp"

AST::Restrictions::Restrictions() { }

AST::TypeList& AST::Restrictions::operator [] (int arg_index) {
	
	TypeList* list = nullptr;

	auto find = rest_map.find(arg_index);

	if (find == rest_map.end()) {

		auto tlw = new TypeListWrapper{ };
		rest_map[arg_index] = tlw;
		
		return tlw->tl;
		
	}

	return find->second->tl;

}

string AST::Restrictions::toString(int alignment) {

	if (rest_map.size() == 0) return "Restrictions: (empty)";

	string s = "Restrictions:\n";

	for (auto& rest : rest_map) {

		s += indent(alignment + 1) + "Argument # " + std::to_string(rest.first) + ":\n";
		s += Type::fromTypeList(rest.second->tl).print(alignment + 2, false);

	}

	return s;

}

bool AST::Restrictions::isEmpty() { return rest_map.empty(); }