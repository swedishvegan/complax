#include <fstream>
#include "./Program.hpp"
#include "./CodeEvaluator.hpp"
#include "./../Code/Bundle.hpp"
#include "./../util/Timer.hpp"

Eval::Program::Program(int argc, char** argv) {

    ptr_Evaluator evaluator = nullptr;
    Code::ptr_Bundle codebase;

    Timer timer;

    parseArgs(argc, argv);

    if (!error.error) {

        codebase = new Code::Bundle();
        auto main_file = codebase->operator[](input_filename.c_str());
        
        if (main_file.error.error) error = main_file.error;
        else if (main_file.builder->error.error) error = main_file.builder->error;
        else {

            evaluator = new Eval::Evaluator();
            error = evaluator->error;

        }

        compilation_time = timer.time();
        
    }
    
    if (error.error) return;
    
    try {

        std::ofstream output_file(output_filename, std::ios::binary | std::ios::out);

        output_file.write("LAX-EXECUTABLE", 14);

        output_file << (size_t)Eval::Evaluator::program_data.size();
        output_file.write(Evaluator::program_data.data(), Evaluator::program_data.size());

        output_file << Evaluator::program_data.initial_stack.size();
        output_file.write((char*)Evaluator::program_data.initial_stack.data(), Evaluator::program_data.initial_stack.size() * sizeof(instruction));

        output_file << evaluator->program_bytecode.instructions.size();
        output_file.write((char*)evaluator->program_bytecode.instructions.data(), evaluator->program_bytecode.instructions.size() * sizeof(instruction));

    }
    catch(...) {

        error.error = true;
        error.info = "Failed to write to file '" + output_filename + "'.";

        return;

    }

    if (emit_syntax_tree) {

        for (auto file : codebase->file_map) {

            string syntax_tree_filename(file.first + ".syntax_tree");

            try {
                
                std::ofstream syntax_tree(syntax_tree_filename, std::ios::binary | std::ios::out);

                auto syntax_tree_string = file.second.builder->print();
                syntax_tree << syntax_tree_string;

            }
            catch (...) {

                error.error = true;
                error.info = "Failed to write to file '" + syntax_tree_filename + "'.";

                break;

            }

        }

    }

    if (error.error) return;

    if (emit_bytecode) {

        string bytecode_string;

        bytecode_string += Evaluator::program_data.print();
        bytecode_string += evaluator->program_bytecode.print();

        string bytecode_filename = output_filename + ".bytecode";

        try {

            std::ofstream bytecode(bytecode_filename, std::ios::binary | std::ios::out);
            bytecode << bytecode_string;

        }
        catch (...) {

            error.error = true;
            error.info = "Failed to write to file '" + bytecode_filename + "'.";

        }

    }

}

void Eval::Program::parseArgs(int argc, char** argv) {

    string error_info;

    if (argc < 3) { error_info = "Not enough arguments."; }

    else {

        input_filename = argv[1];
        output_filename = argv[2];

        for (int arg = 3; arg < argc; arg++) {

            string cur_arg = argv[arg];

            if (cur_arg == "--emit_syntax_tree") emit_syntax_tree = true;
            else if (cur_arg == "--emit_bytecode") emit_bytecode = true;

            else { error_info = "Unrecognized flag: '" + cur_arg + "'."; break; }

        }

    }

    if (error_info.size() > 0) {

        error.error = true;
        error.info = error_info + " Argument format: [input filename] [output filename] [optional flags (--emit_syntax_tree or --emit_bytecode)]";

    }

}