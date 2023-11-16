#ifndef COMPILEERROR_HPP
#define COMPILEERROR_HPP

#include "./string.hpp"
#include "./vec.hpp"
#include "./../Code/Loader.hpp"
#include "./Printable.hpp"

struct CompileError : public Printable {

	bool error = false;
	string info;
	managed_vec<ptr_Printable> sources;
	string note;

	string toString(int alignment = 0);

};

#endif