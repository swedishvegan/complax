#ifndef BUILDERID_HPP
#define BUILDERID_HPP

namespace AST {

	enum class BuilderID {

		GlobalScopeBuilder,

		Builder_Header_Function,
		Builder_Header_Structure,

		Builder_Body_Function,
		Builder_Body_Structure,
		Builder_Body_Restrictions,
		Builder_Body_Precedence,

		_size

	};

	const char* getBuilderIDString(BuilderID ID);

}

#endif