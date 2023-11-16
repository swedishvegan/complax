#ifndef RESTRICTIONS_HPP
#define RESTRICTIONS_HPP

#include "./Type.hpp"

// Maps argument indices to a TypeList denoting all the types that argument is allowed to take on

namespace AST {

	struct Symbol;

	struct Restrictions : public Printable { // Note: An entry having a zero-length managed_vector is interpreted as no type restrictions

		Restrictions();

		TypeList& operator [] (int arg_index);

		string toString(int);

		bool isEmpty();

	protected:

		struct TypeListWrapper  { TypeList tl; }; // Allows for TypeList (an std::vector) to live in the memory pool

		managed_map<int, TypeListWrapper*> rest_map;

	};

	using ptr_Restrictions = ptr<Restrictions>;

}

#endif