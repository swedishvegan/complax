#include "./CodePieceBuilder.hpp"
#include "./CodePiece_implementations.hpp"
#include "./../Pattern/implementations/Body.hpp"
#include "./../Pattern/implementations/Declaration.hpp"
#include "./../Pattern/implementations/Assignment.hpp"
#include "./..//Builder/implementations/Builder_Body_Code.hpp"

AST::CodePiece::CodePiece(CodePieceID ID) : ID(ID) { }

void AST::CodePiece::removeCircularDependencies() {

	this->owner = nullptr;

	if (ID == CodePieceID::Declaration) {

		auto cast = ((CodePiece_Declaration*)this);
		cast->assignment->removeCircularDependencies();

	}

	else if (ID == CodePieceID::NestedCode) {

		auto cast = ((CodePiece_NestedCode*)this);

		cast->header->removeCircularDependencies();
		if (cast->body_pt1 != nullptr) cast->body_pt1->removeCircularDependencies();
		if (cast->body_pt2 != nullptr) cast->body_pt2->removeCircularDependencies();

	}

}

string AST::CodePiece::toString(int alignment) {

	if (ID == CodePieceID::PatternMatch) return ((CodePiece_PatternMatch*)this)->pattern_match->node->toString(alignment);

	if (ID == CodePieceID::Declaration) {

		auto cast = ((CodePiece_Declaration*)this);

		string s = "Declaration:\n";
		s += indent(alignment + 1) + "Variable:\n";
		s += indent(alignment + 2) + cast->new_var->oneLineDescription().c_str() + "\n";
		s += cast->assignment->print(alignment, false);

		return s;

	}

	if (ID == CodePieceID::Assignment) {

		auto cast = ((CodePiece_Assignment*)this);

		string s = "Assignment:\n";
		s += indent(alignment + 1) + "LH:\n";
		s += (cast->LH_assign == nullptr) ? (indent(alignment + 2) + cast->LH_declare->oneLineDescription().c_str() + "\n") : cast->LH_assign->node->print(alignment + 2);
		s += indent(alignment + 1) + "RH:\n";
		s += cast->RH->node->print(alignment + 2, false);

		return s;

	}

	if (ID == CodePieceID::Body) {

		auto cast = ((CodePiece_Body*)this);
		auto& pieces = cast->pieces();

		if (pieces.size() == 0) return "CodeBody (empty)";

		auto last_piece = pieces[pieces.size() - 1];

		string s = pieces[0]->toString(alignment) + (pieces[0] == last_piece ? "" : "\n");
		for (int i = 1; i < pieces.size(); i++) s += pieces[i]->print(alignment, pieces[i] != last_piece);

		return s;

	}

	if (ID == CodePieceID::NestedCode) {

		auto cast = ((CodePiece_NestedCode*)this);

		string s = cast->header->toString(alignment) + "\n";

		if (cast->body_pt1 == nullptr) s += indent(alignment + 1) + "(empty)";

		else if (cast->body_pt2 == nullptr) s += cast->body_pt1->print(alignment + 1, false);

		else {

			s += cast->body_pt1->print(alignment + 1);
			s += cast->body_pt2->print(alignment, false);

		}

		return s;

	}

	if (ID == CodePieceID::For) {
		
		auto cast = (CodePiece_For*)this;

		string s = "For:\n";
		s += indent(alignment + 1) + "Variable:\n";
		s += indent(alignment + 2) + cast->header->table->variables[0]->oneLineDescription().c_str() + "\n";
		s += indent(alignment + 1) + "In:\n";
		s += cast->header->iterator.cast<Expression>()->node->print(alignment + 2);
		s += indent(alignment) + "Do:";

		return s;

	}

	if (ID == CodePieceID::While) {

		auto cast = ((CodePiece_While*)this);

		string s = "While:\n";
		s += cast->condition->node->print(alignment + 1);
		s += indent(alignment) + "Do:";

		return s;

	}

	if (ID == CodePieceID::If) {

		auto cast = ((CodePiece_If*)this);

		string s = "If:\n";
		s += cast->condition->node->print(alignment + 1);
		s += indent(alignment) + "Then:";

		return s;

	}

	if (ID == CodePieceID::Else) return "Else:";

	if (ID == CodePieceID::Return) {

		auto cast = ((CodePiece_Return*)this);

		string s = "Return:\n";
		s += cast->return_expression->node->print(alignment + 1, false);

		return s;

	}

	if (ID == CodePieceID::LoopLogic) {

		auto cast = ((CodePiece_LoopLogic*)this);
		return cast->is_continue ? "Continue" : "Break";

	}

	return "";

}

AST::CodePieceBuilder::CodePieceBuilder() { }

#define _CodePiece_is_short cur->ID == CodePieceID::PatternMatch || cur->ID == CodePieceID::Body || cur->ID == CodePieceID::LoopLogic

void AST::CodePieceBuilder::scanNextPattern(ptr_Pattern p) {

	if (cur == nullptr) {

		cur = getNewCodePiece(p);

		if (_CodePiece_is_short) pushCurrentCodePiece();

		return;

	}

	if (cur->ID == CodePieceID::Assignment) {

		cur.cast<CodePiece_Assignment>()->RH = p.cast<Expression>()();

		pushCurrentCodePiece();
		return;

	}

	if (cur->ID == CodePieceID::NestedCode) {
		
		auto nest = cur.cast<CodePiece_NestedCode>();

		if (nest->body_pt1_done) {

			if (Builder_Body_Code::isElseKW(p->ID)) {

				while (true) {

					while (cur->ID != CodePieceID::NestedCode) cur = cur->owner;

					auto nest = cur.cast<CodePiece_NestedCode>();
					if (nest->header->ID == CodePieceID::If && nest->body_pt2 == nullptr) break;

					cur = cur->owner;

				}
				
				cur = getNewCodePiece(p, cur); 
				return; 
			
			}
			
			while (cur != nullptr) pushCurrentCodePiece(false);

			cur = getNewCodePiece(p);
			if (_CodePiece_is_short) pushCurrentCodePiece();
			
			return;

		}

		cur = getNewCodePiece(p, cur);
		if (_CodePiece_is_short) pushCurrentCodePiece();

		return;

	}

	if (cur->ID == CodePieceID::For) {

		cur.cast<CodePiece_For>()->header = p.cast<ForLoopHeader>()();

		pushCurrentCodePiece();
		return;

	}

	if (cur->ID == CodePieceID::While) {

		cur.cast<CodePiece_While>()->condition = p.cast<Expression>()();

		pushCurrentCodePiece();
		return;

	}

	if (cur->ID == CodePieceID::If) {

		cur.cast<CodePiece_If>()->condition = p.cast<Expression>()();

		pushCurrentCodePiece();
		return;

	}

	if (cur->ID == CodePieceID::Return) {

		cur.cast<CodePiece_Return>()->return_expression = p.cast<Expression>()();

		pushCurrentCodePiece();

	}

}

void AST::CodePieceBuilder::finish() { 
	
	while (cur != nullptr) pushCurrentCodePiece();
	for (auto piece : pieces) piece->removeCircularDependencies();

}

string AST::CodePieceBuilder::toString(int alignment) {

	if (pieces.size() == 0) return "";

	string s = "";
	for (auto piece : pieces) s += piece->print(alignment, piece != pieces[pieces.size() - 1]);

	return s;

};

AST::ptr_CodePiece AST::CodePieceBuilder::getNewCodePiece(ptr_Pattern p, ptr_CodePiece attach_to) {
	
	auto ptype = p->ID;
	bool is_else_kw = Builder_Body_Code::isElseKW(ptype);

	ptr_CodePiece new_piece;
	ptr_CodePiece_NestedCode owner_nest = attach_to.cast<CodePiece_NestedCode>();

	if (ptype == PatternID::Expression) {

		auto exp = p.cast<Expression>();
		auto node = exp->node;

		if (node->ID != NodeID::PatternMatch) return nullptr;

		ptr_CodePiece_PatternMatch pm = new CodePiece_PatternMatch();
		pm->pattern_match = exp();

		new_piece = pm.cast<CodePiece>();

	}

	else if (ptype == PatternID::Declaration) {

		auto sym = p.cast<Declaration>()->sym;

		ptr_CodePiece_Declaration decl = new CodePiece_Declaration();
		decl->new_var = sym();

		ptr_CodePiece_Assignment ass = new CodePiece_Assignment();
		ass->LH_declare = sym();
		
		decl->assignment = ass;
		ass->owner = decl.cast<CodePiece>();

		new_piece = ass.cast<CodePiece>();

	}

	else if (ptype == PatternID::Assignment) {

		auto LH = p.cast<Assignment>()->expression;

		ptr_CodePiece_Assignment ass = new CodePiece_Assignment();
		ass->LH_assign = LH();

		new_piece = ass.cast<CodePiece>();

	}

	else if (ptype == PatternID::Body_Function || ptype == PatternID::Body_Structure) {

		ptr_CodePiece_Body body = new CodePiece_Body();
		body->body = p();

		new_piece = body.cast<CodePiece>();

	}

	else if (ptype == PatternID::Keyword_for || ptype == PatternID::Keyword_while || ptype == PatternID::Keyword_if || is_else_kw) {

		ptr_CodePiece header;

		if (ptype == PatternID::Keyword_for) header = (CodePiece*)new CodePiece_For();
		else if (ptype == PatternID::Keyword_while) header = (CodePiece*)new CodePiece_While();
		else if (ptype == PatternID::Keyword_if) header = (CodePiece*)new CodePiece_If();
		else header = (CodePiece*)new CodePiece_Else();
		
		ptr_CodePiece_NestedCode nest = new CodePiece_NestedCode();
		auto cp_nest = nest.cast<CodePiece>();
		
		if (attach_to != nullptr) {

			nest->owner = attach_to;

			if (is_else_kw) owner_nest->body_pt2 = cp_nest;
			else owner_nest->body_pt1 = cp_nest;

		}

		nest->header = header;
		header->owner = cp_nest;
		
		if (is_else_kw) {

			nest->header_done = true;
			new_piece = cp_nest;

		}
		else new_piece = header;

		attach_to = nullptr;

	}

	else if (ptype == PatternID::Keyword_return) {

		ptr_CodePiece_Return ret = new CodePiece_Return();

		new_piece = ret.cast<CodePiece>();

	}

	else if (ptype == PatternID::Keyword_continue || ptype == PatternID::Keyword_break) {

		ptr_CodePiece_LoopLogic ll = new CodePiece_LoopLogic();
		ll->is_continue = ptype == PatternID::Keyword_continue;

		new_piece = ll.cast<CodePiece>();

	}

	if (attach_to != nullptr) if (new_piece != nullptr) if (new_piece->owner == nullptr) {

		new_piece->owner = attach_to;
		owner_nest->body_pt1 = new_piece;

	}

	return new_piece;

}

void AST::CodePieceBuilder::pushCurrentCodePiece(bool allow_else) {
	
	if (cur == nullptr) return;
	
	auto cur_loop = cur;

	while (true) {

		if (cur_loop->owner == nullptr) { pieces.push_back(cur_loop); cur = nullptr; return; }

		if (cur_loop->owner->ID == CodePieceID::NestedCode) {
			
			cur = cur_loop->owner;
			auto nest = cur.cast<CodePiece_NestedCode>();
			
			if (cur_loop == nest->header) { nest->header_done = true; return; }
			
			if (cur_loop == nest->body_pt1) { nest->body_pt1_done = true; return; }

			if (cur_loop == nest->body_pt2 && allow_else) { return; }

		}

		cur_loop = cur_loop->owner;

	}

}

AST::CodePiece_PatternMatch::CodePiece_PatternMatch() : CodePiece(CodePieceID::PatternMatch) { }

AST::CodePiece_Declaration::CodePiece_Declaration() : CodePiece(CodePieceID::Declaration) { }

AST::CodePiece_Assignment::CodePiece_Assignment() : CodePiece(CodePieceID::Assignment) { }

AST::CodePiece_Body::CodePiece_Body() : CodePiece(CodePieceID::Body){ }

managed_vec<AST::ptr_CodePiece>& AST::CodePiece_Body::pieces() {

	return body->ID == PatternID::Body_Function
		? ((Body_Function*)body)->builder.cast<Builder_Body_Function>()->code_pieces.pieces
		: ((Body_Structure*)body)->builder.cast<Builder_Body_Structure>()->code_pieces.pieces
	;

}

AST::CodePiece_NestedCode::CodePiece_NestedCode() : CodePiece(CodePieceID::NestedCode) { }

AST::CodePiece_For::CodePiece_For() : CodePiece(CodePieceID::For) { }

AST::CodePiece_While::CodePiece_While() : CodePiece(CodePieceID::While) { }

AST::CodePiece_If::CodePiece_If() : CodePiece(CodePieceID::If) { }

AST::CodePiece_Else::CodePiece_Else() : CodePiece(CodePieceID::Else) { }

AST::CodePiece_Return::CodePiece_Return() : CodePiece(CodePieceID::Return) { }

AST::CodePiece_LoopLogic::CodePiece_LoopLogic() : CodePiece(CodePieceID::LoopLogic) { }