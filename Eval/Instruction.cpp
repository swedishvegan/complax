
#include "./Instruction.hpp"

const int Eval::inst_lengths[] = {

    0,  // exit
    1,  // begfnc
    1,  // j
    2,  // jc
    5,  // call
    0,  // ret
    1,  // scpbeg
    0,  // scpend
    1,  // inp
    1,  // out
    0,  // tmrs
    1,  // tmrr
    1,  // rand
    //-1, // gc
    2,  // halloc
    2,  // hinit
    3,  // copy
    2,  // mov
    2,  // strlen
    2,  // arrlen
    3,  // arrcat
    2,  // castid
    2,  // castdi
    2,  // castsi
    2,  // castis
    2,  // castsd
    2,  // castds
    2,  // castas
    2,  // castai
    2,  // castia
    3,  // addi
    3,  // addd
    3,  // adda
    3,  // subi
    3,  // subd
    3,  // suba
    3,  // muli
    3,  // muld
    3,  // divi
    3,  // divd
    3,  // andi
    3,  // andb
    3,  // ori
    3,  // orb
    3,  // modi
    3,  // powi
    3,  // powd
    3,  // adds
    3,  // eqi
    3,  // eqd
    3,  // eqb
    3,  // eqa
    3,  // neqi
    3,  // neqd
    3,  // neqb
    3,  // neqa
    3,  // gti
    3,  // gtd
    3,  // gta
    3,  // gtei
    3,  // gted
    3,  // gtea
    3,  // lti
    3,  // ltd
    3,  // lta
    3,  // ltei
    3,  // lted
    3,  // ltea
    3,  // eqs
    3,  // neqs
    3,  // gts
    3,  // gtes
    3,  // lts
    3,  // ltes
    2,  // noti
    2   // notb

};

const int Eval::inst_info[] = {
    
    // Format: [number of args] [is_address...]

    0,                // exit
    1, 0,             // begfnc
    1, 0,             // j
    2, 0, 1,          // jc
    5, 0, 0, 0, 0, 0, // call
    0,                // ret
    1, 0,             // scpbeg
    0,                // scpend
    1, 1,             // inp
    1, 1,             // out
    0,                // tmrs
    1, 1,             // tmrr
    1, 1,             // rand
    // -1,              // gc
    2, 1, 1,          // halloc
    2, 1, 1,          // hinit
    3, 0, 1, 1,       // copy
    2, 1, 1,          // mov
    2, 1, 1,          // strlen
    2, 1, 1,          // arrlen
    3, 1, 1, 1,       // arrcat
    2, 1, 1,          // castid
    2, 1, 1,          // castdi
    2, 1, 1,          // castsi
    2, 1, 1,          // castis
    2, 1, 1,          // castsd
    2, 1, 1,          // castds
    2, 1, 1,          // castas
    2, 1, 1,          // castai
    2, 1, 1,          // castia
    3, 1, 1, 1,       // addi
    3, 1, 1, 1,       // addd
    3, 1, 1, 1,       // adda
    3, 1, 1, 1,       // subi
    3, 1, 1, 1,       // subd
    3, 1, 1, 1,       // suba
    3, 1, 1, 1,       // muli
    3, 1, 1, 1,       // muld
    3, 1, 1, 1,       // divi
    3, 1, 1, 1,       // divd
    3, 1, 1, 1,       // andi
    3, 1, 1, 1,       // andb
    3, 1, 1, 1,       // ori
    3, 1, 1, 1,       // orb
    3, 1, 1, 1,       // modi
    3, 1, 1, 1,       // powi
    3, 1, 1, 1,       // powd
    3, 1, 1, 1,       // adds
    3, 1, 1, 1,       // eqi
    3, 1, 1, 1,       // eqd
    3, 1, 1, 1,       // eqb
    3, 1, 1, 1,       // eqa
    3, 1, 1, 1,       // neqi
    3, 1, 1, 1,       // neqd
    3, 1, 1, 1,       // neqb
    3, 1, 1, 1,       // neqa
    3, 1, 1, 1,       // gti
    3, 1, 1, 1,       // gtd
    3, 1, 1, 1,       // gta
    3, 1, 1, 1,       // gtei
    3, 1, 1, 1,       // gted
    3, 1, 1, 1,       // gtea
    3, 1, 1, 1,       // lti
    3, 1, 1, 1,       // ltd
    3, 1, 1, 1,       // lta
    3, 1, 1, 1,       // ltei
    3, 1, 1, 1,       // lted
    3, 1, 1, 1,       // ltea
    3, 1, 1, 1,       // eqs
    3, 1, 1, 1,       // neqs
    3, 1, 1, 1,       // gts
    3, 1, 1, 1,       // gtes
    3, 1, 1, 1,       // lts
    3, 1, 1, 1,       // ltes
    2, 1, 1,          // noti
    2, 1, 1           // notb

};

int Eval::inst_info_offsets[Eval::inst::_size];

bool Eval::inst_initialized = false;

const char* Eval::inst_names[] = {

    "exit  ",
    "begfnc",
    "j     ",
    "jc    ",
    "call  ",
    "ret   ",
    "scpbeg",
    "scpend",
    "inp   ",
    "out   ",
    "tmrs  ",
    "tmrr  ",
    "rand  ",
    //"gc    ",
    "halloc",
    "hinit ",
    "copy  ",
    "mov   ",
    "strlen",
    "arrlen",
    "arrcat",
    "castid",
    "castdi",
    "castsi",
    "castis",
    "castsd",
    "castds",
    "castas",
    "castai",
    "castia",
    "addi  ",
    "addd  ",
    "adda  ",
    "subi  ",
    "subd  ",
    "suba  ",
    "muli  ",
    "muld  ",
    "divi  ",
    "divd  ",
    "andi  ",
    "andb  ",
    "ori   ",
    "orb   ",
    "modi  ",
    "powi  ",
    "powd  ",
    "adds  ",
    "eqi   ",
    "eqd   ",
    "eqb   ",
    "eqa   ",
    "neqi  ",
    "neqd  ",
    "neqb  ",
    "neqa  ",
    "gti   ",
    "gtd   ",
    "gta   ",
    "gtei  ",
    "gted  ",
    "gtea  ",
    "lti   ",
    "ltd   ",
    "lta   ",
    "ltei  ",
    "lted  ",
    "ltea  ",
    "eqs   ",
    "neqs  ",
    "gts   ",
    "gtes  ",
    "lts   ",
    "ltes  ",
    "noti  ",
    "notb  "

};

const char* Eval::access_names[] = {

    "none    ",
    "local   ",
    "global  ",
    "imm     ",
    "heap1ll ",
    "heap1gl ",
    "heap1lg ",
    "heap1gg ",
    "heap1li ",
    "heap1gi ",
    "heap8ll ",
    "heap8gl ",
    "heap8lg ",
    "heap8gg ",
    "heap8li ",
    "heap8gi "

}; 

void Eval::inst_init() {

    int cur_offset = 0;

    for (int i = 0; i < Eval::inst::_size; i++) {

        Eval::inst_info_offsets[i] = cur_offset;
        cur_offset += Eval::inst_lengths[i] + 1;

    }

    Eval::inst_initialized = true;

}