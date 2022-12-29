#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include "memory/vaddr.h"

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_PLUS, TK_MINUS, TK_MUL, TK_DIV,
  TK_LEFT_PAR, TK_RIGHT_PAR,
  TK_DEC, TK_HEX, TK_REG,
  TK_NEQ, TK_AND,
  TK_NEG, TK_DEREF,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},     // plus
  {"-", TK_MINUS},      // minus
  {"\\*", TK_MUL},      // multiply
  {"/", TK_DIV},        // dividend
  {"\\(", TK_LEFT_PAR},  // left parenthese
  {"\\)", TK_RIGHT_PAR}, // right parenthese
  {"0x[0-9a-fA-F]{1,10}", TK_HEX}, // hexical integer
  {"[0-9]{1,10}", TK_DEC}, // decimal integer
  {"\\$[\\$atsrg]{1,2}[0-9]{0,2}", TK_REG},     // register
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // bool and
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[128] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;
int eval_errno = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          default:
            if (substr_len >= 32) {
              printf("token length beyond 32, which is invalid");
              return false;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            tokens[nr_token++].type = rules[i].token_type;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


int find_pivot(int p, int q) {
  int cnt = 0;
  int pos = -1;
  for (int i = q; i >= p; i--) {
    if (tokens[i].type == TK_RIGHT_PAR) {
      cnt++;
    }
    if (tokens[i].type == TK_LEFT_PAR) {
      cnt--;
    }
    if (cnt != 0) {
      continue;
    }
    if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS) {
      return i;
    }
    if (pos == -1 && (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV)) {
      pos = i;
    }
  }
  return pos;
}

bool check_parenthese(int p, int q) {
  int cnt = 0;
  if (tokens[p].type != TK_LEFT_PAR || tokens[q].type != TK_RIGHT_PAR) {
    eval_errno = 0;
    return false;
  }
  for (int i = p+1; i <= q-1; i++) {
    if (tokens[i].type == TK_LEFT_PAR) {
      cnt++;
    }
    if (tokens[i].type == TK_RIGHT_PAR) {
      cnt--;
    }
    if (cnt < 0) {
      return false;
    }
  }
  if (cnt != 0) {
    eval_errno = -4;  // parenthese not match
    return false;
  }
  eval_errno = 0;
  return true;
}

word_t eval_single_token(int n) {
  word_t ret;
  bool succ;
  switch (tokens[n].type)
  {
  case TK_DEC:
    sscanf(tokens[n].str, "%d", &ret);
    return ret;
  case TK_HEX:
    sscanf(tokens[n].str, "0x%x", &ret);
    return ret;
  case TK_REG:
    ret = isa_reg_str2val(tokens[n].str+1, &succ);  // eat the prefix '$'
    if (!succ) {
      eval_errno = -5;  // read register error
      return -1;
    }
    return ret;
  default:
    eval_errno = -6;  // eval single value failed
    return -1;
  }
}

word_t eval(int p, int q) {
  word_t ret;
  if (p > q) {
    eval_errno = -1;  // bad expression
    return -1;
  }
  if (p == q) {
    sscanf(tokens[p].str, tokens[p].type == TK_DEC?"%d":"0x%x", &ret);
    return ret;
  }
  if (check_parenthese(p, q) == true) {
    return eval(p+1, q-1);
  }
  if (eval_errno != 0) {
    return -1;
  }

  switch (tokens[p].type)
  {
  case TK_NEG:
    return -eval(p+1, q);
  case TK_DEREF:
    ret = eval(p+1, q);
    return vaddr_read(ret, 4);
  }

  int pivot = find_pivot(p, q);
  if (pivot == -1) {
    eval_errno = -3;  // cannot find pivot op
    return -1;
  }
  Log("pivot token[%d] = \"%s\"", pivot, tokens[pivot].str);
  word_t val1 = eval(p, pivot-1);
  word_t val2 = eval(pivot+1, q);
  Log("val1 = %d, val2 = %d", val1, val2);

  switch (tokens[pivot].type)
  {
  case TK_PLUS: return val1 + val2;
  case TK_MINUS: return val1 - val2;
  case TK_MUL: return val1 * val2;
  case TK_DIV: return val1 / val2;
  case TK_EQ: return val1 == val2;
  case TK_NEQ: return val1 != val2;
  case TK_AND: return val1 && val2;
  default:
    eval_errno = -2;  // bad expression
    break;
  }
  return -1;
}

bool check_unary_op(int i) {
  return i == 0 ||
    tokens[i - 1].type == TK_PLUS ||
    tokens[i - 1].type == TK_MINUS ||
    tokens[i - 1].type == TK_MUL ||
    tokens[i - 1].type == TK_DIV ||
    tokens[i - 1].type == TK_NEG ||
    tokens[i - 1].type == TK_DEREF ||
    tokens[i - 1].type == TK_EQ ||
    tokens[i - 1].type == TK_NEQ ||
    tokens[i - 1].type == TK_AND;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_MUL && check_unary_op(i) ) {
      tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == TK_MINUS && check_unary_op(i) ) {
      tokens[i].type = TK_NEG;
    }
  }

  /* TODO: Insert codes to evaluate the expression. */
  word_t ret = eval(0, nr_token-1);
  if (eval_errno != 0) {
    *success = false;
    return 0;
  }
  *success = true;
  return ret;
}
