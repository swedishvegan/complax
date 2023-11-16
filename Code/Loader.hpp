#ifndef LOADER_HPP
#define LOADER_HPP

#include <fstream>
#include "./../util/string.hpp"
#include "./../util/vec.hpp"
#include "./../util/ptr.hpp"

// Loads code from a file

namespace Code {

	struct Position { 
		
		int index = 0;
		int line = 0; 
		int offset = 0;
		int scope = 0;
		bool is_whitespace = false;
		bool is_comment = false;

		string toString() const;
	
	};

	bool operator < (Position, Position);
	bool operator <= (Position, Position);
	bool operator > (Position, Position);
	bool operator >= (Position, Position);
	bool operator == (Position, Position);
	bool operator != (Position, Position);

	struct Loader {

		Loader(managed_string file_path);

		string getErrorMessage() const;

		int size() const;

		char operator [] (int idx) const;

		Position operator () (int idx) const;

		int getNextScopeStart(int idx, int scope);

	protected:

		struct Bracket { int idx; int scope; bool is_left; };

		managed_string code;
		managed_vec<Position> codep;
		managed_vec<Bracket> brackets;


		string error;

		void loadCode(managed_string);

		void parseBrackets();

		void processCode();

	};

	using ptr_Loader = ptr<Loader>;

}

#endif