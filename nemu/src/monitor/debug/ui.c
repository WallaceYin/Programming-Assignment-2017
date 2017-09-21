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

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) 
    cpu_exec(1);
  else {
    int i,n;
    n = 0;
    for (i = 0; i < strlen(arg); i++) 
      n = n*10 + (int)arg[i] - 48;
    cpu_exec(n);
  }
  return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL," ");
	if (strcmp(arg, "r") == 0) {
		printf("eax\t0x%8x\t%u\n",cpu.eax,cpu.eax);
		printf("ecx\t0x%8x\t%u\n",cpu.ecx,cpu.ecx);
		printf("edx\t0x%8x\t%u\n",cpu.edx,cpu.edx);
		printf("ebx\t0x%8x\t%u\n",cpu.ebx,cpu.ebx);
		printf("esp\t0x%8x\t%u\n",cpu.esp,cpu.esp);
		printf("ebp\t0x%8x\t%u\n",cpu.ebp,cpu.ebp);
		printf("esi\t0x%8x\t%u\n",cpu.esi,cpu.esi);
		printf("edi\t0x%8x\t%u\n",cpu.edi,cpu.edi);
	}
	else if (strcmp(arg,"w") == 0) 
	{
		WP* wp;
		wp = NULL;
		wp = print_wp(wp);
		while (wp != NULL)
		{
			printf("%2d\t%8x\t%u\n",wp->NO, wp->addr, vaddr_read(wp->addr,4));
			wp = wp->next;
		}
	}
	else printf("Unknown command '%s'\n",arg);
	return 0;
}

static int cmd_p(char *args) {
	bool succ = 0;
	int n = expr(args,&succ);
	if (succ)
		printf("%d\n",n);
	else printf("Illegal Sentence\n");
	return 0;
}

static int cmd_x(char *args) {
	char *arg = strtok(NULL, " ");
	int i,n;
	n = 0;
	for (i = 0; i < strlen(arg); i ++)
		n = n * 10 + (int)arg[i] - 48;
	//arg = strtok(NULL, " ");
	char *ex = arg + strlen(arg) + 1;
	bool succ;
	unsigned int scan_addr = expr(ex,&succ);
	Log("scan_addr = %d\n",scan_addr);
	if (!succ)
	{
		printf("Illegal Sentence\n");
		return 0;
	}
	for (i = 0; i < n; i ++)
		printf("0x%8x\t0x%u\n",scan_addr + 4 * i,vaddr_read(scan_addr + 4 * i,4));
	return 0;
}

static int cmd_w(char *args) {

	return 0;
}

static int cmd_d(char *args) {
	return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "run commands step by step",cmd_si },
  { "info", "print the data put in registers or parameters",cmd_info },
  { "p", "print the value of expression",cmd_p },
  { "x", "calculate the value of expression and output the n four-byte",cmd_x },
  { "w", "stop executing when the parameter detected changed",cmd_w },
  { "d", "delect the checkpoints",cmd_d }
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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
