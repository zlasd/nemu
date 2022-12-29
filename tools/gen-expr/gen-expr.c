#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static uint32_t buf_size = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

uint32_t choose(uint32_t n) {
  return rand() % n;
}

static void gen(char c) {
    buf[buf_size++] = c;
}

static void gen_num() {
  uint32_t num = choose(1000);
  buf_size += sprintf(buf+buf_size, "%d", num);
}

static void gen_rand_op() {
  switch (choose(4))
  {
  case 0: gen('+'); break;
  case 1: gen('-'); break;
  case 2: gen('*'); break;
  case 3: gen('/'); break;
  }
}

static void gen_rand_expr() {
  switch (choose(4)) {
    case 0: case 1: gen_num(); break;
    case 2: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
  buf[buf_size] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_size = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    (void)!fscanf(fp, "%d", &result);  // use (void)! to suppress warning
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
