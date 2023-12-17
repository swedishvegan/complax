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

	struct Type : public Printable  { // Wrapper around TypeID with some associated helper functions

		enum {

			Unknown = -1,
			Nothing,
			Anything,
			
			Integer, Decimal, Bool, Ascii, String,

			Array,     // All array types are assigned an ID of Array + 3N
			Structure, // All structure types are assigned an ID of Structure + 3N
			Function,  // All function types are assigned an ID of Function + 3N

			ArgumentList = -2 // All TypeLists are assigned an ID of -2 or less

		};

		enum class Class { Primitive, TypeList, Array, Structure, Function, None } type_class;

		TypeID ID;

		Type(TypeID);

		static Type fromTypeList(TypeList& type_list);
		static Type fromArray(TypeID contined_type);

		const TypeList& getTypeList();   // If type_class is TypeList, returns the TypeList corresponding to ID; otherwise, returns empty array
		TypeID getArrayContainedType(); // If type_class is Array, returns the contained type of the array type corresponding to ID; otherwise, returns Nothing

		bool is(Type); // More sophisticated than simple equality; not symmetric
		bool isConcrete();

		string toString(int);

	protected:

		Type(TypeList& type_list, Class type_class);

		void determineClass();

		template <typename key_type>
		static TypeID getValueFromKey(managed_map<key_type, TypeID>& kv_map, managed_map<TypeID, key_type>& vk_map, TypeID& next_free_val, key_type& key, int increment) {

			auto find_match = kv_map.find(key);

			if (find_match == kv_map.end()) {

				kv_map[key] = next_free_val;
				vk_map[next_free_val] = key;

				next_free_val += increment;
				return next_free_val - increment;

			}

			return find_match->second;

		}

		template <typename key_type>
		static const key_type& getKeyFromValue(managed_map<TypeID, key_type>& vk_map, TypeID ID) {

			static key_type dummy{ };

			auto find_match = vk_map.find(ID);

			if (find_match == vk_map.end()) return dummy;
			return find_match->second;

		}

	};

	using ptr_Type = ptr<Type>;

}

#endif