#include "cpu/exec.h"

make_EHelper(test) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  cpu.eflags.CF=0;
  cpu.eflags.OF=0;
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  cpu.eflags.CF=0;
  cpu.eflags.OF=0;
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  cpu.eflags.CF=0;
  cpu.eflags.OF=0;
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  cpu.eflags.CF=0;
  cpu.eflags.OF=0;
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  t2=id_dest->val;
  rtl_not(&t2);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template1(not);
}

make_EHelper(rcl)
{
    Log("rcl!!");
    print_asm_template2(rcl);
}

make_EHelper(rcr)
{
    Log("rcr");
    print_asm_template2(rcr);
}

make_EHelper(rol)
{
    Log("rol");
    print_asm_template2(rol);
}

make_EHelper(ror)
{
    Log("ror");
    print_asm_template2(ror);
}

