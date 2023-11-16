#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include "./../../util/string.hpp"
#include "./../../util/ptr.hpp"
#include "./../../util/vec.hpp"
#include "./../../util/Printable.hpp"
#include "./../Type/Restrictions.hpp"
#include "./../Pattern/Pattern.hpp"
#include "./../../Eval/NodeEvaluator.hpp"

// Base class for Symbols (variables, functions, structures, etc.)

namespace AST {

	enum class SymbolID {

		None,

		Function,
		Structure,
		Variable,

		_size

	};

	struct SymbolComponent : public Printable {

		managed_string name;
		bool is_argument;

		SymbolComponent();
		SymbolComponent(managed_string name, bool is_argument);

		bool operator == (SymbolComponent&);
		bool operator != (SymbolComponent&);

		string toString(int);

	};

	struct Symbol : public Printable {

		SymbolID ID;
		managed_vec<SymbolComponent> components;

		int index = -1; // Index of Symbol within owner table

		union { void* declaration = nullptr; void* header; }; // Declarations/ headers are set inside of Builder implementations
		union { void* definition = nullptr; void* body; };    // Definitions/ bodies are set inside of Builder implementations

		int num_arguments = 0;
		int num_fillers = 0;
		
		Eval::NodeEvaluator evaluator; // Used during code evaluation to store type evaluation results

		int num_references = 0;   // How many times the symbol is referened in the code; used for error checking
		
		int scope = 0;            // Scope that the Symbol was defined in

		bool is_global = false;   // Whether or not the Symbol is declared in global scope (only relevant for variable symbols)
		bool is_argument = false; // Whether or not the Symbol is an argument for a PatternMatch

		Symbol(SymbolID);

		int size();

		SymbolComponent& operator [] (int);

		virtual void add(SymbolComponent);

		bool operator == (Symbol&); // Checks whether two symbol definitions conflict with each other
		bool operator != (Symbol&);

		managed_string& getSignature(); // Unique string description of Symbol's pattern (i.e. " stuff with {} and {} ")

		string toString(int);

		managed_string oneLineDescription(bool include_arg_names = true);
		
	protected:

		managed_string signature;

	};

	using ptr_Symbol = ptr<Symbol>;
	using SymbolList = managed_vec<ptr_Symbol>;

	struct Precedence : public Printable {

		double p0 = 0.0;
		double p1 = 0.0;
		double p2 = 0.0;

		Precedence(double = 0.0, double = 0.0, double = 0.0);

		bool operator >= (Precedence);
		bool operator > (Precedence);
		bool operator <= (Precedence);
		bool operator < (Precedence);
		bool operator == (Precedence);
		bool operator != (Precedence);

		string toString(int);

	};

	struct HeaderSymbol : public Symbol {

		managed_vec<int> filler_indices;
		managed_vec<int> argument_indices;

		managed_string description; // This is also used in the codebase as an indicator of whether a function is built-in (because only built-in functions have custom descriptions)

		managed_string label; // Set by user with the wlabl keyword

		Restrictions* restrictions = nullptr;
		void* restrictions_source = nullptr;   // Builder_Body_Restrictions object

		enum class RestrictionsEvaluationProgress { None, InProgress, Complete }
		restrictions_eval_progress = RestrictionsEvaluationProgress::None; // Progress of the Restrictions evaluation -- used to detect infinite recursion caused by self-referential type restrictions

		Precedence precedence_lh; // Set by user with the wprec keyword
		bool precedence_set_lh = false;

		Precedence precedence_rh; // Set by user with the <-> keyword after LH precedence
		bool precedence_set_rh = false;

		void* return_expression = nullptr; // Expression object

		struct InstantiationInfo { 
			
			TypeID return_type = Type::Anything;
			bool evaluation_complete = false;

			void* bytecode = nullptr; // Eval::BytecodeBlock* object
			
		};

		managed_map<TypeID, InstantiationInfo> instantiations; // Maps argument TypeList (hashed as a TypeID) to a return TypeID

		HeaderSymbol(SymbolID);

		void add(SymbolComponent);

		int distanceToNextFiller(int filler_index); // Distance from filler_indices[filler_index] to filler_indices[filler_index + 1], or to the last component in the Symbol if there is no next filler
		
	};

	using ptr_HeaderSymbol = ptr<HeaderSymbol>;

}

#endif