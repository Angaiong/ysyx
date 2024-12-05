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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /*test for expr */
  int i=0;
  int i2=0;  
  extern word_t expr(char *e, bool *success);
  char buffer[256];
  int number;
  char expression[256];
    FILE *file = fopen("/home/xwx/Desktop/ysyx-workbench/nemu/tools/gen-expr/build/input", "r");
    if (file == NULL) {
        perror("无法打开文件");
        return EXIT_FAILURE;
    }

    // 循环读取文件中的每一行
    while (fgets(buffer, sizeof(buffer), file) != NULL) 
    {
      i2++;
      char *space_pos = strchr(buffer, ' ');
      if (space_pos != NULL) 
      {
        *space_pos = '\0';
        strcpy(expression, space_pos + 1);
        number = atoi(buffer);
      } 
      else 
      {
        printf("没有找到空格\n");
        continue;
      }
      expression[strcspn(expression, "\n")] = '\0';
      // printf("Number: %d\nExpression: %s\n", number, expression);
      bool *success=0;
      int test_result=expr(expression, success);
      if(test_result==number)
      {
        printf("is true\n");
        i++;
      }else
      {
        printf("Expression: %s\nNumber: %d\nCalculate:%d\n",expression,number,test_result);
      }
    }
    printf("\nsuccess:%d all:%d\n",i,i2);
    fclose(file);
  /*test for expr */

  /* Start engine. */
  engine_start();
  return is_exit_status_bad();
}
