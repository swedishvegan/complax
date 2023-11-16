#include "./Assignment.hpp"

AST::Assignment::Assignment() : Pattern(PatternID::Assignment) { }

string AST::Assignment::getInfo(int alignment) { if (node == nullptr) return ""; return "\n" + node->print(alignment + 1, false); }

AST::Scanner_Assignment::Scanner_Assignment(Code::Loader& loader, int start, int end, int cur_best_start) : Scanner(loader, start, end, cur_best_start) {

	if (expression == nullptr) return;

	auto node = expression->node;
	
	end = node->end;
	while (loader(end).is_whitespace) end++;
	
	if (loader[end] == '=') {
		
		end++;

		ptr_Assignment ass = new Assignment();

		ass->start = loader(start);
		ass->end = loader(end);
		ass->node = node;

		result = ass.cast<Pattern>();

	}

	expression = nullptr;

}

AST::ptr_Expression AST::Scanner_Assignment::expression;