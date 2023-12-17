#include "./Type.hpp"
#include "./../Symbol/Symbol.hpp"

managed_map<AST::TypeList, AST::TypeID> _AST_TypeList_kv_map;
managed_map<AST::TypeID, AST::TypeList> _AST_TypeList_vk_map;
AST::TypeID _AST_TypeList_next_free_val = AST::Type::Unknown - 1;

#define _define_Type_Maps(TheType) \
managed_map<AST::TypeID, AST::TypeID> _AST_##TheType##_kv_map; \
managed_map<AST::TypeID, AST::TypeID> _AST_##TheType##_vk_map; \
AST::TypeID _AST_##TheType##_next_free_val = AST::Type::TheType + 3

_define_Type_Maps(Array);

AST::Type::Type(TypeID ID) : ID(ID) { determineClass(); }

AST::Type AST::Type::fromTypeList(TypeList& type_list) { return Type(type_list, Class::TypeList); }

AST::Type AST::Type::fromArray(TypeID contained_type) { return Type(getValueFromKey<AST::TypeID>(_AST_Array_kv_map, _AST_Array_vk_map, _AST_Array_next_free_val, contained_type, 3)); }

const AST::TypeList& AST::Type::getTypeList() { return getKeyFromValue<AST::TypeList>(_AST_TypeList_vk_map, ID); }

AST::TypeID AST::Type::getArrayContainedType() { return getKeyFromValue<AST::TypeID>(_AST_Array_vk_map, ID); }

bool AST::Type::is(Type type) { 

	if (type.ID == Anything) return true;

	if (type_class != type.type_class) return false;

	if (!isConcrete()) return false;
	
	if (type_class == Class::Array) {

		if (type.ID == Array) return true;
		return Type(getArrayContainedType()).is(Type(type.getArrayContainedType()));

	}

	return ID == type.ID;

}

bool AST::Type::isConcrete() {

	if (ID == Anything || ID == Unknown) return false;

	if (type_class == Class::Primitive) return true;

	if (type_class == Class::Array) {

		if (ID == Array) return false;
		return Type(getArrayContainedType()).isConcrete();

	}

	return true;

}

string AST::Type::toString(int alignment) {

	static const char* type_strings[] = {

		"Nothing",
		"Anything",

		"Integer", "Decimal",
		"Boolean",
		"Ascii", "String",

	};

	if (ID >= Nothing && ID <= String) return type_strings[ID];
	
	if (type_class == Class::TypeList) {

		string s = "TypeList:\n";
		auto& type_list = getTypeList();

		for (int i = 0; i < type_list.size(); i++) s += Type(type_list[i]).print(alignment + 1, i != type_list.size() - 1);

		return s;

	}

	if (type_class == Class::Array) {

		string s = "ArrayOf:\n";
		s += Type(getArrayContainedType()).print(alignment + 1, false);

		return s;

	}

	return "UnknownType";

}

AST::Type::Type(TypeList& type_list, Class type_class) : type_class(type_class) {

	if (type_class == Class::TypeList) ID = getValueFromKey<AST::TypeList>(_AST_TypeList_kv_map, _AST_TypeList_vk_map, _AST_TypeList_next_free_val, type_list, -1);
	// else if (...)

}

void AST::Type::determineClass() {

	if ((ID > Anything && ID < Array) || ID == Nothing) type_class = Class::Primitive;
	else if (ID == Anything || ID == Unknown) type_class = Class::None;
	else if (ID <= ArgumentList) type_class = Class::TypeList;
	else if ((ID - Array) % 3 == 0) type_class = Class::Array;
	else if ((ID - Structure) % 3 == 0) type_class = Class::Structure;
	else type_class = Class::Function;

}