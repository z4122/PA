#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	NOTYPE = 256, EQ,NUMBER,PLUS,MINUS,MULTIPLE,DIVIDE

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", PLUS},					// plus
	{"\\-", MINUS},                 // MINUS
	{"\\*", MULTIPLE},				// multiple
	{"\\/", DIVIDE},				// divide
	{"==", EQ},						// equal
	{"\\w", NUMBER}					// number
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) ) //N个不同的操作符

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	int j = 0;	


	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				printf("值为：%c\n",*substr_start);
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */
				
				tokens[j].type = rules[i].token_type;
				tokens[j].str  = *substr_start;	

				j++;
				break;
				
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

int eval(Token tok[],int start,int end)
{
	int a = 0,b = 0,len = 0;
   	len = end-start; 
	if(len == 0) return atoi(&tok[start].str);
	
	for(int i = end;i>start;i--)
	{
		if(tok[i].type == PLUS || tok[i].type == MINUS)
		{
			a = eval(tok,start,i-1);
			b = eval(tok,i+1,end);
		}
		switch(tok[i].type)
		{
			case PLUS:  return a+b; break;
			case MINUS: return a-b; break;
			default : break;
		}
	}

	for(int i = end;i>start;i--)
	{

		if(tok[i].type == MULTIPLE || tok[i].type == DIVIDE)
		{
			a = eval(tok,start,i-1);
			b = eval(tok,i+1,end);
	   	}
		switch(tok[i].type)
		{
			case MULTIPLE:  return a*b; break;
			case DIVIDE:    return a/b; break;
			default :	break; 
	
	   	} 
	}
	return 0;
}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	
    printf("%d\n",eval(tokens,0,strlen(e)-1));
	/* TODO: Insert codes to evaluate the expression. */

	return 0;
}

