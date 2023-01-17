#include "common.h"

def_EHelper(beq) {
  if (*ddest == *dsrc1) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(bne) {
  if (*ddest != *dsrc1) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(bltu) {
  if (*ddest < *dsrc1) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(bgeu) {
  if (*ddest >= *dsrc1) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(blt) {
  if ((sword_t)(*ddest) < (sword_t)(*dsrc1)) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(bge) {
  if ((sword_t)(*ddest) >= (sword_t)(*dsrc1)) {
    rtl_addi(s, &s->dnpc, &s->pc, id_src2->imm);
  }
}

def_EHelper(jal) {
  rtl_addi(s, ddest, &s->pc, 4);
  rtl_addi(s, &s->dnpc, &s->pc, id_src1->imm);
}

def_EHelper(jalr) {
  rtl_addi(s, ddest, &s->pc, 4);
  rtl_addi(s, &s->dnpc, dsrc1, id_src2->imm);
}

def_EHelper(ebreak) {
  rtl_hostcall(s, HOSTCALL_EXIT, NULL, &gpr(10), NULL, 0); // gpr(10) is $a0
}
