#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/helper.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
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

static int cmd_step(char *args)
{
	unsigned int num = atoi(args);
	cpu_exec(num);
	return 0;
}
static int cmd_info(char *args)
{
	switch(*args)
	{
		case 'r' : 
			for(int i = R_EAX;i <= R_EDI ;i++)
			{
				printf("%s:0x%08x\n",regsl[i],cpu.gpr[i]._32);
			};
			return 0;
		default: return 1;
	}
	return 0;
}


char mem_buf[100];
static int cmd_dump_mem(char *args)
{
	char *num = strtok(NULL, " ");
	char *address = strtok(NULL," ");

	int l = sprintf(mem_buf,"起始地址为：%8s\n",address);

	for(int i = 0;i<atoi(num);i++)
	{
		//strtol把字符串中的值改为16进制
		//instr_fetch读取地址中的保存的值
		l+=sprintf(mem_buf+l,"%02x ",instr_fetch((unsigned)strtol(address,NULL,16)+i,1));
	}
	printf("%s\n",mem_buf);
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

	/* TODO: Add more commands */
	{ "s","Signal step", cmd_step},
	{ "info","Dump information with option:r(registers)",cmd_info},
	{ "x","Dump memory:x length addr",cmd_dump_mem},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);

		}

	}

	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}

		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */

		char *args = cmd + strlen(cmd) + 1;
		
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
		
			}
		
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
