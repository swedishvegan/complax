#include "./FreePattern.hpp"

AST::FreePattern::FreePattern(PatternID ID) : Pattern(ID) { }

string AST::FreePattern::getInfo(int) { return removeNewlines(name.c_str()); }

#define AST_IMPL_FREEPATTERN(classname, primer, terminators, alphabet, ignore_whitespace, include_terminator) \
\
AST::classname::classname() : FreePattern(PatternID::classname) { } \
AST::Scanner_##classname::Scanner_##classname(Code::Loader& loader, int start, int end, int cur_best_start) : Scanner_FreePattern<classname>(loader, start, end, cur_best_start, primer, (const char*)terminators, alphabet, ignore_whitespace, include_terminator) { }

#define _legal_chars(s) AST::Alphabet{ s, true }
#define _illegal_chars(s) AST::Alphabet{ s }

#define _legal_alphabet _legal_chars(variable_alphabet)
#define _illegal_alphabet _illegal_chars("{}=")

AST_IMPL_FREEPATTERN(Argument, '{', "}", _legal_alphabet, true, true);
AST_IMPL_FREEPATTERN(Filler, (char)0, "{}", _illegal_alphabet, true, false);
AST_IMPL_FREEPATTERN(Declaration_base, (char)0, "=", _legal_alphabet, true, true);
AST_IMPL_FREEPATTERN(Label, '{', "}", _legal_alphabet, true, true);