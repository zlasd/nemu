#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"
#include "watchpoint.h"

static int is_batch_mode = false;
static int is_expr_mode = false;

void init_regex();

extern int eval_errno;

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  if (args == NULL) {
    printf("invalid command\n");
    return 0;
  }
  int step;
  sscanf(args, "%d", &step);
  cpu_exec(step);
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("invalid command\n");
    return 0;
  }
  if (strcmp(args, "r") == 0) {
    isa_reg_display();
    return 0;
  }
  if (strcmp(args, "w") == 0) {
    display_watchpoint();
  }
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("invalid command\n");
    return 0;
  }
  int i;
  paddr_t addr;
  char* tok = strtok(args, " ");
  sscanf(tok, "%d", &i);
  tok = strtok(NULL, " ");
  if (tok == NULL) {
    printf("invalid command\n");
    return 0;
  }
  sscanf(tok, FMT_PADDR, &addr);

  for (paddr_t c = addr; i > 0; c+=sizeof(word_t), i--) {
    printf(FMT_PADDR"\t"FMT_WORD"\n", c, paddr_read(c, sizeof(word_t)));
  }
  return 0;
}

static int cmd_p(char *args) {
  bool succ;
  word_t ret = expr(args, &succ);
  if (!succ) {
    printf("bad expression! %s\n", args);
    return 0;
  }
  printf(FMT_WORD"\n", ret);
  return 0;
}

static int cmd_w(char *args) {
  bool succ;
  word_t ret = expr(args, &succ);
  if (!succ) {
    printf("bad expression! %s\n", args);
    return 0;
  }

  WP *w = new_wp();
  w->expr = (char*)malloc(strlen(args)+1);
  strcpy(w->expr, args);
  w->old_val = ret;
  w->hits = 0;
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("invalid command\n");
    return 0;
  }
  int no;
  sscanf(args, "%d", &no);
  free_wp_by_no(no);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute single step of the program", cmd_si},
  { "info", "Print program state", cmd_info},
  { "x", "Scan the memory and print the value", cmd_x},
  { "p", "Evaluate expression's value", cmd_p},
  { "w", "Set watchpoints on expression", cmd_w},
  { "d", "Delete watchpoints", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_set_expr_mode() {
  is_expr_mode = true;
}

void expr_mode() {
  uint32_t ans, ret;
  char buf[1024];
  bool succ;

  while (EOF != scanf("%d %s", &ans, buf)) {
    ret = expr(buf, &succ);
    printf("%s=%d, expected=%d\n", buf, ret, ans);
    if (!succ) {
      printf("errno=%d\n", eval_errno);
    }
    assert(ans == ret);
  }
  nemu_state.state = NEMU_QUIT;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  if (is_expr_mode) {
    expr_mode();
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
