#include "./Keyword.hpp"

AST::Keyword::Keyword(PatternID ID, const char* kw) : Pattern(ID), kw(kw) { }

#define AST_IMPL_KEYWORD_(classname, kw_string) \
\
AST::Keyword_##classname::Keyword_##classname() : Keyword(PatternID::Keyword_##classname, kw_string) { precedence = 1; } \
AST::Scanner_Keyword_##classname::Scanner_Keyword_##classname(Code::Loader& loader, int start, int end, int cur_best_start) : Scanner_Keyword(loader, start, end, cur_best_start, kw_string) { }

#define AST_IMPL_KEYWORD(classname) AST_IMPL_KEYWORD_(classname, #classname)

AST_IMPL_KEYWORD(startprogram);

AST_IMPL_KEYWORD(function);
AST_IMPL_KEYWORD(structure);

AST_IMPL_KEYWORD(wrest);
AST_IMPL_KEYWORD(wprec);
AST_IMPL_KEYWORD_(precarrow, "<->");
AST_IMPL_KEYWORD(wlabl);

AST_IMPL_KEYWORD(import);
AST_IMPL_KEYWORD(include);

AST_IMPL_KEYWORD(for);
AST_IMPL_KEYWORD(while);

AST_IMPL_KEYWORD(if);
AST_IMPL_KEYWORD(else);
AST_IMPL_KEYWORD_(else_comma, "else,");
AST_IMPL_KEYWORD_(else_colon, "else:");

AST_IMPL_KEYWORD(return);
AST_IMPL_KEYWORD(returns);

AST_IMPL_KEYWORD(continue);
AST_IMPL_KEYWORD(break);