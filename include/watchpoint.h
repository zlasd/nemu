#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expr;
  word_t old_val;
  int hits;
} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);

#endif
