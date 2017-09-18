#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ = 2,
  TK_PLUS = 43, TK_MINUS = 45,
  TK_TIMES = 42, TK_DIV = 47,
  TK_lpar = 28, TK_rpar = 29,
  TK_HEX = 127, TK_DEX = 129,
  TK_REG = 1
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\)", TK_rpar},          // right parenthesis
  {"\\(", TK_lpar},          // left parenthesis
  {"/", TK_DIV},         // divide
  {"//*", TK_TIMES},         // times
  {"-", TK_MINUS},         // minus
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},         // equal
  {"[0-9]+",TK_DEX},    //dexnumber
  {"0x[0-9abcde]+",TK_HEX},    //hexnumber
  {"\\$e[a-z]+",TK_REG}	//register
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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

Token tokens[32];
int nr_token;

bool higherLevel(int TK_type_1, int TK_type_2)
{
	if (TK_type_2 == TK_lpar || TK_type_2 == TK_rpar)
		return 0;
	if (TK_type_1 == TK_TIMES || TK_type_1 == TK_DIV)
	{
		if (TK_type_2 == TK_PLUS || TK_type_2 == TK_MINUS)
			return 1;
	}
	return 0;
}

uint32_t regToVal(int p)
{
	if (strcmp(tokens[p].str,"$eax") == 0)
		return cpu.eax;
	if (strcmp(tokens[p].str,"$ecx") == 0)
		return cpu.ecx;
	if (strcmp(tokens[p].str,"$ebx") == 0)
		return cpu.ebx;
	if (strcmp(tokens[p].str,"$edx") == 0)
		return cpu.edx;
	if (strcmp(tokens[p].str,"$esp") == 0)
		return cpu.esp;
	if (strcmp(tokens[p].str,"$ebp") == 0)
		return cpu.ebp;
	if (strcmp(tokens[p].str,"$esi") == 0)
		return cpu.esi;
	if (strcmp(tokens[p].str,"$edi") == 0)
		return cpu.edi;
	return 0;
}

uint32_t hexToVal(int p)
{
	int i;
	uint32_t n;
	n = 0;
	for (i = 2; i < strlen(tokens[p].str); i++)
	{
		if (tokens[p].str[i] > '9')
			n = n * 16 + (int)tokens[p].str[i] - 51;
		else n = n * 10 + (int)tokens[p].str[i] -48;
	}
	return n;
}

uint32_t dexToVal(int p)
{
	printf("Enter dextoval with p =%d",p);
	int i; 
	uint32_t n;
	n = 0;
	for (i = 0; i < strlen(tokens[p].str); i++)
		n = n * 10 + (int)tokens[p].str[i] - 48;
	printf("n = %d",n);
	return n;
}

uint32_t eval(int p,int q)
{
	printf("Enter eval with p = %d and q = %d\n",p,q);
	if (p == q)
	{
		if (tokens[p].type == TK_REG)
			return regToVal(p);
		else if (tokens[p].type == TK_HEX)
			return hexToVal(p);
		else return dexToVal(p);
	}
	else if (tokens[p].type == TK_lpar && tokens[q].type == TK_rpar)
	{
		return eval(p+1,q-1);
	}
	int i,domain,Inpair,domainTri;
	domain = 0;
	Inpair = 0;
	domainTri = 0;
	for (i = p; i <= q; i++)
	{
		if (Inpair == 0 && higherLevel(domain,tokens[i].type))
		{
			domain = tokens[i].type;
			domainTri = i;
		}
		if (tokens[i].type == TK_lpar)
			Inpair++;
		if (tokens[i].type == TK_rpar)
			Inpair--;
	}
	if (domain == TK_PLUS)
		return eval(p,domainTri - 1) + eval(domainTri + 1,q);
	if (domain == TK_MINUS)
		return eval(p,domainTri - 1) - eval(domainTri + 1,q);
	if (domain == TK_TIMES)
		return eval(p,domainTri - 1) * eval(domainTri + 1,q);
	if (domain == TK_DIV)
		return eval(p,domainTri - 1) / eval(domainTri + 1,q);
	return 0;
}

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
	if (rules[i].token_type == 256)
		continue;
        tokens[nr_token].type = rules[i].token_type;
	nr_token ++;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
		case TK_DEX:	strcpy(tokens[nr_token].str, substr_start);
		case TK_HEX:	strcpy(tokens[nr_token].str, substr_start);
		case TK_REG:	strcpy(tokens[nr_token].str, substr_start);
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  uint32_t ans = eval(0, nr_token-1);
    *success = true;
  /* TODO: Insert codes to evaluate the expression. */
  return ans;
}
