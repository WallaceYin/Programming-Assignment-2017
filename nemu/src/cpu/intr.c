#include "cpu/exec.h"
#include <setjmp.h>
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  //printf("0x%x\n", ret_addr);
  //printf("0x%x 0x%x\n", cpu.idtr.base, cpu.idtr.limit);
  rtl_push(&cpu.EFLAGS);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);
  //GateDesc GD;
  uint32_t tmp[2];
  tmp[0]=vaddr_read(cpu.idtr.base+NO*8, 2);
  tmp[1]=vaddr_read(cpu.idtr.base+NO*8+6, 2);
  //memcpy(&GD, tmp, 8);
  //decoding.jmp_eip = (GD.offset_15_0)|(GD.offset_31_16<<16);
  decoding.jmp_eip = tmp[0] + (tmp[1] << 16);
  decoding.is_jmp = true;
}

void dev_raise_intr() {
}
