#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include "./../Pattern.hpp"

// Base class for wrapper objects around child Builders

namespace AST {

	template <PatternID PID>
	struct Wrapper : public Pattern {

		ptr_Builder builder = nullptr;

		Wrapper() : Pattern(PID) { }

		string getInfo(int alignment) { auto info = builder->printChildren(alignment + 1); if (info.size() == 0) return ""; return "\n" + info; }

	};

	template <typename WrapperType, typename BuilderType, typename... BuilderArgs>
	struct Scanner_Wrapper : public Scanner {

		Scanner_Wrapper(Code::Loader& loader, int start, int end, int cur_best_start, int scope, BuilderArgs... builder_args) : Scanner(loader, 0, 0, 0) {
			
			int s = loader.getNextScopeStart(start, scope); 
			if (s < 0 || s >= end || s > cur_best_start) return;
			if (!isRangeEmpty(loader, start, s - 1)) return;

			int e = loader.getNextScopeStart(s, scope - 1);
			if (e < 0 || e > end) return;

			WrapperType header;

			header.start = loader(s - 1);
			header.end = loader(e);
			
			ptr<BuilderType> builder = new BuilderType(loader, s, e - 1, scope, builder_args...);

			if (builder->error.error) header.error = builder->error;
			else header.builder = builder.template cast<Builder>();

			result = ptr<WrapperType>(new WrapperType(header)).template cast<Pattern>();

		}

	};

}

#endif