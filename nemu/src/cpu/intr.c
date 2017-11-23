#include "cpu/exec.h"
#include "memory/mmu.h"

void extern raise_intr(uint8_t NO, vaddr_t ret_addr) {
   t0 = cpu.eflags_init;
   Log("raise_intr");
   rtl_push(&t0);
   t0 = cpu.cs;
   rtl_push(&t0);
   t0 = ret_addr;
   rtl_push(&t0);
   uint32_t offset_L, offset_H, offset;
   offset_L = vaddr_read(cpu.IDTR.base + 8*NO, 2);
   offset_H = vaddr_read(cpu.IDTR.base + 8*NO + 6, 2);
   offset = (offset_H << 16) + offset_L;
   decoding.is_jmp = 1;
   decoding.jmp_eip = offset;
}
void dev_raise_intr() {
}
