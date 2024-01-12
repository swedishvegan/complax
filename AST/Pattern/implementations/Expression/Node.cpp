#include "Node.hpp"
#include "Expression.hpp"

AST::Node::Node(NodeID ID) : ID(ID) { }

bool AST::Node::isArgument() { if (ID == NodeID::None || ID == NodeID::Filler) return false; return true; }

bool AST::Node::contains(ptr_Symbol sym) {

	if (sym == nullptr) return false;

	if (ID == NodeID::Expression) return ((ExpressionNode*)this)->expression->node->contains(sym);

	if (ID == NodeID::ArrayInitializer) {

		for (auto exp : ((ArrayInitializerNode*)this)->components)
			if (exp->node->contains(sym)) return true;

		return false;

	}

	if (ID == NodeID::Filler || ID == NodeID::Variable) return ((KeywordNode*)this)->sym == sym;

	if (ID == NodeID::PatternMatch) { 
		
		auto pm = (PatternMatchNode*)this;

		auto& comps = pm->components;
		for (auto comp : comps) if (comp->contains(sym)) return true;
	
	}

	if (ID == NodeID::StructureMember) return ((StructureMemberNode*)this)->base->contains(sym);

	return false;

}

void AST::Node::updateReferenceCounts() {

	if (ID == NodeID::Expression) ((ExpressionNode*)this)->expression->node->updateReferenceCounts();

	else if (ID == NodeID::ArrayInitializer)
		for (auto exp : ((ArrayInitializerNode*)this)->components) exp->node->updateReferenceCounts();

	else if (ID == NodeID::Variable) ((VariableNode*)this)->sym->num_references++;

	else if (ID == NodeID::PatternMatch) {

		auto pm = (PatternMatchNode*)this;

		for (auto sym : pm->bundle->elements) sym->num_references++;
		for (auto n : pm->components) n->updateReferenceCounts();

	}

	else if (ID == NodeID::StructureMember) ((StructureMemberNode*)this)->base->updateReferenceCounts();

}
#include <iostream>
AST::SymbolBundle* AST::Node::findPrecedenceConflicts() {
	
	if (ID == NodeID::Expression) return ((ExpressionNode*)this)->expression->node->findPrecedenceConflicts();

	if (ID == NodeID::ArrayInitializer) {

		for (auto exp : ((ArrayInitializerNode*)this)->components) {
			
			auto cft = exp->node->findPrecedenceConflicts();
			if (cft) return cft;

		}

		return nullptr;

	}

	if (ID == NodeID::PatternMatch) {

		auto pm = (PatternMatchNode*)this;
		auto bundle = pm->bundle;

		if (bundle->has_unacknowledged_updates) {
			
			auto& syms = bundle->elements;

			for (int i = 0; i < syms.size(); i++) for (int j = i + 1; j < syms.size(); j++) {

				auto sym1 = syms[i].cast<HeaderSymbol>();
				auto sym2 = syms[j].cast<HeaderSymbol>();
				
				bool precedence_conflict = false;

				if (sym1->operator[](0).is_argument && sym1->precedence_lh != sym2->precedence_lh) precedence_conflict = true;
				if (sym1->operator[](sym1->size() - 1).is_argument && sym1->precedence_rh != sym2->precedence_rh) precedence_conflict = true;

				if (precedence_conflict) {
					
					bundle->conflict_first = i;
					bundle->conflict_second = j;

					return bundle();

				}

			}

			bundle->has_unacknowledged_updates = false;

		}

		for (auto comp : pm->components) {
			
			auto cft = comp->findPrecedenceConflicts();
			if (cft) return cft;

		}

		return nullptr;

	}

	if (ID == NodeID::StructureMember) return ((StructureMemberNode*)this)->base->findPrecedenceConflicts();

	return nullptr;

}

CompileError* AST::Node::getContainedError() {

	if (ID == NodeID::Literal) {

		auto& err = ((LiteralNode*)this)->literal->error;
		return err.error ? &err : nullptr;

	}

	else if (ID == NodeID::Expression) {

		auto& err = ((ExpressionNode*)this)->expression->error;
		return err.error ? &err : nullptr;

	}

	else if (ID == NodeID::ArrayInitializer) {

		for (auto exp : ((ArrayInitializerNode*)this)->components) {

			auto err = exp->node->getContainedError();
			if (err) return err;

		}

	}

	else if (ID == NodeID::PatternMatch) {

		auto pm = (PatternMatchNode*)this;

		auto& comps = pm->components;
		for (auto comp : comps) { auto err = comp->getContainedError(); if (err) return err; }

	}

	else if (ID == NodeID::StructureMember) return ((StructureMemberNode*)this)->base->getContainedError();

	return nullptr;

}

bool AST::Node::equals(ptr_Node lh, ptr_Node rh) {

	if (lh->ID != rh->ID) return false;

	if (lh->start != rh->start) return false;
	if (lh->end != rh->end) return false;

	if (lh->ID == NodeID::Expression) return equals(lh.cast<ExpressionNode>()->expression->node, rh.cast<ExpressionNode>()->expression->node);
	
	if (lh->ID == NodeID::ArrayInitializer) {

		auto lh_ai = lh.cast<ArrayInitializerNode>();
		auto rh_ai = rh.cast<ArrayInitializerNode>();

		if (lh_ai->components.size() != rh_ai->components.size()) return false;
		for (int i = 0; i < lh_ai->components.size(); i++) if (!equals(lh_ai->components[i]->node, rh_ai->components[i]->node)) return false;

		return true;

	}

	if (lh->ID == NodeID::Literal) return lh->start == rh->start && lh->end == rh->end;
	
	if (lh->ID == NodeID::Filler || lh->ID == NodeID::Variable) {

		auto lh_kw = lh.cast<KeywordNode>();
		auto rh_kw = rh.cast<KeywordNode>();

		return lh_kw->sym == rh_kw->sym && lh_kw->sym_idx == rh_kw->sym_idx;

	}

	if (lh->ID == NodeID::PatternMatch) {

		auto lh_pm = lh.cast<PatternMatchNode>();
		auto rh_pm = rh.cast<PatternMatchNode>();

		if (lh_pm->components.size() != rh_pm->components.size()) return false;
		for (int i = 0; i < lh_pm->components.size(); i++) if (!equals(lh_pm->components[i], rh_pm->components[i])) return false;

		return true;

	}

	if (lh->ID == NodeID::StructureMember) {

		auto lh_sm = lh.cast<StructureMemberNode>();
		auto rh_sm = rh.cast<StructureMemberNode>();

		if (lh_sm->base != rh_sm->base) return false;
		
		if (lh_sm->members.size() != rh_sm->members.size()) return false;
		for (int i = 0; i < lh_sm->members.size(); i++) if (lh_sm->members[i] != rh_sm->members[i]) return false;

		return true;

	}

	if (lh->ID == NodeID::StructureMemberKW) {

		auto lh_sm = lh.cast<StructureMemberKWNode>();
		auto rh_sm = rh.cast<StructureMemberKWNode>();

		return lh_sm->kw == rh_sm->kw;

	}

	return false;

}

bool AST::Node::necessitates(NodeID LH, NodeID RH) {

	if (LH == NodeID::None) return RH != NodeID::Label;

	if (LH == NodeID::Expression) return RH != NodeID::Label;

	if (LH == NodeID::ArrayInitializer) return RH != NodeID::Label;

	if (LH == NodeID::Literal) return RH == NodeID::Literal;

	if (LH == NodeID::Variable) return RH == NodeID::Variable;

	if (LH == NodeID::PatternMatch) return RH != NodeID::Label;

	if (LH == NodeID::StructureMember) return RH != NodeID::Label;

	if (LH == NodeID::Label) return RH == NodeID::Label;

	return false;

}

AST::EmptyNode::EmptyNode(int start) : Node(NodeID::None) { this->start = start; this->end = start; }

string AST::EmptyNode::toString(int) { return "EmptyNode"; }

AST::ExpressionNode::ExpressionNode(ptr_Expression expression, int start, int end) : Node(NodeID::Expression), expression(expression) { this->start = start; this->end = end; }

string AST::ExpressionNode::toString(int alignment) { return expression->node->toString(alignment); }

AST::ArrayInitializerNode::ArrayInitializerNode() : Node(NodeID::ArrayInitializer) { }

string AST::ArrayInitializerNode::toString(int alignment) {

	if (components.size() == 0) return "Array (empty)";

	string s = "Array:\n";
	for (auto exp : components) s += exp->node->print(alignment + 1, exp != components[components.size() - 1]);

	return s;

}

AST::LiteralNode::LiteralNode(ptr_Literal literal) : Node(NodeID::Literal), literal(literal) { this->start = literal->start.index; this->end = literal->end.index; }

string AST::LiteralNode::toString(int) { return "LiteralNode:    " + removePadding(removeNewlines(literal->info.c_str())); }

AST::KeywordNode::KeywordNode(NodeID ID, ptr_Symbol sym, int sym_idx, int start, int end) : Node(ID), sym(sym), sym_idx(sym_idx) { this->start = start; this->end = end; }

string AST::KeywordNode::toString(int) { return (ID == NodeID::Variable ? "VariableNode:   " : "FillerNode:     ") + string(sym->operator[](sym_idx).name.c_str()); }

AST::VariableNode::VariableNode(ptr_Symbol sym, int start, int end) : KeywordNode(NodeID::Variable, sym, 0, start, end) { }

AST::FillerNode::FillerNode(ptr_SymbolBundle bundle, int sym_idx, int start, int end) : KeywordNode(NodeID::Filler, bundle->elements[0], sym_idx, start, end), bundle(bundle) { }

AST::PatternMatchNode::PatternMatchNode(ptr_SymbolBundle bundle) : Node(NodeID::PatternMatch), bundle(bundle) { }

void AST::PatternMatchNode::addComponent(ptr_Node node) {

	if (components.size() == 0) start = node->start;
	end = node->end;

	components.push_back(node);

}

string AST::PatternMatchNode::toString(int alignment) {

	string desc;
	auto sym = bundle->elements[0].cast<HeaderSymbol>();

	if (sym->description.size() > 0) desc = sym->description;
	else desc = "PatternMatch";
	
	string s = desc + ":\n";
	for (int i = 0; i < components.size(); i++) s += components[i]->print(alignment + 1, i < components.size() - 1);

	return s;

}

AST::StructureMemberNode::StructureMemberNode(ptr_Node base) : Node(NodeID::StructureMember), base(base) { start = base->start; end = base->end; }

void AST::StructureMemberNode::addMember(ptr_StructureMemberInfoBundle member, int new_end) { members.push_back(member); end = new_end; }

string AST::StructureMemberNode::toString(int alignment) {

	string s = "StructureMemberNode:\n";

	s += indent(alignment + 1) + "BaseNode:\n" + base->print(alignment + 2);

	s += indent(alignment + 1) + "MemberTrace:\n";

	for (int i = 0; i < members.size(); i++) s += indent(alignment + 2) + members[i]->getSignature()->c_str() + (i < members.size() - 1 ? "\n" : "");

	return s;

}

AST::StructureMemberKWNode::StructureMemberKWNode(ptr_StructureMemberInfoBundle kw, int start, int end) : Node(NodeID::StructureMemberKW), kw(kw) { this->start = start; this->end = end; }

string AST::StructureMemberKWNode::toString(int) { return "MemberKWNode:   " + string(kw->getSignature()->c_str()); }

AST::LabelNode::LabelNode(ptr_HeaderSymbol sym, int start, int end) : Node(NodeID::Label), sym(sym) { this->start = start; this->end = end; }

string AST::LabelNode::toString(int) { return "LabelNode:      " + string(sym->label.c_str()); }