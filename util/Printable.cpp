#include "./Printable.hpp"

string Printable::print(int alignment, bool add_newline) { return indent(alignment) + this->toString(alignment) + (add_newline ? "\n" : ""); }

Printable::~Printable() { }

string Printable::indent(int nind) {

	if (nind == 0) return " ";

	string ind;
	for (int i = 0; i < nind - 1; i++) ind += "|  ";
	return " " + ind + "| ";

}

PrintableString::PrintableString(string str) : str(str) { }
PrintableString::PrintableString(const char* str) : str(str) { }

string PrintableString::toString(int) { return str; }