#include "./Type.hpp"
#include "./../Symbol/Symbol.hpp"

managed_map<AST::TypeList, AST::TypeID> _AST_TypeList_kv_map;
managed_map<AST::TypeID, AST::TypeList> _AST_TypeList_vk_map;
AST::TypeID _AST_TypeList_next_free_val = AST::Type::Unknown - 1;

managed_map<AST::TypeID, AST::TypeID> _AST_Array_kv_map;
managed_map<AST::TypeID, AST::TypeID> _AST_Array_vk_map;
AST::TypeID _AST_Array_next_free_val = AST::Type::Array + 3;

managed_map<AST::StructureInstantiation, AST::TypeID> _AST_Structure_kv_map;
managed_map<AST::TypeID, AST::StructureInstantiation> _AST_Structure_vk_map;
AST::TypeID _AST_Structure_next_free_val = AST::Type::Structure + 3;

AST::Type::Type(TypeID ID) : ID(ID) { determineClass(); }

AST::Type AST::Type::fromTypeList(const TypeList& type_list) { return Type(type_list, Class::TypeList); }

AST::Type AST::Type::fromArray(TypeID contained_type) { return Type(getValueFromKey<AST::TypeID>(_AST_Array_kv_map, _AST_Array_vk_map, _AST_Array_next_free_val, contained_type, 3)); }

AST::Type AST::Type::fromStructure(const StructureInstantiation& structure) { return Type(getValueFromKey<AST::StructureInstantiation>(_AST_Structure_kv_map, _AST_Structure_vk_map, _AST_Structure_next_free_val, structure, 3)); }

const AST::TypeList& AST::Type::getTypeList() { return getKeyFromValue<AST::TypeList>(_AST_TypeList_vk_map, ID); }

AST::TypeID AST::Type::getArrayContainedType() { return getKeyFromValue<AST::TypeID>(_AST_Array_vk_map, ID); }

const AST::StructureInstantiation& AST::Type::getStructureInstantiation() { return getKeyFromValue<AST::StructureInstantiation>(_AST_Structure_vk_map, ID); }

bool AST::Type::is(Type type) { 

	if (type.ID == Anything) return true;

	if (type_class != type.type_class) return false;

	if (!isConcrete()) return false;
	
	if (type_class == Class::Array) {

		if (type.ID == Array) return true;
		return Type(getArrayContainedType()).is(Type(type.getArrayContainedType()));

	}

	if (type_class == Class::Structure) if (type.ID == Structure) return true;
	
	return ID == type.ID;

}

bool AST::Type::isConcrete() {

	if (ID == Anything || ID == Unknown) return false;

	if (type_class == Class::Primitive) return true;

	if (type_class == Class::Array) {

		if (ID == Array) return false;
		return Type(getArrayContainedType()).isConcrete();

	}

	if (type_class == Class::Structure) return (ID != Structure);

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

	if (type_class == Class::Structure) {

		auto& sinst = getStructureInstantiation();

		string s = "Structure:\n";
		s += indent(alignment + 1) + "Signature:\n";
		s += indent(alignment + 2) + ((AST::HeaderSymbol*)sinst.sym)->getSignature() + "\n";
		s += indent(alignment + 1) + "Elements:\n";
		s += Type::fromTypeList(sinst.element_types).print(alignment + 2, false);

		return s;

	}

	return "UnknownType";

}

AST::Type::Type(const TypeList& type_list, Class type_class) : type_class(type_class) {

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