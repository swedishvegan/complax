#include "./BuilderID.hpp"

const char* _BuilderID_strings[] = {

	"GlobalScopeBuilder",

	"Builder_Header_Function",
	"Builder_Header_Structure",

	"Builder_Body_Function",
	"Builder_Body_Structure",
	"Builder_Body_Restrictions",
	"Builder_Body_Precedence",

};

const char* AST::getBuilderIDString(BuilderID ID) { return _BuilderID_strings[(int)ID]; }