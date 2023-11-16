#include "./Bundle.hpp"
#include "./../AST/Symbol/BuiltInSymbols.hpp"

Code::Bundle::Bundle() { default_symbols = AST::genDefaultSymbolTable(); }

Code::File Code::Bundle::operator [] (managed_string filename) {

	auto find_file = file_map.find(filename);
	if (find_file != file_map.end()) return find_file->second;

	auto& new_file = file_map[filename];

	ptr_Loader loader = new Loader(filename);

	auto error_message = loader->getErrorMessage();

	if (error_message.size() > 0) {

		new_file.error.error = true;
		new_file.error.info = error_message;

		return new_file;

	}

	AST::ptr_Builder builder = new AST::GlobalScopeBuilder(this, *loader);

	if (builder->error.error) { new_file.error = builder->error; return new_file; }

	new_file.loader = loader;
	new_file.builder = builder;

	return new_file;

}

managed_map<managed_string, Code::File>& Code::Bundle::operator () () { return file_map; }