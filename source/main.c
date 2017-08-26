#include <string.h>
#include <stdlib.h>
#include "gba.h"
#include "petitlib.h"
#include <mappy.h>

#define OP_MAX 256

// 命令のID
enum OPECODE { 
	OP_HAGE , OP_CLS , OP_LOCATE, OP_POKE, OP_PEEK, OP_CALL
};

// 命令の文字列
const char op_tbl[OP_MAX][16] = {
	"hage", "cls", "locate", "poke", "peek", "call"
};

// 比較を繰り返し、命令のIDを割り出す
int op_search(const char* str){
	for (int i = 0; i < OP_MAX; i++){
		if (strcmp(str, op_tbl[i]) == 0)
			return i;
	}
	return -1;
}

// オマケ命令
void hage() {
	static int count;
	count++;
	print("('O') < ");
	for (int i = 0; i < count; i++)
		(i&1) ? print("HAGE! ") : print("kono ");
	print("\n");
}

// 配列データからマシン語を実行する "call"命令 のテスト用配列。
// "call 0x0800d4b9"(適宜変化) で画面に横線がでれば成功。
// アドレスは +1 する必要有り。(thumb命令なので)
const unsigned short call_test[12] = {
	0xB5FF, 0x0000, // スタックに来た道を保存
	0x4900, 0xE001, 0xC000, 0x0600, // 0x0600C000(キャラ画像格納箇所アドレス) を レジスタ1に。
	0x4800, 0xE001, 0x1111, 0x1111, // 0x11111111 を レジスタ0に。
	0x6008, // レジスタ0 の値をレジスタ1のアドレスに書き込み
	0xBDFF, // 来た道へリターン
};

int main()
{
	char str[256];
	char* tok[8];
	int op;
	init();
	print("petit lib\n");
	print("call_test : %08X\n", (unsigned int)call_test);
	print("now booting...\n");
	
	while(1)
	{	
		// 文字列をゲット
		linput(str);
		// 先頭をゲット
		tok[0] = strtok(str, " ");
		// 残りもゲット
		for (int i = 1; i < 8; i++)
			tok[i] = strtok(NULL, ",");
		
		// 先頭を見て、何の命令か判断
		op = op_search(tok[0]);

		// 命令別に処理
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
				unsigned int adr;
				adr = strtoul(tok[1], NULL, 0);
				call(adr);
				break;
			}
		}
	}

	finish();
}