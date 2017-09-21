#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  uint32_t Val;
  char exp[32];
  struct watchpoint *next;
  /* TODO: Add more members if necessary */


} WP;

WP* new_up();

void free_wp(WP *wp);

WP* print_wp(WP *wp);

#endif
