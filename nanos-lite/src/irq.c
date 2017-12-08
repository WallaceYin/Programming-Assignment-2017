#include "common.h"
//#include "syscall.h"

_RegSet* do_syscall(_RegSet *);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case 0x8: do_syscall(r); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
