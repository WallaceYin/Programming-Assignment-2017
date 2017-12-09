#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t2);
  operand_write(id_dest, &t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
    /*t0 = reg_l(4);
    for (int i = 0; i < 8; i++)
    {
      if (i != 4)
      {
        t1 = reg_l(i);
//        rtl_sext(&t1, &t1, 4);
        rtl_push(&t1);
      }
      else
      {
//        rtl_sext(&t0, &t0, 4);
        rtl_push(&t0);
      }
    }*/
  t1 = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t1);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
    /*for (int i = 7; i >= 0; i--)
    {
      if (i != 4)
      {
        rtl_pop(&t1);
        reg_l(i) = t1;
      }
      else
      {
        rtl_pop(&t1);
      }
    }*/
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t1);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp=cpu.ebp;
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    if((cpu.ax & 0x8000)>>15==1)
        cpu.dx=0xffff;
    else
        cpu.dx=0;
  }
  else {
    //TODO();
    if((cpu.eax & 0x80000000)>>31==1)
        cpu.edx=0xffffffff;
    else
        cpu.edx=0;
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  /*if (decoding.is_operand_size_16) {
    t0 = reg_b(0);
    rtl_sext(&t1, &t0, 1);
    reg_w(0) = t1;
  }
  else {
    t0 = reg_w(0);
    rtl_sext(&t1, &t0, 2);
    reg_l(0) = t1;*/
  if (decoding.is_operand_size_16) {
  if((cpu.al & 0x80)>>7==1)
      cpu.ax=0xff00+cpu.al;
  else
      cpu.ax=cpu.al;
  }
  else {
    //TODO();
    if((cpu.ax & 0x8000)>>15==1)
        cpu.eax=0xffff0000+cpu.ax;
    else
        cpu.eax=cpu.ax;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
