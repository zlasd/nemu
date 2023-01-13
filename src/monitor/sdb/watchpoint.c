#include "sdb.h"
#include "watchpoint.h"

#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *free_ = NULL;
WP *head = NULL;

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
  free(wp->expr);
}

void free_wp_by_no(int no) {
  free_wp(&wp_pool[no]);
}

void display_watchpoint() {
  printf("NO\texpr                \told_value\thits\n");
  WP *cur = head;
  while (cur != NULL) {
    printf("%d\t%-20s\t"FMT_WORD"\t%d\n", cur->NO, cur->expr, cur->old_val, cur->hits);
    cur = cur->next;
  }
  printf("\n");
}
