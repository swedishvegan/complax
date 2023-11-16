cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(complax)

message("Compiler being used: ${CMAKE_CXX_COMPILER_ID}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/EHsc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-O3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-O3")
else()
	message(WARNING, "The Complax library has not been tested with this compiler before. Use at your own risk.")
endif()

add_executable(lax lax.cpp)

add_executable(complax complax.cpp)

target_sources(
	complax
	PUBLIC AST/utility_functions.cpp
	PUBLIC AST/utility_functions.hpp
	PUBLIC AST/Builder/Builder.cpp
	PUBLIC AST/Builder/Builder.hpp
	PUBLIC AST/Builder/BuilderID.cpp
	PUBLIC AST/Builder/BuilderID.hpp
	PUBLIC AST/Builder/implementations/Builder_Body.hpp
	PUBLIC AST/Builder/implementations/Builder_Body_Code.cpp
	PUBLIC AST/Builder/implementations/Builder_Body_Code.hpp
	PUBLIC AST/Builder/implementations/Builder_Body_Function.cpp
	PUBLIC AST/Builder/implementations/Builder_Body_Function.hpp
	PUBLIC AST/Builder/implementations/Builder_Body_Precedence.cpp
	PUBLIC AST/Builder/implementations/Builder_Body_Precedence.hpp
	PUBLIC AST/Builder/implementations/Builder_Body_Restrictions.cpp
	PUBLIC AST/Builder/implementations/Builder_Body_Restrictions.hpp
	PUBLIC AST/Builder/implementations/Builder_Body_Structure.cpp
	PUBLIC AST/Builder/implementations/Builder_Body_Structure.hpp
	PUBLIC AST/Builder/implementations/Builder_Header.cpp
	PUBLIC AST/Builder/implementations/Builder_Header.hpp
	PUBLIC AST/Builder/implementations/GlobalScopeBuilder.cpp
	PUBLIC AST/Builder/implementations/GlobalScopeBuilder.hpp
	PUBLIC AST/Builder/implementations/implement_builder.hpp
	PUBLIC AST/CodePiece/CodePiece.cpp
	PUBLIC AST/CodePiece/CodePiece.hpp
	PUBLIC AST/CodePiece/CodePieceBuilder.hpp
	PUBLIC AST/CodePiece/CodePiece_implementations.hpp
	PUBLIC AST/Pattern/Pattern.cpp
	PUBLIC AST/Pattern/Pattern.hpp
	PUBLIC AST/Pattern/PatternID.cpp
	PUBLIC AST/Pattern/PatternID.hpp
	PUBLIC AST/Pattern/implementations/Assignment.cpp
	PUBLIC AST/Pattern/implementations/Assignment.hpp
	PUBLIC AST/Pattern/implementations/Body.hpp
	PUBLIC AST/Pattern/implementations/Declaration.cpp
	PUBLIC AST/Pattern/implementations/Declaration.hpp
	PUBLIC AST/Pattern/implementations/ForLoopHeader.cpp
	PUBLIC AST/Pattern/implementations/ForLoopHeader.hpp
	PUBLIC AST/Pattern/implementations/FreePattern.cpp
	PUBLIC AST/Pattern/implementations/FreePattern.hpp
	PUBLIC AST/Pattern/implementations/Header.hpp
	PUBLIC AST/Pattern/implementations/Keyword.cpp
	PUBLIC AST/Pattern/implementations/Keyword.hpp
	PUBLIC AST/Pattern/implementations/Literal.cpp
	PUBLIC AST/Pattern/implementations/Literal.hpp
	PUBLIC AST/Pattern/implementations/Wrapper.hpp
	PUBLIC AST/Pattern/implementations/Expression/Expression.cpp
	PUBLIC AST/Pattern/implementations/Expression/Expression.hpp
	PUBLIC AST/Pattern/implementations/Expression/Node.cpp
	PUBLIC AST/Pattern/implementations/Expression/Node.hpp
	PUBLIC AST/Symbol/BuiltInSymbols.cpp
	PUBLIC AST/Symbol/BuiltInSymbols.hpp
	PUBLIC AST/Symbol/Symbol.cpp
	PUBLIC AST/Symbol/Symbol.hpp
	PUBLIC AST/Symbol/SymbolSearchTree.cpp
	PUBLIC AST/Symbol/SymbolSearchTree.hpp
	PUBLIC AST/Symbol/SymbolSearchTreeBase.hpp
	PUBLIC AST/Symbol/SymbolTable.cpp
	PUBLIC AST/Symbol/SymbolTable.hpp
	PUBLIC AST/Symbol/SymbolTableLinker.cpp
	PUBLIC AST/Symbol/SymbolTableLinker.hpp
	PUBLIC AST/Type/Restrictions.cpp
	PUBLIC AST/Type/Restrictions.hpp
	PUBLIC AST/Type/Type.cpp
	PUBLIC AST/Type/Type.hpp
	PUBLIC Code/Bundle.cpp
	PUBLIC Code/Bundle.hpp
	PUBLIC Code/Loader.cpp
	PUBLIC Code/Loader.hpp
	PUBLIC Eval/BytecodeBlock.cpp
	PUBLIC Eval/BytecodeBlock.hpp
	PUBLIC Eval/CodeEvaluator.cpp
	PUBLIC Eval/CodeEvaluator.hpp
	PUBLIC Eval/Evaluator.cpp
	PUBLIC Eval/Evaluator.hpp
	PUBLIC Eval/Instruction.hpp
	PUBLIC Eval/NodeEvaluator.cpp
	PUBLIC Eval/NodeEvaluator.hpp
	PUBLIC Eval/Program.cpp
	PUBLIC Eval/Program.hpp
	PUBLIC Eval/ProgramDataTable.cpp
	PUBLIC Eval/ProgramDataTable.hpp
	PUBLIC util/CompileError.cpp
	PUBLIC util/CompileError.hpp
	PUBLIC util/map.hpp
	PUBLIC util/Printable.cpp
	PUBLIC util/Printable.hpp
	PUBLIC util/ptr.hpp
	PUBLIC util/string.hpp
	PUBLIC util/Timer.hpp
	PUBLIC util/vec.hpp
	PUBLIC util/vstack.hpp
)
