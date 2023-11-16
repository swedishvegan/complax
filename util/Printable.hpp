#ifndef PRINTABLE_HPP
#define PRINTABLE_HPP

#include "./string.hpp"
#include "./ptr.hpp"
#include "./vec.hpp"

// Base class for printable objects, used for debugging and printing compiler info

struct Printable {

	string print(int alignment = 0, bool add_newline = true);

	virtual string toString(int alignment) = 0;

	virtual ~Printable();

	static string indent(int);

	bool single_line = false; // Set this to true in order to indicate that the printed string should be limited to one line

};

using ptr_Printable = ptr<Printable>;

struct PrintableString : public Printable {

	string str;

	PrintableString(string);
	PrintableString(const char*);

	string toString(int);

};

using ptr_PrintableString = ptr<PrintableString>;

#endif