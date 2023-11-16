#include "./Pattern.hpp"

AST::Pattern::Pattern(PatternID ID) : ID(ID) { }

AST::Pattern::~Pattern() { }

string AST::Pattern::toString(int alignment) { 

	auto po_str = getPositionString(start, end, false);
	auto info = single_line ? "" : this->getInfo(alignment);

	if (info.size() > 0) {

		po_str = po_str.size() >= AST_POSITION_PAD_LENGTH - 1 ? (" (" + pad(po_str, AST_POSITION_PAD_LENGTH - 2) + "):") : (" (" + pad(po_str + "):", AST_POSITION_PAD_LENGTH));
		po_str += " ";

	}
	else po_str = " (" + po_str + ")";
	
	return pad(getPatternIDString(ID)) + po_str + info; 

}

string AST::Pattern::getInfo(int) { return ""; }

AST::Scanner::Scanner(Code::Loader& loader, int start, int end, int cur_best_start) { }

AST::Scanner::~Scanner() { }