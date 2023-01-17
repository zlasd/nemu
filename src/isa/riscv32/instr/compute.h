#define def_EHelper_Simple(name) def_EHelper(name) { rtl_##name(s, ddest, dsrc1, dsrc2); }

def_EHelper_Simple(add)
def_EHelper_Simple(sub)
def_EHelper_Simple(xor)
def_EHelper_Simple(or)
def_EHelper_Simple(and)
def_EHelper_Simple(sll)
def_EHelper_Simple(srl)
def_EHelper_Simple(sra)

def_EHelper(slt) {
  if ((sword_t)(*dsrc1) < (sword_t)(*dsrc2)) {
    rtl_li(s, ddest, 1);
  }
  rtl_li(s, ddest, 0);
}

def_EHelper(sltu) {
  if (*dsrc1 < *dsrc2) {
    rtl_li(s, ddest, 1);
  }
  rtl_li(s, ddest, 0);
}

#define def_EHelper_Simple_Imm(name) def_EHelper(name) { rtl_##name(s, ddest, dsrc1, id_src2->imm); }
def_EHelper_Simple_Imm(addi)
def_EHelper_Simple_Imm(xori)
def_EHelper_Simple_Imm(ori)
def_EHelper_Simple_Imm(andi)
def_EHelper_Simple_Imm(slli)
def_EHelper_Simple_Imm(srli)
def_EHelper_Simple_Imm(srai)

def_EHelper(slti) {
  if ((sword_t)(*dsrc1) < (sword_t)(id_src2->imm)) {
    rtl_li(s, ddest, 1);
  }
  rtl_li(s, ddest, 0);
}

def_EHelper(sltiu) {
  if (*dsrc1 < id_src2->imm) {
    rtl_li(s, ddest, 1);
  }
  rtl_li(s, ddest, 0);
}

def_EHelper(lui) {
  rtl_li(s, ddest, id_src1->imm);
}

def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm);
  rtl_add(s, ddest, &s->dnpc, ddest);
}
