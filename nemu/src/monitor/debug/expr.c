#include "nemu.h"
#include <stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NOEQ, TK_NUM_DEC, TK_NUM_HEX, TK_REG, TK_DEREF,
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\ +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NOEQ}, 	//not equal
  {"-", '-'}, 		//minus
  {"\\*", TK_DEREF},		//product
  {"/", '/'},		//division
  {"0x[0-9a-fA-F]+", TK_NUM_HEX},//numbers, hex
  {"[0-9]+", TK_NUM_DEC},//numbers, decimal
  {"\\$[a-z]{2,3}", TK_REG}, //registers
  {"\\(", '('},	//parenthesis on the left
  {"\\)", ')'},	//parenthesis on the right
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];//Compiled regex

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

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* XXX: Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

/*        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
*/        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	int j;
      switch (rules[i].token_type) {
		case TK_NUM_DEC://记下十进制数值
		    tokens[nr_token].type=TK_NUM_DEC;
		    for(j=0;j<substr_len;++j)
		    {
			tokens[nr_token].str[j]=substr_start[j];
		    }
		    if(j<32)
		    {	tokens[nr_token].str[j]='\0';}
			nr_token++;
		    break;
		case TK_NUM_HEX://记下0x数值
		    tokens[nr_token].type=TK_NUM_HEX;
		    for(j=0;j<substr_len;++j)
		    {
			tokens[nr_token].str[j]=substr_start[j];
		    }
		    if(j<32)
		    {	tokens[nr_token].str[j]='\0';}
		    nr_token++;
	            break;
		case TK_REG:	//记下寄存器编号
		    tokens[nr_token].type=TK_REG;//register
		    for(j=1;j<substr_len;++j)
		    {
			tokens[nr_token].str[j-1]=substr_start[j];
		    }
		    tokens[nr_token].str[j]='\0';
		    nr_token++;
		    break;
		case TK_DEREF:	//解引用还是乘法
		    if(nr_token>0&&(tokens[nr_token-1].type==TK_NUM_HEX||tokens[nr_token-1].type==TK_NUM_DEC||tokens[nr_token-1].type==TK_REG||tokens[nr_token-1].type==')'))
		    tokens[nr_token].type='*';//乘法
		    else
			tokens[nr_token].type=TK_DEREF;//解引用
		    nr_token++;
		    break;
		case TK_NOTYPE:
		    break;
         	default:
		    tokens[nr_token].type=rules[i].token_type;
		    nr_token++; 
		    break; 
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

int start, end;

uint32_t expression_value(bool*);

uint32_t term_value(bool*);

uint32_t factor_value(bool*);

uint32_t expr(char *e, bool *success) {
    *success=true;
if (!make_token(e)) {
   *success = false;
    return 0;
  }
  start=0;
  end=nr_token-1;
  /* TODO: Insert codes to evaluate the expression. */
  *success=true;
  return expression_value(success);
}
uint32_t reg_value(char* reg,bool* flag)
{
    uint32_t ret=-1;    
    if(strcmp(reg, "eax")==0)
	ret=cpu.eax;
    else if(strcmp(reg, "ebx")==0)
	ret=cpu.ebx;
    else if(strcmp(reg, "ecx")==0)
	ret=cpu.ecx;
    else if(strcmp(reg, "edx")==0)
	ret=cpu.edx;
    else if(strcmp(reg, "esp")==0)
	ret=cpu.esp;
    else if(strcmp(reg, "ebp")==0)
	ret=cpu.ebp;
    else if(strcmp(reg, "esi")==0)
	ret=cpu.esi;
    else if(strcmp(reg, "edi")==0)
	ret=cpu.edi;
    else if(strcmp(reg, "eip")==0)
	ret=cpu.eip;
    else if(strcmp(reg, "al")==0)
	ret=cpu.al;
    else if(strcmp(reg, "ah")==0)
	ret=cpu.ah;
    else if(strcmp(reg, "bl")==0)
	ret=cpu.bl;
    else if(strcmp(reg, "bh")==0)
	ret=cpu.bh;
    else if(strcmp(reg, "cl")==0)
	ret=cpu.cl;
    else if(strcmp(reg, "ch")==0)
	ret=cpu.ch;
    else if(strcmp(reg, "dl")==0)
	ret=cpu.dl;
    else if(strcmp(reg, "dh")==0)
	ret=cpu.dh;
    else if(strcmp(reg, "ax")==0)
	ret=cpu.ax;
    else if(strcmp(reg, "bx")==0)
	ret=cpu.bx;
    else if(strcmp(reg, "cx")==0)
	ret=cpu.cx;
    else if(strcmp(reg, "dx")==0)
	ret=cpu.dx;
    else if(strcmp(reg, "si")==0)
	ret=cpu.si;
    else if(strcmp(reg, "di")==0)
	ret=cpu.di;
    else if(strcmp(reg, "sp")==0)
	ret=cpu.sp;
    else if(strcmp(reg, "bp")==0)
	ret=cpu.bp;
    else
    {
        *flag=false;
	    printf("No such register\n");
    }
    return ret;
}    
uint32_t expression_value(bool* flag)
{
    if(start>end||*flag==false)
	    return 0;
    int result=term_value(flag);
    while(1)
    {
	    int op=tokens[start].type;
	    if(op=='+'||op=='-')
	    {
	        start++;
	        int value=term_value(flag);
	        if(op=='+')
		        result+=value;
	        else
		        result-=value;
	    }
	    else if(op==TK_EQ||op==TK_NOEQ)
	    {
	        start++;
	        int value=term_value(flag);
	        if(op==TK_EQ)
		        result=(result==value);
	        else
		        result=(result!=value);
	    }//好像没有问题了？
	    else
            break;
    }
    return result;
}
uint32_t term_value(bool* flag)
{
    if(start>end||*flag==false)
	return 0;
    int result=factor_value(flag);
    while(1)
    {
	int op=tokens[start].type;
	if(op=='*'||op=='/')
	{
	    start++;
	    if(start>end)
		break;
	    int value=factor_value(flag);
	    if(op=='*')
	    {	result*=value;}
	    else
	    {	
		if(value==0)
		{   
		    printf("Divided by 0!\n");
		    *flag=false;
            return 0;
		}
		result/=value;
	    }
	}
	else
	    break;
    }
    return result;
}
uint32_t factor_value(bool* flag)
{
    if(start>end||*flag==false)
	    return 0;
    int result=0;
    int c=tokens[start].type;
    if(c=='(')
    {
	    start++;
	    result=expression_value(flag);
	    if(tokens[start].type!=')')
        {
            *flag=false;
            printf("Syntax error\n");
        }
        else
            start++;
    }
    else if(c==TK_DEREF)//暂时先规定解引用后面被括号包围吧
    {
        start++;//去掉*
        if(tokens[start].type!='(')
        {
            *flag=false;
            printf("Syntax error\n");
            return 0;
        }
        else
        {    start++;}
	    result=vaddr_read(expression_value(flag), 4);
        if(tokens[start].type!=')')
        {
            *flag=false;
            printf("Syntax error\n");
        }
        else
	        start++;//去掉)
    }	    
     else
    {
        if(c==TK_NUM_DEC)
	    {
	        result=atoi(tokens[start].str);
	        start++;
	    }
	    else if(c==TK_NUM_HEX)
	    {
	        result=(int)strtol(tokens[start].str, NULL, 0);
	        start++;
	    }
	    else if(c==TK_REG)
	    {
	        result=reg_value(tokens[start].str, flag);
	        start++;
	    }
    }
    return result;
}
