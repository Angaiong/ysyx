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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
int a=0;
int choose(int n) {
    return rand() % n;
}
void gen_rand_op()
{
  char operators[] = {'+', '-', '*', '/'};
  int ran = rand() % 4; // 生成0到3之间的随机整数
  if (choose(2)) buf[++a] = ' ';
  buf[++a] = operators[ran];
  if (choose(2)) buf[++a] = ' ';
}
void gen_num()
{
  int num = rand() % 10;
  if (choose(2)) buf[++a] = ' ';
  buf[++a]=num+'0';
}
void gen(char c)
{
  if (choose(2)) buf[++a] = ' ';
  buf[++a]=c;
}
static void gen_rand_expr(int depth, int max_depth) {
  // buf[0] = '\0';
  if (depth >= max_depth) 
  {
    gen_num(); // 如果达到最大深度，只生成一个数字
    return;
  }
  switch (choose(3)) {
    case 0: 
      gen_num(); 
      break;
    case 1: 
      gen('('); 
      gen_rand_expr(depth+1,max_depth); 
      gen(')'); 
      break;
    default: 
      gen_rand_expr(depth+1,max_depth); 
      gen_rand_op(); 
      gen_rand_expr(depth+1,max_depth); 
      break;
  }
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
    a=-1;
    memset(buf, 0, sizeof(buf));
    // buf[a]='\'';
    gen_rand_expr(0,5);
    buf[++a]='\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");//读取了输出的内容 printf(\"%%u\", result);
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
        if (strstr(buf, "/") && strstr(buf, "0")) {
        continue;
    }
    printf("%d %s\n", result, buf);//原本长这样printf("%u %s\n", result, buf);
  }
  return 0;
}