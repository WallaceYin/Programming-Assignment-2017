#include "cpu/exec.h"
void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  //TODO();
  /*rtl_lm(&t0, &id_dest->addr, 2);
  cpu.idtr.limit=t0;
  t1=id_dest->addr+2;
  rtl_lm(&t0, &t1, 4);
  if(decoding.is_operand_size_16)
    t0=t0&0x00ffffff;*/
  cpu.IDTR.base=id_dest->val;
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  //TODO();
  if (id_dest->reg == 0)
    cpu.cr0_init = id_src->val;
  else if (id_dest->reg == 3)
    cpu.cr3_init = id_src->val;
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  //TODO();
  if (id_src->reg == 0)
    operand_write(id_dest,&cpu.cr0_init);
  else if (id_src->reg == 3)
    operand_write(id_dest,&cpu.cr3_init);
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
extern void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(int) {
  //TODO();

  raise_intr(id_dest->val, *eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //TODO();
  rtl_pop(&t0);
  decoding.jmp_eip=t0;
  //printf("%d\n", t0);
  rtl_pop(&t0);
  cpu.cs=(uint16_t)t0;
  rtl_pop(&cpu.eflags_init);
  decoding.is_jmp=1;

  print_asm("iret");
}

extern uint32_t pio_read(ioaddr_t, int);
extern void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //TODO();
  t1=pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &t1);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();
  pio_write(id_dest->val, id_src->width, id_src->val);
  //operand_write(id_dest, &id_dest->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
