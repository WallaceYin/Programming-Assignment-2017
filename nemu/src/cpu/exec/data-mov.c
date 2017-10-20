#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t2);
  operand_write(id_dest, &t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if (decoding.is_operand_size_16)
  {
    t0 = reg_w(4);
    for (int i = 0; i < 8; i++)
    {
      if (i != 4)
      {
        t1 = reg_w(i);
        rtl_push(&t1);
      }
      else
      {
        rtl_push(&t0);
      }
    }
  }
  else
  {
    t0 = reg_l(4);
    for (int i = 0; i < 8; i++)
    {
      if (i != 4)
      {
        t1 = reg_l(i);
        rtl_push(&t1);
      }
      else
      {
        rtl_push(&t0);
      }
    }
  }
  print_asm("pusha");
}

make_EHelper(popa) {
  if (decoding.is_operand_size_16)
  {
    for (int i = 0; i < 8; i++)
    {
      if (i != 4)
      {
        rtl_pop(&t1);
        reg_w(i) = t1;
      }
      else
      {
        rtl_pop(&t1);
      }
    }
  }
  else
  {
    for (int i = 0; i < 8; i++)
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
    }
  }
  print_asm("popa");
}

make_EHelper(leave) {
  if (decoding.is_operand_size_16)
  {
    rtl_pop(&t0);
    reg_w(5) = t0;
  }
  else
  {
    rtl_pop(&t0);
    reg_l(5) = t0;
  }
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    t1 = reg_w(0);
    rtl_msb(&t0, &t1, 2);
    if (t0)
      reg_w(2) = 0xffff;
    else
      reg_w(2) = 0;
  }
  else {
    t1 = reg_l(0);
    rtl_msb(&t0, &t1, 4);
    if (t0)
      reg_l(2) = 0xffff;
    else
      reg_l(2) = 0;
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    t0 = reg_b(0);
    rtl_sext(&t1, &t0, 1);
    reg_w(0) = t1;
  }
  else {
    t0 = reg_w(0);
    rtl_sext(&t1, &t0, 2);
    reg_l(0) = t1;
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
