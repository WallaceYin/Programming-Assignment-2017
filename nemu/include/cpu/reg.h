#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */
 #define reg1(a,b,c,d) union{rtlreg_t a;uint16_t b;struct{uint8_t c;uint8_t d;};};
 #define reg2(a,b) union{rtlreg_t a;uint16_t b;};

typedef struct {
  union {
    union {
    uint32_t _32;
    uint16_t _16;
    uint8_t _8[2];
    } gpr[8];

  /* Do NOT change the order of the GPRs' definitions. */

  /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
   * in PA2 able to directly access these registers.
   */
  struct {
    reg1(eax,ax,al,ah);
  	reg1(ecx,cx,cl,ch);
  	reg1(edx,dx,dl,dh);
  	reg1(ebx,bx,bl,bh);
  	reg2(esp,sp);
  	reg2(ebp,bp);
  	reg2(esi,si);
  	reg2(edi,di);
  //  rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  };
  };
  vaddr_t eip;
  union {
    struct {
      /*uint32_t :8;
      uint32_t :8;
      uint32_t VM:1;
      uint32_t RF:1;
      uint32_t :1;
      uint32_t NT:1;
      uint32_t IOPL:2;
      uint32_t OF:1;
      uint32_t DF:1;
      uint32_t IF:1;
      uint32_t TF:1;
      uint32_t SF:1;
      uint32_t ZF:1;
      uint32_t :1;
      uint32_t AF:1;
      uint32_t :1;
      uint32_t PF:1;
      uint32_t :1;
      uint32_t CF:1;*/
      uint32_t CF:1;
      uint32_t :1;
      uint32_t PF:1;
      uint32_t :1;
      uint32_t AF:1;
      uint32_t :1;
      uint32_t ZF:1;
      uint32_t SF:1;
      uint32_t TF:1;
      uint32_t IF:1;
      uint32_t DF:1;
      uint32_t OF:1;
      uint32_t :20;
    } eflags;
    rtlreg_t eflags_init;
  };
  uint32_t cs;
  struct {
    uint16_t limit;
    uint32_t base;
  } IDTR;
  union {
    struct {
      uint32_t PE:1;
      uint32_t :30;
      uint32_t PG:1;
    } cr0;
    uint32_t cr0_init;
  };
  union {
    struct {
      uint32_t addr:20;
      uint32_t :12;
    } cr3;
    uint32_t cr3_init;
  };
} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif
