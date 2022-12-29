#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expr;
  word_t old_val;
  int type;
  int disp;
  int enable;
  int hits;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (free_ == NULL) {
    assert(0);
  }
  WP* cur = free_;
  free_ = free_->next;
  cur->next = head;
  head = cur;
  return cur;
}

void free_wp(WP *wp) {
  if (wp == head) {
    head = wp->next;
    wp->next = free_;
    free_ = wp;
    return;
  }

  WP *prev = head;
  while (prev != NULL) {
    if (prev->next == wp) {
      break;
    }
    prev = prev->next;
  }
  if (prev->next != wp) {
    assert(0);
  }
  prev->next = wp->next;
  wp->next = free_;
  free_ = wp;
}

