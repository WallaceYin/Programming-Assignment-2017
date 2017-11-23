#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

void raise_intr(uint8_t, vaddr_t);

make_EHelper(lidt) {
  if (id_dest->width == 2)
  {
    rtl_lm(&t0, &id_dest->addr, 2);
    cpu.IDTR.limit = t0;
    id_dest->addr = id_dest->addr + 2;
    rtl_lm(&t0, &id_dest->addr, 3);
    cpu.IDTR.base = t0;
  }
  else
  {
    rtl_lm(&t0, &id_dest->addr, 2);
    cpu.IDTR.limit = t0;
    id_dest->addr = id_dest->addr + 2;
    rtl_lm(&t0, &id_dest->addr, 4);
    cpu.IDTR.base = t0;
  }
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  rtl_lm(&t0, &id_dest->val, 1);
  uint8_t NO;
  NO = t0 & 0xff;
  NO = 3;
  raise_intr(NO, cpu.eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t1 = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &t1);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
