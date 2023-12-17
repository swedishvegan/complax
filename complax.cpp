#include <iostream>
#include <iomanip>

#include "./Eval/Program.hpp"

int main(int argc, char** argv) {

	double compile_time = 0.0;
	int exit_code = 0;

	{

		Eval::Program program(argc, argv);

		if (program.error.error) std::cout << program.error.print();

		compile_time = program.compilation_time;
		exit_code = (int)program.error.error;

	}

	//if (!exit_code) 
		std::cout 
			<< " Compilation finished in " 
			<< std::fixed << std::setprecision(3) 
			<< compile_time 
			<< " seconds with " 
			<< ptr_container::num_allocs
			<< " detected memory leak(s).\n"
		;

	return exit_code;
	
}