#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);//Single step execution

static int cmd_info(char *args);//Print registers

static int cmd_x(char *args);//Print memory

static int cmd_p(char *args);//Evaluation

static int cmd_w(char *args);//Set watchpoints

static int cmd_d(char *args);//Delete watchpoints

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Enter single step execution model, execute N steps before pause. N is 1 by default", cmd_si},
  { "info", "Output registers' states if followed by 'r', or watchpoints if followed by 'w'", cmd_info},
  { "x", "Output the given number of bytes starting from the given address", cmd_x},
  { "p", "Output the value of the expression followed, note that if the expression is not legal, the program may print an error message or terminates unexpectedly", cmd_p},
  { "w", "Set a watchpoint( or breakpoint), if the value of the expression given is changed, the program will pause", cmd_w},
  { "d", "Delete the watchpoint you input", cmd_d },

  /* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_d(char *args)
{
    if(args==NULL)
    {
        printf("Please input a complete command\n");
        return 0;
    }
 
    int num=atoi(args);
    WP *tmp=wp_head();
    int found=0;//有没有找到这个断点
    if(!tmp)
    {
        printf("There is no watchpoints currently\n");
        return 0;
    }
    else
    {
        while(tmp)
        {
            if(tmp->NO==num)
            {
                found=1;
                free_wp(tmp);
            }
            tmp=tmp->next;
        }
    }
    if(!found)
        printf("No watchpoint No.%d, check your input by typing info w\n", num);
    return 0;
}

static int cmd_w(char *args)
{
    if(args==NULL)
    {
	printf("Please input a complete command\n");
	return 0;
    }
    WP *wpt;//新加入的监视点指针
    wpt=new_wp();//监视点
    strcpy(wpt->str, args);//将表达式复制进监视点中
    bool success=true;//求表达式现在的值
    wpt->old_value=expr(wpt->str, &success);
    if(!success)//若求值失败，直接退出
    {	
        printf("The expression given is not valid\n");
        free_wp(wpt);
        return 0;
    } 
    return 0;
}

static int cmd_p(char *args)
{
    uint32_t ret;
    bool success=true;
    if(args==NULL)
    {
	    printf("Please input a complete command\n");
	    return 0;
    }
    ret=expr(args, &success);
    if(success)
	    printf("%d\t%uU\n0x%08x\n", ret, ret, ret);
    else
	    printf("The expression given is not valid\n");
    return 0;
}

static int cmd_x(char *args)
{
    int arg_len=strlen(args);
    char expr_arg[arg_len+1];
    strcpy(expr_arg,args);
    int cnt=0;
    while(isdigit(expr_arg[cnt]))
    {
        expr_arg[cnt]=' ';
        cnt++;
    }
    char *arg = strtok(args, " ");
    bool success=true;
    if(arg==NULL)
    {
	printf("Please input a complete command\n");
	return 0;
    }
    int len = atoi(arg);//len of memory
    arg = strtok(NULL, " ");
    if(arg==NULL)
    {
	printf("Please input a complete command\n");
	return 0;
    }
    int vadd = expr(expr_arg, &success);//address of memory
    if(!success){
	printf("The expression is not valid\n");
    	return 0;
    }
    unsigned byte;//present byte
    unsigned vadd_end = vadd + len*4;//output's end, not printed
    unsigned i;
    
    for(i = vadd; i<vadd_end; i+=4)
    {
        printf("0x%x:  ",i);
        int j;
        for(j=i;(j<i+4)&&(j<vadd_end);++j)
        {
       	   byte = vaddr_read(j, 1);
	       printf(" %02x", byte);
        }
 	    printf("\n");
    }

    return 0;

}
static int cmd_info(char *args)
{
    char *arg = strtok(args, " ");
    char *correspond[]={"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    if(arg==NULL)
    {
	    printf("Please input a complete command\n");
	    return 0;
    }
    if(strcmp(arg, "r")==0)
    {
	    int i;
        for(i=0;i<8;++i)
	    {
	        printf("%s:\t0x%x\t%uU\n", correspond[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
	    }
	    printf("eip:\t0x%x\t%uU\n", cpu.eip, cpu.eip);
        printf("eflags:\tCF: %d, OF: %d, ZF: %d, SF: %d, IF: %d\n", cpu.eflags.CF, cpu.eflags.OF, cpu.eflags.ZF, cpu.eflags.SF, cpu.eflags.IF);
        //printf("idtr.base: 0x%x, idtr.limit: 0x%x\n", cpu.idtr.base, cpu.idtr.limit);
    }
    else if(strcmp(arg, "w")==0)
    {
        WP* tmp=wp_head();
        if(!tmp)
        {
            printf("There is no watchpoints currently\n");
        }
        else
        {
            while(tmp)
            {
                printf("Watchpoint No.%d: %s\tCurrent value: 0x%x\t%d\n", tmp->NO, tmp->str, tmp->old_value, tmp->old_value);
                tmp=tmp->next;
            }

        }
    }
    return 0;
}

static int cmd_si(char *args)
{
    char *arg = strtok(args, " ");
    if(arg == NULL)
    {
	cpu_exec(1);
    	return 0;
    }
    else
    {
	int steps = atoi(arg);
	cpu_exec(steps);	
	return 0;
    }
}
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

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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
