#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(lui) f(auipc) f(jal) f(jalr)f(ebreak) f(lw) f(sw) f(addi) f(inv) f(nemu_trap)

def_all_EXEC_ID();
