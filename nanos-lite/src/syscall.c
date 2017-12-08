#include "common.h"
#include "syscall.h"
#include "fs.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
	//printf("a0 = %x a1= %x a2= %x a3=%x\n", a[0],a[1],a[2],a[3]);
	//char *buf;

  switch (a[0]) {
    case SYS_none:  a[0] = 1; break;

    case SYS_exit:  a[0] = 1; _halt(a[2]); break;

    case SYS_write:
      a[0] = (uintptr_t)fs_write((int)a[1], (void *)a[2], (size_t)a[3]);
			  break;

    case SYS_brk:
      a[0] = 0;
    break;

    case SYS_read:
      a[0] = (uintptr_t)fs_read((int)a[1], (void *)a[2], (size_t)a[3]);
      break;

    case SYS_open:
      a[0] = (uintptr_t)fs_open((char *)a[1], (int)a[2], (int)a[3]);
      //printf("fd = %d\n",a[0]);
      break;

    case SYS_close:
      a[0] = (uintptr_t)fs_close((int)a[1]);
      break;

		case SYS_lseek:
			//printf("a0 = %x a1 = %x a2 = %x a3 = %x\n",a[0], a[1], a[2], a[3]);
			a[0] = (uintptr_t)fs_lseek((int)a[1], (off_t)a[2], (int)a[3]);
      //printf("fd = %d\n", a[1]);
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  SYSCALL_ARG1(r) = a[0];
  SYSCALL_ARG2(r) = a[1];
  SYSCALL_ARG3(r) = a[2];
  SYSCALL_ARG4(r) = a[3];
  return NULL;
}
