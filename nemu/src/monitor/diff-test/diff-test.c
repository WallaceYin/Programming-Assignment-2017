#include "nemu.h"
#include "monitor/monitor.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>

#include "protocol.h"
#include <stdlib.h>

bool gdb_connect_qemu(void);
bool gdb_memcpy_to_qemu(uint32_t, void *, int);
bool gdb_getregs(union gdb_regs *);
bool gdb_setregs(union gdb_regs *);
bool gdb_si(void);
void gdb_exit(void);

static bool is_skip_qemu;
static bool is_skip_nemu;

void diff_test_skip_qemu() { is_skip_qemu = true; }
void diff_test_skip_nemu() { is_skip_nemu = true; }

#define regcpy_from_nemu(regs) \
  do { \
    regs.eax = cpu.eax; \
    regs.ecx = cpu.ecx; \
    regs.edx = cpu.edx; \
    regs.ebx = cpu.ebx; \
    regs.esp = cpu.esp; \
    regs.ebp = cpu.ebp; \
    regs.esi = cpu.esi; \
    regs.edi = cpu.edi; \
    regs.eip = cpu.eip; \
  } while (0)

static uint8_t mbr[] = {
  // start16:
  0xfa,                           // cli
  0x31, 0xc0,                     // xorw   %ax,%ax
  0x8e, 0xd8,                     // movw   %ax,%ds
  0x8e, 0xc0,                     // movw   %ax,%es
  0x8e, 0xd0,                     // movw   %ax,%ss
  0x0f, 0x01, 0x16, 0x44, 0x7c,   // lgdt   gdtdesc
  0x0f, 0x20, 0xc0,               // movl   %cr0,%eax
  0x66, 0x83, 0xc8, 0x01,         // orl    $CR0_PE,%eax
  0x0f, 0x22, 0xc0,               // movl   %eax,%cr0
  0xea, 0x1d, 0x7c, 0x08, 0x00,   // ljmp   $GDT_ENTRY(1),$start32

  // start32:
  0x66, 0xb8, 0x10, 0x00,         // movw   $0x10,%ax
  0x8e, 0xd8,                     // movw   %ax, %ds
  0x8e, 0xc0,                     // movw   %ax, %es
  0x8e, 0xd0,                     // movw   %ax, %ss
  0xeb, 0xfe,                     // jmp    7c27
  0x8d, 0x76, 0x00,               // lea    0x0(%esi),%esi

  // GDT
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00,

  // GDT descriptor
  0x17, 0x00, 0x2c, 0x7c, 0x00, 0x00
};

void init_difftest(void) {
  int ppid_before_fork = getpid();
  int pid = fork();
  if (pid == -1) {
    perror("fork");
    panic("fork error");
  }
  else if (pid == 0) {
    // child

    // install a parent death signal in the chlid
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      perror("prctl error");
      panic("prctl");
    }

    if (getppid() != ppid_before_fork) {
      panic("parent has died!");
    }

    close(STDIN_FILENO);
    execlp("qemu-system-i386", "qemu-system-i386", "-S", "-s", "-nographic", NULL);
    perror("exec");
    panic("exec error");
  }
  else {
    // father

    gdb_connect_qemu();
    Log("Connect to QEMU successfully");

    atexit(gdb_exit);

    // put the MBR code to QEMU to enable protected mode
    bool ok = gdb_memcpy_to_qemu(0x7c00, mbr, sizeof(mbr));
    assert(ok == 1);

    union gdb_regs r;
    gdb_getregs(&r);

    // set cs:eip to 0000:7c00
    r.eip = 0x7c00;
    r.cs = 0x0000;
    ok = gdb_setregs(&r);
    assert(ok == 1);

    // execute enough instructions to enter protected mode
    int i;
    for (i = 0; i < 20; i ++) {
      gdb_si();
    }
  }
}

void init_qemu_reg() {
  union gdb_regs r;
  gdb_getregs(&r);
  regcpy_from_nemu(r);
  bool ok = gdb_setregs(&r);
  assert(ok == 1);
}

void difftest_step(uint32_t eip) {
  union gdb_regs r;
  bool diff = false;

  if (is_skip_nemu) {
    is_skip_nemu = false;
    return;
  }

  if (is_skip_qemu) {
    // to skip the checking of an instruction, just copy the reg state to qemu
    gdb_getregs(&r);
    regcpy_from_nemu(r);
    gdb_setregs(&r);
    is_skip_qemu = false;
    return;
  }

  gdb_si();
  gdb_getregs(&r);

  // TODO: Check the registers state with QEMU.
  // Set `diff` as `true` if they are not the same.
  //TODO();
  union gdb_regs r_in_nemu;
  regcpy_from_nemu(r_in_nemu);
  if(r_in_nemu.eax!=r.eax)
  {
      diff=true;
      printf("eax not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.eax, r_in_nemu.eax);
  }
  if(r_in_nemu.ebx!=r.ebx)
  {
      diff=true;
      printf("ebx not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.ebx, r_in_nemu.ebx);
  }
  if(r_in_nemu.ecx!=r.ecx)
  {
      diff=true;
      printf("ecx not equal!\n");      
      printf("Right: 0x%x, Wrong: 0x%x\n", r.ecx, r_in_nemu.ecx);
  }
  if(r_in_nemu.edx!=r.edx)
  {
      diff=true;
      printf("edx not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.edx, r_in_nemu.edx);
  }
  if(r_in_nemu.ebp!=r.ebp)
  {
      diff=true;
      printf("ebp not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.ebp, r_in_nemu.ebp);
  }
  if(r_in_nemu.esi!=r.esi)
  {
      diff=true;
      printf("esi not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.esi, r_in_nemu.esi);
  }
  if(r_in_nemu.esp!=r.esp)
  {
      diff=true;
      printf("esp not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.esp, r_in_nemu.esp);
  }
  if(r_in_nemu.eip!=r.eip)
  {
      diff=true;
      printf("eip not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", r.eip, r_in_nemu.eip);
  }
/*  struct{
      uint32_t flag;
      uint8_t CF:1;
      uint8_t ZF:1;
      uint8_t SF:1;
      uint8_t IF:1;
      uint8_t OF:1;
  }tmp;
  tmp.CF=r.eflags & 1;
  tmp.ZF=r.eflags & 64;
  tmp.SF=r.eflags & 128;
  tmp.IF=r.eflags & 512;
  tmp.OF=r.eflags & 2048;
  if(cpu.eflags.CF!=tmp.CF)
  {
      diff=true;
      printf("eflags.CF not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", tmp.CF, cpu.eflags.CF);
  }
  if(cpu.eflags.ZF!=tmp.ZF)
  {
      diff=true;
      printf("eflags.ZF not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", tmp.ZF, cpu.eflags.ZF);
  }
  if(cpu.eflags.SF!=tmp.SF)
  {
      diff=true;
      printf("eflags.SF not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", tmp.SF, cpu.eflags.SF);
  }
  if(cpu.eflags.IF!=tmp.IF)
  {
      diff=true;
      printf("eflags.IF not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", tmp.IF, cpu.eflags.IF);
  }
  if(cpu.eflags.OF!=tmp.OF)
  {
      diff=true;
      printf("eflags.OF not equal!\n");
      printf("Right: 0x%x, Wrong: 0x%x\n", tmp.OF, cpu.eflags.OF);
  }
*/
  if (diff) {
    printf("eip=0x%x", r_in_nemu.eip);
    nemu_state = NEMU_END;
  }
}
