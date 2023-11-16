#include "./Type.hpp"
#include "./../Symbol/Symbol.hpp"

AST::PrintableTypeList::PrintableTypeList(TypeList type_list) : type_list(type_list) { }

string AST::PrintableTypeList::toString(int alignment) {

	if (type_list.size() == 0) return "TypeList (empty)";

	string s = "TypeList:\n";
	for (auto ID : type_list) s += TypeID_toString(ID, alignment + 1);

	return s.substr(0, s.size() - 1);

}

string AST::PrintableTypeList::TypeID_toString(TypeID ID, int alignment) {

	static const char* type_strings[] = {

		"Nothing",
		"Anything",

		"Integer", "Decimal",
		"Boolean", "String",

	};

	auto type = Type(ID);
	auto itype = type.ID;

	if (itype >= Type::Nothing && itype <= Type::String) return indent(alignment) + type_strings[ID] + "\n";

	if (itype < Type::Unknown) return PrintableTypeList(TypeList(itype)).print(alignment);
	
	return indent(alignment) + "UnknownType\n";

}

AST::Type::Type(TypeID ID) : ID(ID) { }

managed_map<AST::TypeList, AST::TypeID> _AST_Type_kv_map;
managed_map<AST::TypeID, AST::TypeList> _AST_Type_vk_map;
AST::TypeID _AST_Type_next_free_val = AST::Type::Unknown - 1;

AST::Type::Type(TypeList& type_list) : ID(getValueFromKey<AST::TypeList>(_AST_Type_kv_map, _AST_Type_vk_map, _AST_Type_next_free_val, type_list)) { }

const AST::TypeList& AST::Type::getTypeList() { return getKeyFromValue<AST::TypeList>(_AST_Type_vk_map, ID); }