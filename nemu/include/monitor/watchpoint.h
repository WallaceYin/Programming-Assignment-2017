#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;//监视点的编号
  struct watchpoint *next;
  char str[32];//记录下这个表达式
  uint32_t old_value;//过去的值,在加入池的时候初始化

  /* TODO: Add more members if necessary */

} WP;
WP* new_wp();
void free_wp(WP *wp);
void init_wp_pool();
WP *wp_head();
#endif
