#include "./SymbolTableLinker.hpp"
#include "./SymbolSearchTree.hpp"

AST::SymbolTableLinker::SymbolTableLinker() { }

AST::SymbolTableLinker::SymbolTableLinker(std::initializer_list<ptr_SymbolTable> init_list) { auto v = managed_vec<ptr_SymbolTable>(init_list); link(v); }

AST::SymbolTableLinker::SymbolTableLinker(managed_vec<ptr_SymbolTable>& v) { link(v); }

void AST::SymbolTableLinker::clear() { table = nullptr; next = nullptr; }

AST::SymbolTableLinker AST::SymbolTableLinker::deepCopy() {

	SymbolTableLinker linker;

	auto cur_linker = this;
	auto cur_copy = &linker;

	while (cur_linker) {

		cur_copy->table = cur_linker->table;

		if (cur_linker->next()) {

			cur_copy->next = new SymbolTableLinker();
			cur_copy = cur_copy->next();

		}

		cur_linker = cur_linker->next();

	}

	return linker;

}

AST::SymbolTableLinker AST::SymbolTableLinker::linkWith(ptr_SymbolTable table) {

	auto new_linker = deepCopy();

	linkWith_(new_linker, table);
	return new_linker;

}

AST::SymbolTableLinker AST::SymbolTableLinker::linkWith(SymbolTableLinker linker) { 

	auto new_linker = deepCopy();
	
	auto cur_linker = &linker;
	while (cur_linker) { linkWith_(new_linker, cur_linker->table); cur_linker = cur_linker->next(); }

	return new_linker;
	
}

AST::SymbolTableLinker AST::SymbolTableLinker::attachTree(ptr_SymbolSearchTreeBase search_tree) { 
	
	if (search_tree == nullptr) return *this;

	auto sym_search_tree = search_tree.cast<SymbolSearchTree>();

	if (sym_search_tree->bundle_symbols) pattern_match_search_tree = search_tree;
	else variable_search_tree = search_tree;
	
	return *this; 
	
}

AST::SymbolTableLinker::Iterator AST::SymbolTableLinker::iterate(SymbolID ID) {

	Iterator it;

	auto owner = this;
	while (owner) { if (owner->table != nullptr) if (owner->table->getSymbolList(ID).size() > 0) break; owner = owner->next(); }

	if (owner) { it.sym = owner->table->getSymbolList(ID)[0]; it.owner = owner; }

	return it;

}

string AST::SymbolTableLinker::toString(int alignment) {

	if (next == nullptr && table == nullptr) return "SymbolTableLinker (empty)\n";

	string s = "SymbolTableLinker:\n";
	
	auto cur = this;
	while (cur) {

		if (cur->table != nullptr) s += cur->table->print(alignment + 1, cur->next != nullptr);
		cur = cur->next();

	}

	return s;

}

AST::ptr_Symbol AST::SymbolTableLinker::Iterator::operator () () { return sym; }

AST::SymbolTableLinker::Iterator AST::SymbolTableLinker::Iterator::next() {

	Iterator it_next;

	if (idx == owner->table->getSymbolList(sym->ID).size() - 1) {

		auto next_owner = owner->next;

		while (next_owner != nullptr) { 
			
			if (next_owner->table != nullptr) if (next_owner->table->getSymbolList(sym->ID).size() > 0) break;
			next_owner = next_owner->next;
			
		}

		if (next_owner != nullptr) {

			it_next.sym = next_owner->table->getSymbolList(sym->ID)[0];
			it_next.owner = next_owner();

		}

	}
	else { 
		
		it_next = *this; 
		it_next.idx++; 
		it_next.sym = owner->table->getSymbolList(sym->ID)[it_next.idx];
	
	}

	return it_next;

}

bool AST::SymbolTableLinker::Iterator::finished() { return sym == nullptr; }

void AST::SymbolTableLinker::link(managed_vec<ptr_SymbolTable>& v) {
	
	if (v.size() == 0) return;
	
	managed_vec<ptr_SymbolTable> v_without_repeats;

	for (auto t1 : v) {

		bool is_repeat = false;
		for (auto t2 : v_without_repeats) if (t1 == t2) { is_repeat = true; break; }

		if (!is_repeat) v_without_repeats.push_back(t1);

		for (auto t2 : t1->includes) {

			is_repeat = false;
			for (auto t3 : v_without_repeats) if (t2 == t3) { is_repeat = true; break; }

			if (!is_repeat) v_without_repeats.push_back(t2);

		}

	}
	
	auto cur_linker = this;

	for (auto table : v_without_repeats) {

		cur_linker->table = table;
		cur_linker->next = new SymbolTableLinker();

		cur_linker = cur_linker->next();

	}

}

void AST::SymbolTableLinker::linkWith_(SymbolTableLinker& linker, ptr_SymbolTable table) {

	if (table == nullptr) return;
	
	auto cur_linker = &linker;
	auto last_linker = cur_linker;

	bool is_repeat = false;

	while (cur_linker) {
		
		if (cur_linker->table == table) { is_repeat = true; break; }

		last_linker = cur_linker;
		cur_linker = cur_linker->next();
		
	}

	if (!is_repeat) {

		last_linker->next = new SymbolTableLinker();
		last_linker->next->table = table;

	}

	for (auto include : table->includes) linkWith_(linker, include);

}