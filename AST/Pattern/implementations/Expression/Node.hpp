#ifndef NODE_HPP
#define NODE_HPP

#include "./../../../../util/vec.hpp"
#include "./../../../../util/vstack.hpp"
#include "./../../../../util/Printable.hpp"
#include "./../Literal.hpp"
#include "./../../../Symbol/Symbol.hpp"
#include "./../../../Symbol/SymbolSearchTree.hpp"

// defines Nodes that makes up an Expression, also defines ExpressionStacks and ValidatorStacks which are used in Expression parsing

namespace AST {

	struct Node;
	struct Expression;

	using ptr_Node = ptr<Node>;
	using ptr_Expression = ptr<Expression>;

	using NodeList = managed_vec<ptr_Node>;
	using NodeCache = managed_vec<NodeList>;
	
	using ExpressionStack = vstack<ptr_Node>;

	struct Validator { // Used to check that PatternMatches follow proper syntax
		
		ptr_SymbolBundle bundle; // Pattern being matched (either a function call or structure instantiation)
		int index = 0;           // Index on the stack where the PatternMatch begins
		int distance = 0;        // Number of objects that must be on the stack before the next filler is expected
		int next_filler = 0;     // Index of next expected filler within sym
	
	};
	using ValidatorStack = vstack<Validator>;

	enum class NodeID { 

		None,
		
		Expression, // Parenthetical expression
		Literal, 
		Variable, 
		Filler, 
		PatternMatch, // A PatternMatch is either a function call or a structure instantiation
		StructureMember,
		StructureMemberKW, // Temporary node signifying that a StructureMemberNode should either be created or appended with the new KW
		Label,

		_size,
		
	};

	struct Node : public Printable {

		int start = -1, end = -1;
		NodeID ID;

		Node(NodeID);

		bool isArgument(); // True unless node is type None or Filler

		bool contains(ptr_Symbol);

		void updateReferenceCounts(); // Updates reference count for every variable referenced within this Node

		SymbolBundle* findPrecedenceConflicts();

		CompileError* getContainedError(); // Checks for errors within any contained ExpressionNodes

		static bool equals(ptr_Node, ptr_Node);

		static bool necessitates(NodeID, NodeID); // Indicates whether an expression with the LH node type necessitates checking for expressions with the RH node type

	};

	struct EmptyNode : public Node { EmptyNode(int start); string toString(int); }; // Used to flush a pending pattern match

	struct ExpressionNode : public Node {

		ptr_Expression expression;

		ExpressionNode(ptr_Expression, int start, int end);

		string toString(int);

	};

	struct LiteralNode : public Node {

		ptr_Literal literal;

		LiteralNode(ptr_Literal);

		string toString(int);

	};

	using ptr_LiteralNode = ptr<LiteralNode>;

	struct KeywordNode : public Node { // Base class for VariableNode and FillerNode

		ptr_Symbol sym;
		int sym_idx;

		KeywordNode(NodeID, ptr_Symbol, int sym_idx, int start, int end);

		string toString(int);

	};

	using ptr_KeywordNode = ptr<KeywordNode>;

	struct VariableNode : public KeywordNode { VariableNode(ptr_Symbol, int start, int end); };

	using ptr_VariableNode = ptr<VariableNode>;

	struct FillerNode : public KeywordNode {
		
		ptr_SymbolBundle bundle;

		FillerNode(ptr_SymbolBundle, int sym_idx, int start, int end); 
		
	};

	using ptr_FillerNode = ptr<FillerNode>;

	struct PatternMatchNode : public Node { // Base class for FunctionCallNode and StructureInstantiationNode

		ptr_SymbolBundle bundle;
		NodeList components;

		PatternMatchNode(ptr_SymbolBundle);

		void addComponent(ptr_Node);

		string toString(int alignment);

	};

	using ptr_PatternMatchNode = ptr<PatternMatchNode>;

	struct StructureMemberNode : public Node {

		ptr_Node base;
		managed_vec<managed_string> members;

		StructureMemberNode(ptr_Node base);

		void addMember(managed_string, int new_end);

		string toString(int alignment);

	};

	using ptr_StructureMemberNode = ptr<StructureMemberNode>;

	struct StructureMemberKWNode : public Node {

		managed_string kw;

		StructureMemberKWNode(managed_string kw, int start, int end);

		string toString(int);

	};

	using ptr_StructureMemberKWNode = ptr<StructureMemberKWNode>;

	struct LabelNode : public Node {

		ptr_HeaderSymbol sym;

		LabelNode(ptr_HeaderSymbol, int start, int end);

		string toString(int);

	};

}

#endif