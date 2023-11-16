#include "./CompileError.hpp"

string CompileError::toString(int alignment) {

	string s = "ERROR: " + info;
	if (sources.size() == 0) return s;
	
	s += "\n";
	for (int i = 0; i < sources.size(); i++) s += sources[i]->print(alignment + 1, i < sources.size() - 1);
	if (note.size() > 0) s += "\n" + indent(alignment) + "NOTE: " + note;

	return s;

}