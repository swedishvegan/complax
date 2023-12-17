#include "./ProgramDataTable.hpp"
#include "./NodeEvaluator.hpp"
#include "./../AST/Symbol/Symbol.hpp"

Eval::ProgramDataTable::ProgramDataTable() {

    managed_string s;
    getStackPosition(nullptr, s);

}

pointer Eval::ProgramDataTable::getStackPosition(void* literal_node, managed_string& s) {
    
    if (s.size() == 0 && next_avail_index > 0) return 0;

    if (next_avail_index == 0) { 
        
        offset = (8 - (pointer)the_data.data() % 8) % 8;
        count = offset;
        
        the_data.resize(offset); 
        
    }

    pointer strsize = s.size() + 1;

    auto find = global_variable_indices.find(literal_node);
    if (find != global_variable_indices.end()) return find->second;

    auto offset8b = (8 - (count - offset) % 8) % 8;
    auto space_needed = offset8b + strsize + 8;

    the_data.resize(count + space_needed);

    count += offset8b;

    char* data_start = the_data.data() + count;
    *(pointer*)data_start = strsize;

    count += 8;
    data_start += 8;

    initial_stack.push_back(count);
    initial_stack_types.push_back(StackType{ AST::Type::String, false });

    for (int i = 0; i < strsize - 1; i++) data_start[i] = s[i];
    data_start[strsize - 1] = (char)0;

    count += strsize;

    auto pos = next_avail_index;

    global_variable_indices[literal_node] = pos;
    next_avail_index++;

    return pos;

}

pointer Eval::ProgramDataTable::getStackPosition(void* global_var) {
    
    auto find = global_variable_indices.find(global_var);
				
    if (find == global_variable_indices.end()) {

        if (next_avail_index == 0) { 
        
            offset = (8 - (pointer)the_data.data() % 8) % 8;
            count = offset;
            
            the_data.resize(offset); 
            
        }

        auto& eval = ((AST::Symbol*)global_var)->evaluator;
        instruction in;

        if (eval.eval_type == AST::Type::String) in = initial_stack[getStackPosition(eval.nd, eval.value<managed_string>())];
        else in = eval.value<int64_t>();

        initial_stack.push_back(in);
        initial_stack_types.push_back(StackType{ eval.eval_type, true });

        auto pos = next_avail_index;

        global_variable_indices[global_var] = pos;
        next_avail_index++;

        return pos;

    }

    return find->second;

}

pointer Eval::ProgramDataTable::size() { return count - offset; }

char* Eval::ProgramDataTable::data() { return the_data.data() + offset; }

std::string Eval::ProgramDataTable::toString(int alignment) {

    std::string s = "DataTable:\n";

    s += indent(alignment + 1) + "GlobalValues:\n";

    for (int i = 0; i < initial_stack.size(); i++) {

        s += indent(alignment + 2) + AST::pad("(" + std::to_string(i) + ")", 11) + " ";
        
        auto st = initial_stack_types[i];
        
        if (st.is_variable) {

            s += "GlobalVariable: ";

            auto d = initial_stack[i];

            if (st.AST_Type == AST::Type::Integer) s += std::to_string(d);
            else if (st.AST_Type == AST::Type::Decimal) s += std::to_string(*(double*)(&d));
            else if (st.AST_Type == AST::Type::Bool) s += std::to_string(*(bool*)(&d));
            else if (st.AST_Type == AST::Type::Bool) s += "Ascii (" + std::to_string((int)*(unsigned char*)(&d));
            else if (st.AST_Type == AST::Type::String) s += "(string literal w/ index = " + std::to_string(d) + ")";
            else s += "(uninitialized heap object)";

        }
        
        else s += "StringLiteral (index = " + std::to_string(initial_stack[i]) + ")";

        s += "\n";

    }

    s += indent(alignment + 1) + "BuiltInStrings:\n";

    pointer p = 0;
    pointer sz = size();
    char* d = data();

    while (p < sz) {

        p += (8 - p % 8) % 8;
        auto strlen = *(pointer*)(d + p);

        p += 8;
        s += indent(alignment + 2) + AST::pad("(" + std::to_string(p) + ")", 11) + " ";

        auto printlen = strlen;
        if (printlen > 40) printlen = 40;

        for (int i = 0; i < printlen; i++) {

            auto c = (d + p)[i];
            if (!AST::isWhitespace(c) && c != (char)0) s += c;

        }
        if (printlen < strlen) s += "...";
        if (printlen == 0) s += "(empty string)";

        p += strlen;
        if (p < sz) s += "\n";

    }

    return s;

}