#ifndef BUNDLE_HPP
#define BUNDLE_HPP

#include "./Loader.hpp"
#include "./../AST/Builder/implementations/GlobalScopeBuilder.hpp"
#include "./../util/CompileError.hpp"
#include "./../util/map.hpp"
#include "./../util/ptr.hpp"

// Keeps track of the entire codebase, loading new files when necessary

namespace Code {

	struct File  {
		
		CompileError error;

		ptr_Loader loader;
		AST::ptr_Builder builder;

	};

	struct Bundle  {
	public:

		Bundle();

		File operator [] (managed_string);

		managed_map<managed_string, File>& operator () ();

		AST::ptr_SymbolTable default_symbols;

		managed_map<managed_string, File> file_map;
		
	};

	using ptr_Bundle = ptr<Bundle>;

}

#endif