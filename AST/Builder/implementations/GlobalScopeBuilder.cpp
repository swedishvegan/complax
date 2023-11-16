#include "./GlobalScopeBuilder.hpp"
#include "./../../Builder/implementations/implement_builder.hpp"
#include "./../../Pattern/implementations/Header.hpp"
#include "./../../Pattern/implementations/Body.hpp"
#include "./../../Pattern/implementations/Declaration.hpp"
#include "./../../Pattern/implementations/Expression/Expression.hpp"
#include "./../../Symbol/SymbolSearchTree.hpp"
#include "./../../../Code/Bundle.hpp"
#include "./../../../Eval/NodeEvaluator.hpp"

#define AST_GSB_codebase (*((Code::Bundle*)codebase))

AST::GlobalScopeBuilder::GlobalScopeBuilder(void* codebase, Code::Loader& loader)
	: Builder(BuilderID::GlobalScopeBuilder, loader, loader.getNextScopeStart(0, 0), loader.getNextScopeStart(0, -1)), codebase(codebase) {
	
	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::Label);

	NodeCache cache(loader.size());

	auto cur_cache = Scanner_Expression::cache;
	Scanner_Expression::cache = &cache;
	
	build();

	Scanner_Expression::cleanup();
	Scanner_Expression::cache = cur_cache;

}

AST::Pattern* AST::GlobalScopeBuilder::main_function = nullptr;

AST_DECL_SUCCESSORS(GlobalScopeBuilder) {

	bool var_decl = (AST_SUCC_ISTYPE(Expression) && last_pattern_was_declaration);
	bool returns_exp = (AST_SUCC_ISTYPE(Expression) && allow_returns_exp);

	if (
		AST_SUCC_ISTYPE(None) ||
		AST_SUCC_ISTYPE(Literal) ||
		AST_SUCC_ISTYPE(Body_Function) ||
		AST_SUCC_ISTYPE(Body_Structure) ||
		AST_SUCC_ISTYPE(Body_Restrictions) ||
		AST_SUCC_ISTYPE(Label) ||
		AST_SUCC_ISTYPE(Body_Precedence) ||
		var_decl ||
		returns_exp
	) {

		if (allow_impinc) AST_SUCC_ALLOW(Keyword_import);
		if (allow_impinc) AST_SUCC_ALLOW(Keyword_include);

		AST_SUCC_ALLOW(Keyword_startprogram);

		AST_SUCC_ALLOW(Keyword_function);
		//AST_SUCC_ALLOW(Keyword_structure);

		AST_SUCC_ALLOW(Declaration);
		AST_SUCC_ALLOW(Expression);

		AST_SUCC_ALLOW(Done);

	}

	if (AST_SUCC_ISTYPE(Keyword_import) || AST_SUCC_ISTYPE(Keyword_include)) AST_SUCC_ALLOW(Literal);

	AST_SUCC_CASE(Keyword_startprogram) { AST_SUCC_ALLOW(Body_Function); allow_impinc = false; }

	AST_SUCC_CASE(Declaration) AST_SUCC_ALLOW(Expression);

	AST_SUCC_CASE(Keyword_function) { AST_SUCC_ALLOW(Header_Function); allow_impinc = false; }
	if (
		AST_SUCC_ISTYPE(Header_Function) ||
		AST_SUCC_ISTYPE(Body_Function) ||
		(AST_SUCC_ISTYPE(Expression) && !last_pattern_was_declaration && relevant_header_type == SymbolID::Function) ||
		(AST_SUCC_ISTYPE(Body_Restrictions) && relevant_header_type == SymbolID::Function) ||
		(AST_SUCC_ISTYPE(Label) && relevant_header_type == SymbolID::Function) ||
		(AST_SUCC_ISTYPE(Body_Precedence) && relevant_header_type == SymbolID::Function)
	) {

		if (allow_body) AST_SUCC_ALLOW(Body_Function);
		if (allow_restrictions) AST_SUCC_ALLOW(Keyword_wrest);
		if (allow_label) AST_SUCC_ALLOW(Keyword_wlabl);
		if (precedence_count == 0) AST_SUCC_ALLOW(Keyword_wprec);
		else if (precedence_count == 1) AST_SUCC_ALLOW(Keyword_precarrow);
		if (allow_returns_kw) AST_SUCC_ALLOW(Keyword_returns);

	}

	AST_SUCC_CASE(Keyword_structure) { AST_SUCC_ALLOW(Header_Structure); allow_impinc = false; }
	if (
		AST_SUCC_ISTYPE(Header_Structure) ||
		AST_SUCC_ISTYPE(Body_Structure) ||
		(AST_SUCC_ISTYPE(Expression) && !last_pattern_was_declaration && relevant_header_type == SymbolID::Structure) ||
		(AST_SUCC_ISTYPE(Body_Restrictions) && relevant_header_type == SymbolID::Structure) ||
		(AST_SUCC_ISTYPE(Label) && relevant_header_type == SymbolID::Structure) ||
		(AST_SUCC_ISTYPE(Body_Precedence) && relevant_header_type == SymbolID::Structure)
	) {

		if (allow_body) AST_SUCC_ALLOW(Body_Structure);
		if (allow_restrictions) AST_SUCC_ALLOW(Keyword_wrest);
		if (allow_label) AST_SUCC_ALLOW(Keyword_wlabl);
		if (precedence_count == 0) AST_SUCC_ALLOW(Keyword_wprec);
		else if (precedence_count == 1) AST_SUCC_ALLOW(Keyword_precarrow);

	}

	AST_SUCC_CASE(Keyword_wrest) AST_SUCC_ALLOW(Body_Restrictions);
	AST_SUCC_CASE(Keyword_wlabl) AST_SUCC_ALLOW(Label);
	if (AST_SUCC_ISTYPE(Keyword_wprec) || AST_SUCC_ISTYPE(Keyword_precarrow)) AST_SUCC_ALLOW(Body_Precedence);

	AST_SUCC_CASE(Keyword_returns) AST_SUCC_ALLOW(Expression);

	if (var_decl) last_pattern_was_declaration = false;
	
	AST_SUCC_CASE(Expression) {

		allow_impinc = false;
		allow_returns_exp = false;

	}

}

bool AST::GlobalScopeBuilder::processPattern(ptr_Pattern p) {
	
	if (p->ID == PatternID::Declaration) {

		auto sym = p.cast<Declaration>()->sym;

		auto symbols = SymbolTableLinker{ table };
		auto& name = sym->components[0].name;
		auto it = symbols.iterate(SymbolID::Variable);

		while (!it.finished()) {

			if (it()->components[0].name == name) {

				error.error = true;
				error.info = "Repeat declaration at " + getPositionString(p->start, p->end) + " of symbol:";
				error.sources.push_back(new PrintableString(((Pattern*)it()->declaration)->print(0)));

				finished = true;
				return false;

			}

			it = it.next();

		}

		allow_impinc = false;

		sym->declaration = p();
		sym->is_global = true;

		table->add(sym);

		if (variable_search_tree != nullptr) variable_search_tree->add(sym);
		else getSymbols();

		Scanner_Expression::banned_symbol = sym;
		Scanner_Expression::setRequirements(NodeID::None);

		relevant_header_type = SymbolID::None;
		last_pattern_was_declaration = true;

		return true;

	}

	Scanner_Expression::banned_symbol = nullptr;
	Scanner_Expression::setRequirements(NodeID::Label);

	if (p->ID == PatternID::Keyword_startprogram) { relevant_header = nullptr; return true; }

	if (p->ID == PatternID::Literal) {

		auto l = p.cast<Literal>();
		bool rest_valid = l->type == Type::String;

		if (!rest_valid) {

			error.error = true;
			error.info = "Expected string literal after statement:";
			error.sources.push_back(patterns[patterns.size() - 1].cast<Printable>());

			finished = true;
			return false;

		}

		auto filename = l->info;

		auto file_AST = AST_GSB_codebase[filename];
		
		if (file_AST.error.error) {

			error = file_AST.error;

			finished = true;
			return false;

		}

		if (file_AST.builder == nullptr) {

			error.error = true;
			error.info = "Attempt to import or include a file before it finished compiling at " + l->start.toString() + ". This most likely means the file references itself, either directly or indirectly. Problematic file:";
			error.sources.push_back(new PrintableString(filename.c_str()));

			finished = true;
			return false;

		}

		auto last_kw_type = patterns[patterns.size() - 1]->ID;

		if (last_kw_type == PatternID::Keyword_import) imports = imports.linkWith(file_AST.builder->table);
		else table->include(file_AST.builder->table);

		return true;

	}

	if (p->ID == PatternID::Expression) {

		if (last_pattern_was_declaration) {
			
			auto decl = patterns[patterns.size() - 1].cast<Declaration>();
			decl->sym->definition = p();

			auto exp = p.cast<Expression>();
			Eval::NodeEvaluator evaluator(exp(), true);

			if (evaluator.error.error) {

				error = evaluator.error;

				finished = true;
				return false;

			}

			if (!evaluator.is_constant) {

				error.error = true;
				error.info = "Non-constant variable declared in global scope:";
				error.sources.push_back(decl.cast<Printable>());

				finished = true;
				return false;

			}

			decl->sym->evaluator = evaluator;

		}

		else if (allow_returns_exp) relevant_header->sym->return_expression = p();

		else {

			auto header_sym = p.cast<Expression>()->node.cast<LabelNode>()->sym;

			relevant_header = (Builder_Header*)header_sym->header;
			relevant_header_type = header_sym->ID;

			allow_body = true;
			allow_restrictions = true;
			allow_label = true;
			if (relevant_header_type == SymbolID::Function) allow_returns_kw = true;
			precedence_count = 0;

		}

		return true;

	}

	bool is_function = p->ID == PatternID::Header_Function;
	bool is_structure = p->ID == PatternID::Header_Structure;

	if (is_function || is_structure) {

		auto builder = is_function ? p.cast<Header_Function>()->builder : p.cast<Header_Structure>()->builder;

		relevant_header = builder.cast<Builder_Header>()();
		relevant_header_type = is_function ? SymbolID::Function : SymbolID::Structure;

		auto sym = relevant_header->sym.cast<Symbol>();

		sym->header = builder();
		table->add(sym);

		if (pattern_match_search_tree != nullptr) pattern_match_search_tree->add(sym);
		else getSymbols();

		allow_body = true;
		allow_restrictions = true;
		allow_label = true;
		if (is_function) allow_returns_kw = true;
		precedence_count = 0;

		return true;

	}

	is_function = p->ID == PatternID::Body_Function;
	is_structure = p->ID == PatternID::Body_Structure;

	if (is_function || is_structure) {

		if (!relevant_header) {

			if (main_function) {

				error.error = true;
				error.info = "Program start redefinition:";

				p->single_line = true;

				error.sources.push_back(p.cast<Printable>());

				finished = true;
				return false;

			}

			main_function = p();

			return true;

		}

		if (relevant_header->sym->body) {

			error.error = true;
			error.info = "Symbol body redefinition:";

			p->single_line = true;

			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}

		relevant_header->sym->body = is_function ? p.cast<Body_Function>()->builder() : p.cast<Body_Structure>()->builder();

		allow_body = false;
		return true;

	}

	if (p->ID == PatternID::Body_Restrictions) {

		if (relevant_header->sym->restrictions) {

			error.error = true;
			error.info = "Symbol restrictions redefinition:";

			p->single_line = true;

			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}

		relevant_header->sym->restrictions_source = p.cast<Body_Restrictions>()->builder();

		allow_restrictions = false;
		return true;

	}

	if (p->ID == PatternID::Label) {

		if (relevant_header->sym->label.size() > 0) {

			error.error = true;
			error.info = "Symbol label redefinition:";
			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}

		relevant_header->sym->label = p.cast<Label>()->name;

		allow_label = false;
		return true;

	}

	if (p->ID == PatternID::Body_Precedence) {

		auto prec = p.cast<Body_Precedence>()->builder.cast<Builder_Body_Precedence>()->sym_precedence;
		auto sym = relevant_header->sym;
		
		if (sym->precedence_set_lh && precedence_count == 0) {

			error.error = true;
			error.info = "Symbol precedence redefinition:";

			p->single_line = true;

			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}

		if (sym->num_references > 0) {

			error.error = true;
			error.info = "Symbol precedence cannot be changed after the symbol has been used in expression parsing:";

			p->single_line = true;

			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}

		if (precedence_count == 0) {

			sym->precedence_set_lh = true;
			sym->precedence_lh = prec;
			sym->precedence_rh = prec;

		}
		else {

			sym->precedence_set_rh = true;
			sym->precedence_rh = prec;

		}

		precedence_count++;

		return true;

	}

	if (p->ID == PatternID::Keyword_returns) { 

		auto sym = relevant_header->sym;

		if (sym->return_expression) {

			error.error = true;
			error.info = "Symbol return type redefinition:";

			p->single_line = true;

			error.sources.push_back(p.cast<Printable>());

			finished = true;
			return false;

		}
		
		allow_returns_kw = false; 
		allow_returns_exp = true;

		Scanner_Expression::banned_symbol = nullptr;
		Scanner_Expression::setRequirements(NodeID::None);
		
		return true;
		
	}

	return true;

}

AST::SymbolTableLinker AST::GlobalScopeBuilder::getSymbols() {

	auto syms = SymbolTableLinker{ table, AST_GSB_codebase.default_symbols }.linkWith(imports);

	if (pattern_match_search_tree == nullptr && !allow_impinc) pattern_match_search_tree = new PatternMatchSearchTree(syms, SEARCH_TREE_DEPTH);

	if (variable_search_tree == nullptr && !allow_impinc) variable_search_tree = new VariableSearchTree(syms, SEARCH_TREE_DEPTH);
	if (variable_search_tree != nullptr) variable_search_tree->local_symbols = relevant_header ? SymbolTableLinker{ relevant_header->table } : SymbolTableLinker{ };

	if (relevant_header) syms = syms.linkWith(relevant_header->table);

	return syms.attachTree(pattern_match_search_tree.cast<SymbolSearchTreeBase>()).attachTree(variable_search_tree.cast<SymbolSearchTreeBase>());

}