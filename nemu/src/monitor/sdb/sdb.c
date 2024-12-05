/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

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
  return -1;
}

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  // printf("%ld\n",strlen(arg));
  int a=0;
  int flag=0;
  if (arg != NULL) 
  {
    for(a=0;a<strlen(arg);a++)
    {
      if( arg[a]>= 48 && arg[a]<= 57)//[0-9]
      {
        flag=1;
      }
      // printf("flag=%d\narg=%d\n",flag,arg[a]);
    }
  }
  if (arg == NULL) {
    cpu_exec(1);
  }else if(flag){
    int num=atoi(arg);
    cpu_exec(num);
    flag=0;
  }else{
    printf("error:(si N)N is a number\n");
  }
	return 0;
};//newsi

void isa_reg_display();//newinfo
static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("error:(info r/w)\n");
  }
  else if (*arg == 'r') 
  {
    isa_reg_display();
  }else if (*arg == 'w') 
  {
    printf("还没写\n");
  }else
  {
    printf("error:(info r/w)\n");
  }
  return 0;
};//newinfo
static int cmd_x(char *args)
{
  extern word_t vaddr_read();

  // printf("%x\n",vaddr_read(2147483648,4));

  char *arg = strtok(NULL, " ");
  char *arg2 = strtok(NULL, " ");
  int n=0;
  int num=0;
  if (arg==NULL||arg2==NULL)
  {
    printf("error:(x N EXPR)\n");
  }else {
    sscanf(arg2, "%x", &num);
    sscanf(arg, "%x", &n);
  }
  if(1)//(int)num==(int)0x80000000)
  {
    for(int b=0;b<n;b++)
    {
      printf("0x%x:",num);
      for(int a=0;a<4;a++)
      {
        printf(" %02x",vaddr_read(num+3-a,1));
      }
      printf("\n");
      num=num+4;
    }
  }else{
    printf("还没写\n");
  }
  return 0;
}//newx
static int cmd_p(char *args)
{
  char *arg = strtok(NULL, "");
  bool *success=0;
  if(arg==NULL)
  {
    printf("p %s\n",arg);
  }
  else
  {
    expr(arg,success);
  }
  return 0;
}
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q   },
  {	"si", "si [N]:N-step execution", cmd_si },//newsi
  { "info", "info r:Print register status\n\tinfo w:Print watch information", cmd_info},//newinfo
  { "x", "x N EXPR:Evaluate the expression EXPR and use the result as the starting memory Address, output N consecutive 4 bytes in hexadecimal form",cmd_x},//newx
  { "p", "p EXPR:Find the value of the expression EXPR", cmd_p},//newp

  /* TODO: Add more commands */

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

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
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
