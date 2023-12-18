#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "./Node.hpp"
#include "./../../Pattern.hpp"
#include "./../../../Symbol/SymbolTableLinker.hpp"

/*

This file and the corresponding cpp file are arguably the most important files in the entire codebase,
as the way expression parsing works in this language is fundamentally different from other languages.

The loose syntax in this language allows for ambiguous expressions that could have several different 
interpretations, so it is therefore the compiler's job to find ALL possible interpretations of an 
expression, and decide which one is valid given the context.

Expressions are recognized using a tree-like search over the entire space of possible valid expressions.
This is accomplished mainly using two functions:

    generateSuccessors() looks at the current stack of nodes and checks for the existence of
    all possible nodes that could follow, given the language's syntax and the context of what's already
    on the stack. Then, for each possible successor node, this function calls the next function:

    validateExpression() looks at the current expression stack and validator stack and checks to see if 
    the pending pattern matches on the validator stack are completely filled out, and if they are, it 
    collapses them into singular nodes.

    Once an expression is validated, generateSuccessors() is called recursively on every successor node
    that the current generateSuccessors() call found.

Once every possible expression interpretation is found, they are compared inside the Scanner_Expression
and the final expression is chosen using the following heuristics:

 >> If two expressions have the same starting point, the longer one is chosen and the shorter one is
 abandoned.

 >> If two valid expressions have the same starting and ending point, a compiler error is generated.

 >> If no valid expressions are found, the Scanner_Expression simply returns failure.

 Note also that since the nature of expression parsing requires the scanner to scan ahead as far as it possibly
 can, this allows for expression caches to be implemented as an optimization, so that the same
 expression need not be parsed twice.

*/

namespace AST {

    struct Expression : public Pattern {

        ptr_Node node;

        Expression();

        string getInfo(int);

        Printable* singleLine();
        
        static unsigned long long total_successors_generated;
        
    };

    using ptr_Expression = ptr<Expression>;

    struct Scanner_Expression : public Scanner {

        Scanner_Expression(Code::Loader& loader, int start, int end, int cur_best_start, SymbolTableLinker symbols, bool is_top_level = true);
        
        static ptr_Symbol banned_symbol; // Symbol that is not allowed to appear in the winner expression

        template <typename... NodeID_args> // Determines the types of Nodes that will be accepted as a winner
        static void setRequirements(NodeID ID, NodeID_args... args) { _num_reqs = 0; for (int i = 0; i < (int)AST::NodeID::_size; i++) requirements[i] = false; _setRequirements(ID, args...); }

        static void cleanup(); // Cleans up the static variables (ensures reference count is zero at the end of compilation)

        static NodeCache* cache; // Optimization to avoid parsing the same Expression twice unnecessarily

        static void clearCache();

    protected:

        Code::Loader& loader;
        int start, end;
        NodeList candidates;       // List of all possible valid expressions; chooses the longest one out of the list
        SymbolTableLinker symbols; // Symbols that the Expression looks at when looking for symbol matches
        bool is_top_level;         // Indicates whether the Scanner was created by another Scanner

        static bool requirements[];       // Types of Nodes that will be accepted as a winner

        static NodeID _singular_requirement;
        static int _num_reqs;

        void generateSuccessors(ExpressionStack&, ValidatorStack&, int start); // See above explanation

        // Everything from here to validateExpression() is a helper function for generateSuccessors()

        void generateExpressionSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        void generateArrayInitializerSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        void generateLiteralSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        void generateVariableSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        void generateFillerSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors); // Fillers that indicate a new pattern match is commencing

        void generateIntermediateFillerSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors); // Fillers that follow up on patterns already being observed by the validator stack

        void generateStructureMemberSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        void generateLabelSuccessors(ExpressionStack&, ValidatorStack&, int start, NodeList& successors);

        bool validateExpression(ExpressionStack&, ValidatorStack&); // See above explanation (Note: This function returns false if there is a syntax error found during validation)

        // Everything from here to terminateBranch() is a helper function for validateExpression() (but some of these functions are still used elsewhere too)

        void handleStructureMembers(ExpressionStack&); // Merges any pending StructureMemberKWNodes on top of the stack with an existing StructureMemberNode if there is one, and creates one otherwise

        bool handleFlushes(ExpressionStack&, ValidatorStack&); // Flushes a pending pattern match, returns false if failure occurs

        void updateValidators(ExpressionStack&, ValidatorStack&);

        bool patternMatchExists(ExpressionStack&, ValidatorStack&); // Checks whether there is a pattern match currently on the stack that needs collapsing

        bool collapsePatternMatch(ExpressionStack&, ValidatorStack&, Validator* val = nullptr); // Collapses current pattern match into singular node, returns false if failure occurs

        bool createValidator(ExpressionStack&, ValidatorStack&); // Checks if a new validator is needed, and adds one to the stack if so, provided that there are enough arguments preceding the current filler

        void terminateBranch(ExpressionStack&, ValidatorStack&); // Finishes up the expression parsing and does some maintenance

        void getWinner(NodeList& candidates); // Picks the most valid expression out of all terminal branches, if there is any such expression

        int getArgumentStreakLength(ExpressionStack&, int offset); // Returns the number of consecutive arguments on top of the expression stack

        static bool isPossibleSuccessor(NodeID); // Determines whether the NodeID signifies a possible successor type given the context of what is currently in the requirements array

        template <typename... NodeID_args>
        static void _setRequirements(NodeID ID, NodeID_args... args) { requirements[(int)ID] = true; _singular_requirement = _num_reqs == 0 ? ID : NodeID::None; _num_reqs++; _setRequirements(args...); }

        static void _setRequirements(NodeID);

        static void addToCache(ptr_Node); // Adds node to cache, accounting for collisions

        static void addToCache(ptr_Node, NodeList&);

    };

}

#endif