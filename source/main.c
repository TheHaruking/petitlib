#include <string.h>
#include <stdlib.h>
#include "gba.h"
#include "petitlib.h"
#include <mappy.h>
#define OP_MAX 256

enum OPECODE { 
	OP_HAGE , OP_CLS , OP_LOCATE, OP_POKE, OP_PEEK, OP_CALL
};

const char op_tbl[OP_MAX][16] = {
	"hage", "cls", "locate", "poke", "peek", "call"
};

int op_search(const char* str){
	for (int i = 0; i < OP_MAX; i++){
		if (strcmp(str, op_tbl[i]) == 0)
			return i;
	}
	return -1;
}

void hage() {
	static int count;
	count++;
	print("('O') < ");
	for (int i = 0; i < count; i++)
		(i&1) ? print("HAGE! ") : print("kono ");
	print("\n");
}

int main()
{
	char str[256];
	char* tok[8];
	int op;
	init();
	print("petit lib\n");
	print("---------\n");
	print("now booting...\n");
	
	while(1)
	{	
		linput(str);
		tok[0] = strtok(str, " ");
		for (int i = 1; i < 8; i++)
			tok[i] = strtok(NULL, ",");
		op = op_search(tok[0]);
		switch (op) {
			default: {
				print("> %s\n", str);
				break;
			}
			case OP_HAGE: {
				hage();
				break;
			}
			case OP_CLS: {
				cls();
				break;
			}
			case OP_LOCATE: {
				int x, y;
				x = atoi(tok[1]);
				y = atoi(tok[2]);
				locate(x, y);
				break;
			}
			case OP_POKE: {
				int x, y;
				x = strtoul(tok[1], NULL, 0);
				y = strtoul(tok[2], NULL, 0);
				poke(x, y);
				break;
			}
			case OP_PEEK: {
				unsigned int x;
				unsigned short ret;
				x = strtoul(tok[1], NULL, 0);
				ret = peek(x);
				print("> %04X\n", ret);
				break;
			}
			case OP_CALL: {
				break;
			}
		}
	}

	finish();
}