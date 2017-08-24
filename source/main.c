#include <string.h>
#include <stdlib.h>
#include "gba.h"
#include "petitlib.h"
#define OP_MAX 256

enum OPECODE { 
	OP_HAGE , OP_CLS , OP_LOCATE, 
};

const char op_tbl[OP_MAX][16] = {
	"hage", "cls", "locate"
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
	int op;
	init();
	print("petit lib\n");
	print("---------\n");
	print("now booting...\n");
	
	while(1)
	{	
		linput(str);
		op = op_search(str);
		switch (op) {
			default:
				print("> %s\n", str);
				break;
			case OP_HAGE:
				hage();
				break;
			case OP_CLS: {
				cls();
				break;
			}
			case OP_LOCATE: {
				int x, y;
				print("x? : \n");
				linput(str);				
				x = atoi(str);
				print("y? : \n");
				linput(str);
				y = atoi(str);
				locate(x, y);
				break;
			}
		}
	}

	finish();
}