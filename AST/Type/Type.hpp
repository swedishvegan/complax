#ifndef TYPE_HPP
#define TYPE_HPP

#include "./../../util/ptr.hpp"
#include "./../../util/vec.hpp"
#include "./../../util/map.hpp"
#include "./../../util/Printable.hpp"

// Some helper functions for handling type logic in the AST codebase

namespace AST {

	struct Symbol;
	using ptr_Symbol = ptr<Symbol>;

	using TypeID = int;
	using TypeList = managed_vec<TypeID>; // TypeLists are convenient ways of bundling together a list of TypeIDs into one single TypeID

	struct PrintableTypeList : public Printable { // Helper object for debugging and generating error messages

		TypeList type_list;

		PrintableTypeList(TypeList);

		string toString(int);

		static string TypeID_toString(TypeID, int);

	};

	using ptr_PrintableTypeList = ptr<PrintableTypeList>;

	struct Type  { // Wrapper around TypeID with some associated helper functions

		enum {

			Unknown = -1,
			Nothing,
			Anything,
			
			Integer, Decimal, Bool, String,

			ArgumentList = -2 // All TypeLists are assigned an ID of -2 or less

		};

		TypeID ID;

		Type(TypeID);

		Type(TypeList& type_list);

		const TypeList& getTypeList();

	protected:

		template <typename key_type>
		static TypeID getValueFromKey(managed_map<key_type, TypeID>& kv_map, managed_map<TypeID, key_type>& vk_map, TypeID& next_free_val, key_type& key) {

			auto find_match = kv_map.find(key);

			if (find_match == kv_map.end()) {

				kv_map[key] = next_free_val;
				vk_map[next_free_val] = key;

				next_free_val--;
				return next_free_val + 1;

			}

			return find_match->second;

		}

		template <typename key_type>
		static const key_type& getKeyFromValue(managed_map<TypeID, key_type>& vk_map, TypeID ID) {

			static key_type dummy;

			auto find_match = vk_map.find(ID);

			if (find_match == vk_map.end()) return dummy;
			return find_match->second;

		}

	};

	using ptr_Type = ptr<Type>;

}

#endif