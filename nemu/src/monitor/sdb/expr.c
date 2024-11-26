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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_INT,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", '-'},         //minus
  {"\\*", '*'},         //multiply
  {"\\/", '/'},         //divide
  {"\\(", '('},         //open parenthesis
  {"\\)", ')'},         //close parenthesis
  {"[0-9]+", TK_INT}    // integer
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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  for (int i = 0; i < 32; i++) {
    memset(&tokens[i], 0, sizeof(Token));//empty tokens
  }
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  int here=0;//newhere
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
          case TK_INT:
            for(int a=0;here<position;here++,a++)
            {
              tokens[nr_token].str[a]=e[here];
            }
          default:
          here=position;
          tokens[nr_token].type=rules[i].token_type;
          nr_token+=1;  
          //TODO();
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
bool check_parentheses(int p, int q)//括号判断
{
  int flag=0;
  int flagout=1;
  if(tokens[p].type=='('&&tokens[q].type==')')//表达式是否被一对括号包围
  { 
    flagout=0;
    //左右括号是否匹配
    for(p=p+1;p<q;p++)
    {
      if(flag==-1)
      {
        return false;
      }
      else if(tokens[p].type=='(')
      {
        flag++;
      }
      else if(tokens[p].type==')')
      {
        flag--;
      }
    }
    if(flag==0&&flagout==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}

typedef struct {
  int op;
  int op_type;
} proer;
int proflag=0;
proer pro(int p,int q)//识别主运算符
{
  int sel=1;
  proer resulta={0,0};
  proer resultb={0,0};
  for(;p<q;p++)  
  {
    if(tokens[p].type=='(')//排除括号内容
    while(sel)
    {
      p++;
      if(tokens[p].type==')')
      {
        sel=0;
      }
    }
    //排除非运算符      //判断符号优先级
    if(tokens[p].type=='*'||tokens[p].type=='/')
    {
      proflag=1;
      resulta.op=p;
      resulta.op_type=tokens[p].type;
    }
    if(tokens[p].type=='+'||tokens[p].type=='-')
    {
      proflag=1;
      resultb.op=p;
      resultb.op_type=tokens[p].type;
    }
  }
  if(resulta.op==0)
  {
    return resultb;
  }else
  {
    return resulta;
  }
}

bool tf=true;
uint32_t eval(int p, int q)//递归求值
 {
  tf=true;
  if (p > q) {
    /* Bad expression */
    tf=false;
    return 0;
  }
  if (p == q) 
  {
    /* Single token.
    * For now this token should be a number.
    * Return the value of the number.
    */
    if(tokens[p].type==TK_INT)
    {
      return atoi(tokens[p].str);
    }else
    {
      tf=false;
      return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    // op = the position of 主运算符 in the token expression;
    int op=0;int op_type=0;

    proer result=pro(p,q);
    if(proflag==1)
    {
      op_type=result.op_type;
      op=result.op;

      int val1=0;int val2=0;

      val1 = eval(p, op - 1);
      val2 = eval(op + 1, q);
      switch (op_type) 
      {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        default: assert(0);
      }
    }
  }
  tf=false;
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // eval(1,nr_token);
  int result=eval(0,nr_token-1);
  if(tf==true)
  printf(":%d\n",result);
  else
  printf("error\n");
  // TODO();

  return 0;
}
