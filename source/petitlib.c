#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gba.h>
#include <mappy.h>
#include "petitlib.h"

////////////////////////////////
// ## define
////////////////////////////////
#define CON_MAX_X 30
#define CON_MAX_Y 20
#define CON_LINEBUF_SIZE 256
#define CON_HISTORY_SIZE 32

#define INPUT_KEY_A	KEY_A
#define INPUT_KEY_B KEY_B
#define INPUT_KEY_X 0
#define INPUT_KEY_Y 0
#define INPUT_KEY_L KEY_L
#define INPUT_KEY_R KEY_R
#define INPUT_KEY_SELECT KEY_SELECT
#define INPUT_KEY_START KEY_START

#define INPUT_KEY_UP KEY_UP
#define INPUT_KEY_DN KEY_DOWN
#define INPUT_KEY_LE KEY_LEFT
#define INPUT_KEY_RI KEY_RIGHT

#define INPUT_KEY_AB (KEY_A | KEY_B)
#define INPUT_KEY_XY 0
#define INPUT_KEY_LR (KEY_L | KEY_R)
#define INPUT_KEY_DPAD (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)
#define INPUT_KEY_DPAD_Y (KEY_UP | KEY_DOWN)
#define INPUT_KEY_DPAD_X (KEY_LEFT | KEY_RIGHT)
#define INPUT_KEY_OPTIONAL (KEY_START | KEY_SELECT)
#define INPUT_KEY_ALL (0xFFFFFFFF)

#define INPUT_AB2_A 0
#define INPUT_AB2_B 1

#define INPUT_AB4_A  0
#define INPUT_AB4_AB 1
#define INPUT_AB4_B  2
#define INPUT_AB4_BA 3

#define INPUT_DPAD4_UP 0
#define INPUT_DPAD4_DN 1
#define INPUT_DPAD4_LE 2
#define INPUT_DPAD4_RI 3

#define INPUT_NONE -1

#define INPUT_FIRST_NONE 0
#define INPUT_FIRST_AB  1
#define INPUT_FIRST_DPAD 2
#define INPUT_FIRST_L  3


////////////////////////////////
// ## const data
////////////////////////////////
// ボタン関連
// ↑ が 0 。↗ が 1 。
// 右周りで、↖ が 7 。
static const int num_8_tbl[16] = {
	// → ←
	-1, 2, 6,-1,
	 0, 1, 7,-1, // ↑
	 4, 3, 5,-1, // ↓
	-1,-1,-1,-1,
};

// ↑ が 0 。→ が 1 。
// 右周りで、← が 3 。
static const int num_4_tbl[16] = {
	//  →  ←
	-1,  1,  3, -1,
	 0, -1, -1, -1, // ↑
	 2, -1, -1, -1, // ↓
	-1, -1, -1, -1,
};

// 最初 ↑、押したまま ↖ で、 0 。
// 最初 ↑、押したまま ↑ で、 1 。
// 最初 ↑、押したまま ↗ で、 2 。
// 最初 →、押したまま ↗ で、 3 。
// 最初 →、押したまま → で、 4 。
// 最初 →、押したまま ↘ で、 5 。
// ...
// 最初 ←、押したまま ↖ で、 11。
static const int num_4_3_tbl[4][8] = {
	{	1, 2,-1,-1,-1,-1,-1, 0, },
	{  -1, 0, 1, 2,-1,-1,-1,-1, },
	{  -1,-1,-1, 0, 1, 2,-1,-1, },
	{  -1,-1,-1,-1,-1, 0, 1, 2, },
};

// 最初 ↑、押したまま ← で、 0 。
// 最初 ↑、押したまま ↖ で、 1 。
// 最初 ↑、押したまま ↑ で、 2 。
// 最初 ↑、押したまま ↗ で、 3 。
// 最初 ↑、押したまま → で、 4 。
// 最初 →、押したまま ↑ で、 5 。
// ...
// 最初 ←、押したまま ↑ で、 19。
static const int num_4_5_tbl[4][8] = {
	{   2, 3, 4,-1,-1,-1, 0, 1, },
	{   0, 1, 2, 3, 4,-1,-1,-1, },
	{  -1,-1, 0, 1, 2, 3, 4,-1, },
	{   4,-1,-1,-1, 0, 1, 2, 3, },
};
// 最大32まで可能ですが、操作性を考えると限度は4 x 5個(90度の指移動)です。

// 文字入力間連
static const char input_sym_tbl[2][4][4] = { {
	{'(', ')', '.', ',' }, {'<', '>', '\'','\"'}, 
	{'_', '|', '+', '-' }, {'!', '?', '@', '`' }, 
},{
	{'{', '}', ':', ';' }, {'[', ']', '^', '~' }, 
	{'/', '\\','*', '=' }, {'#', '$', '%', '&' }, 
} };

// 旧
static const char input_alph_tbl[2][2][4][5] ={ { {
	{ '\0', 'a', 'b', 'c', '\0' }, { '\0', 'd', 'e', 'f', '\0' }, 
	{ '\0', 'g', 'h', 'i', '\0' }, { '\0', 'j', 'k', 'l', '\0' }, 
},{
	{ '\0', 'm', 'n', 'o', '\0' }, { '\0', 'p', 'r', 's', '\0' }, 
	{ '\0', 't', 'u', 'v', '\0' }, {  'q', 'w', 'x', 'y', 'z'  }, 
},},{ {
	{ '\0', 'A', 'B', 'C', '\0' }, { '\0', 'D', 'E', 'F', '\0' }, 
	{ '\0', 'G', 'H', 'I', '\0' }, { '\0', 'J', 'K', 'L', '\0' }, 
},{
	{ '\0', 'M', 'N', 'O', '\0' }, { '\0', 'P', 'R', 'S', '\0' }, 
	{ '\0', 'T', 'U', 'V', '\0' }, {  'Q', 'W', 'X', 'Y', 'Z'  }, 
} } };

static const char input_num_tbl[2][8] = { {	
	'3', '4', '5', '\0', '\0', '\0', '1', '2', 
},{	
	'6', '7', '8', '\0', '\0', '\0', '9', '0', 
} };

// 新
static const char input_alph_num_tbl[2][8][4] ={ {
	{ 'a', 'b', 'c', '1' }, { 'd', 'e', 'f', '2' }, 
	{ 'g', 'h', 'i', '3' }, { 'j', 'k', 'l', '4' }, 
	{ 'm', 'n', 'o', '5' }, { 'p', 'q', 'r', 's' }, 
	{ 't', 'u', 'v', '.' }, { 'w', 'x', 'y', 'z' }, 
},{
	{ 'A', 'B', 'C', '6' }, { 'D', 'E', 'F', '7' }, 
	{ 'G', 'H', 'I', '8' }, { 'J', 'K', 'L', '9' }, 
	{ 'M', 'N', 'O', '0' }, { 'P', 'Q', 'R', 'S' }, 
	{ 'T', 'U', 'V', ',' }, { 'W', 'X', 'Y', 'Z' }, 
} };

////////////////////////////////
// ## 構造体
// - プチライブラリ
//   - ボタン
//   - コンソール
//     - バッファ(インプット)
//     - バッファ(アウトプット)
////////////////////////////////

// バッファ
typedef struct {
	int bufp;
	char buf[CON_LINEBUF_SIZE];
} buf_t;

// コンソール
typedef struct {
	unsigned int y;
	unsigned int x;
	buf_t out;
	buf_t in;
	buf_t history[CON_HISTORY_SIZE];
} console_t;

// ボタン
typedef struct {
	unsigned int b[4];
	unsigned int ab[4];
	unsigned int dpad[4];

	int ab_2;
	int ab_4;
	int ab_4_stack;
	int ab_4_pressed;
	int ab_4_hold;
	int dpad_4;
	int dpad_4_old;
	int dpad_4_stack;
	int dpad_8;
	int dpad_first;
	int dpad_4_3;
	int dpad_4_5;
	unsigned int stack[16];
	int stack_p;
	int shift_first;
	int r_hold;
	int r_stack;
	int l_hold;
	int all_first;
} button_t;

// プチライブラリ
typedef struct {
	console_t con;
	button_t btn;
} lib_t;

////////////////////////////////
// ## static 変数 関数
////////////////////////////////
// static 変数
static lib_t* lib;

// static 関数
static void conadr_inc();
static void conadr_dec();
static void conadr_break();
static void conadr_backspace();
static inline int conadr_get();
static void print_char(char c);
static void print_str(const char* str);
static char input_char();

static void btn_update_array(unsigned int dst[4], unsigned int src, unsigned int mask);
static void btn_update(button_t* btn);
static void btn_pop_r(button_t* btn);

static void buf_init(buf_t* b);
static void buf_push(buf_t* b, char c);
static void buf_change(buf_t* b, char c);
static char buf_pop(buf_t* b);
static inline char buf_get(const buf_t* b);


////////////////////////////////
// ## static 関数
////////////////////////////////
static void conadr_inc(){
	lib->con.x++;
	if(lib->con.x >= CON_MAX_X){
		lib->con.y++;
		lib->con.y &= 63;
		lib->con.x = 0;
	}
}

static void conadr_dec(){
	lib->con.x--;
	if(lib->con.x < 0){
		lib->con.y--;
		lib->con.y &= 63;
	}
}

static void conadr_break(){
	lib->con.x = 0;
	lib->con.y++;
	lib->con.y &= 63;
}

static void conadr_backspace(){
	if(!buf_pop(&lib->con.in))
		return;
	conadr_dec();
	*(u16 *)(MAP_BASE_ADR(0) + conadr_get()) = ' ';
	return;
}

static inline int conadr_get(){
	return lib->con.x * 2 + lib->con.y * 32 * 2;
}



static void print_char(char c){
	switch(c){
		default:
			*(u16 *)(MAP_BASE_ADR(0) + conadr_get()) = (u16)c;
			conadr_inc();			
			break;
		case '\n':
			conadr_break();
			break;
		case '\b':
			conadr_backspace();
			break;
	}
}

static void print_str(const char* str){
	for(int i = 0; str[i]; i++){
		print_char(str[i]);
	}
}

static char input_char(){
	char c = '\0';
	switch (lib->btn.all_first) {
		case INPUT_FIRST_DPAD:
			if ((lib->btn.dpad_8 >= 0) && (lib->btn.ab_4 >= 0)) {
				// 左からスタート
				//int dpad_first_left0 = (lib->btn.dpad_first + 1) & 0x03;
				//c = input_alph_tbl[lib->btn.r_hold][lib->btn.ab_2][dpad_first_left0][lib->btn.dpad_4_5];
				int dpad_8_left_to_0 = (lib->btn.dpad_8 + 2) & 0x07;
				c = input_alph_num_tbl[lib->btn.r_hold][dpad_8_left_to_0][lib->btn.ab_4];
			}
			break;
		
		case INPUT_FIRST_L:
			if (lib->btn.ab_2 >= 0) {
				switch(lib->btn.ab_2){
					case INPUT_AB2_A:
						c = ' ';
						break;
					case INPUT_AB2_B:
						c = '\b';
						break;
				}
			}

		case INPUT_FIRST_NONE:
			switch(lib->btn.b[1] & INPUT_KEY_START) {
				case INPUT_KEY_START:
					c = '\n';
					break;
			}
			break;
	}
	return c;
}

static void btn_update_array(unsigned int dst[4], unsigned int src, unsigned int mask) {
	dst[3] = dst[0];
	dst[0] = src & mask;
	dst[1] = (dst[0] ^ dst[3]) & dst[0];
	dst[2] = (dst[0] ^ dst[3]) & dst[3];
}

void btn_update(button_t* btn){
	scanKeys();
	btn_update_array(btn->b,    (u32)keysHeld(), INPUT_KEY_ALL);
	btn_update_array(btn->ab,   btn->b[0], INPUT_KEY_AB);
	btn_update_array(btn->dpad, btn->b[0], INPUT_KEY_DPAD);

	// 方向化
	btn->dpad_8 = num_8_tbl[btn->dpad[0] >> 4]; // DPAD は2桁目 (00X0)
	btn->dpad_4_old = btn->dpad_4;
	btn->dpad_4 = num_4_tbl[btn->dpad[0] >> 4];
	if ((btn->dpad_4 < 0) && (btn->dpad[0])) {
		btn->dpad_4 = btn->dpad_4_old;
	}	

	// 最初に押した方向を保存。 ( 上右下左:0123。ななめ無視)
	if (!btn->dpad[3]) { // 前フレームに十字押していない
		btn->dpad_first = num_4_tbl[btn->dpad[1] >> 4];
	}
	// 離されていれば -1
	if (!btn->dpad[0])
		btn->dpad_first = -1;
	
	if(btn->dpad_first >= 0) {
		btn->dpad_4_3 = num_4_3_tbl[btn->dpad_first][btn->dpad_8];
		btn->dpad_4_5 = num_4_5_tbl[btn->dpad_first][btn->dpad_8];
	} else {
		btn->dpad_4_3 = -1;
		btn->dpad_4_5 = -1;
	}

	switch (btn->ab[1]) {
		case INPUT_KEY_A:
			btn->ab_2 = INPUT_AB2_A;
			break;
		case INPUT_KEY_B:
			btn->ab_2 = INPUT_AB2_B;
			break;
		default:
			btn->ab_2 = INPUT_NONE;
			break;
	}

	// AB4 処理 
	int ab_push = btn->ab[1]; 
	int ab_rrse = btn->ab[2]; 
	int ab_hold = btn->ab[0] ^ btn->ab[1]; 
	int a_p = ((ab_push & INPUT_KEY_A) > 0) << 0; // 0x01 ... A を 押した 
	int b_p = ((ab_push & INPUT_KEY_B) > 0) << 1; // 0x02 ... B を 押した 
	int a_r = ((ab_rrse & INPUT_KEY_A) > 0) << 2; // 0x04 ... A を 離した 
	int b_r = ((ab_rrse & INPUT_KEY_B) > 0) << 3; // 0x08 ... B を 離した 
	int a_h = ((ab_hold & INPUT_KEY_A) > 0) << 4; // 0x10 ... A を 押している(押した瞬間除く) 
	int b_h = ((ab_hold & INPUT_KEY_B) > 0) << 5; // 0x20 ... B を 押している(押した瞬間除く) 

	// ab_4_pressed : xA, xBした後、元ボタン離した際 iA, iB が反応しないようにするフラグ
	// ボタンが押されていないとき(離した瞬間は含まない)は解除しておく
	if( !btn->ab[0] ) {
		btn->ab_4_hold = -1;
		if (!btn->ab[2])
			btn->ab_4_pressed = 0;
	}

	btn->ab_4 = -1;
	switch (a_p | b_p | a_r | b_r | a_h | b_h) {
		case 0x04: // iA
			if (btn->ab_4_pressed)
				break;
			btn->ab_4 = INPUT_AB4_A; 
			break;
		case 0x08: // iB
			if (btn->ab_4_pressed)
				break;
			btn->ab_4 = INPUT_AB4_B; 
			break;
		case 0x12: // xA
			btn->ab_4 = INPUT_AB4_AB; 
			if (btn->ab_4_pressed)
				break;
			btn->ab_4_hold = INPUT_AB4_AB;
			btn->ab_4_pressed = 1; 
			break;
		case 0x21: // xB
			btn->ab_4 = INPUT_AB4_BA;
			if (btn->ab_4_pressed)
				break;
			btn->ab_4_hold = INPUT_AB4_BA;
			btn->ab_4_pressed = 1; 
			break;
		case 0x01: // Aを押した
			if (btn->ab_4_pressed)
				break;
			btn->ab_4_hold = INPUT_AB4_A;
			break;
		case 0x02: // Bを押した
			if (btn->ab_4_pressed)
				break;
			btn->ab_4_hold = INPUT_AB4_B;
			break;
	}

	// 最後のab4を保存しておく
	if(btn->ab_4)
		btn->ab_4_stack = btn->ab_4;

	// stack
	if(btn->b[1]) {
		btn->stack[btn->stack_p] = btn->b[1];
		btn->stack_p = btn->stack_p + 1 & 0x0f;
	}
		
	// l r
	btn->r_hold = (btn->b[0] & INPUT_KEY_R) > 0;
	btn->l_hold = (btn->b[0] & INPUT_KEY_L) > 0;

	// first key
	unsigned int hold_keys = ((btn->dpad_8 >= 0) << 0) | ((btn->ab_4_hold >= 0) << 1) | ((btn->l_hold) << 2);
	if (!btn->b[0])
		btn->all_first = 0;
	if (!btn->all_first) {
		switch (hold_keys){
			case 0x01: // DPAD
				btn->all_first = INPUT_FIRST_DPAD;
				break;
			case 0x02: // A B
				btn->all_first = INPUT_FIRST_AB;
				break;
			case 0x04: // L
				btn->all_first = INPUT_FIRST_L;
				break;
		}
	}
	dprintf("all_first : %08X\n", btn->all_first);
}

static void buf_init(buf_t* b){
	b->buf[0] = '\0';
	b->bufp = 0;
}
 
static void buf_push(buf_t* b, char c){
	if( b->bufp >= CON_LINEBUF_SIZE )
		return;

	b->bufp++;
	b->buf[b->bufp - 1] = c;
	b->buf[b->bufp    ] = '\0';
}

static void buf_change(buf_t* b, char c){
	b->buf[b->bufp - 1] = c;
}

static char buf_pop(buf_t* b){
	char c;
	if ( b->bufp <= 0)
		return '\0';
	
	c = b->buf[b->bufp - 1];
	b->buf[b->bufp - 1] = '\0';
	b->bufp--;
	return c;
}

static inline char buf_get(const buf_t* b){
	return b->buf[b->bufp - 1];
}



////////////////////////////////
// ## extern 関数
////////////////////////////////
// 必須関数
void init(){
	lib = (lib_t*)malloc(sizeof(lib_t));
	if(lib == NULL)
		return;

	// irq有効化
	irqInit();
	irqEnable(IRQ_VBLANK);
	// 映像初期化
	SetMode(MODE_0 | BG_ALL_ON | OBJ_ON);
	// BG_SIZE(3)   ... 512 x 512 [0x2000]
	// MAP_BASE(0)  ... 0x06000000
	// MAP_BASE(4)  ... 0x06002000
	// MAP_BASE(8)  ... 0x06004000
	// MAP_BASE(12) ... 0x06006000
	// CHAR_BASE(3) ... 0x0600C000 [0x06013FFF]
	// MAX_VRAM     ... 0x06017FFF
	BGCTRL[0] = MAP_BASE(0)  | CHAR_BASE(3) | TEXTBG_SIZE_512x512;
	BGCTRL[1] = MAP_BASE(4)  | CHAR_BASE(3) | TEXTBG_SIZE_512x512;
	BGCTRL[2] = MAP_BASE(8)  | CHAR_BASE(3) | TEXTBG_SIZE_512x512;
	BGCTRL[3] = MAP_BASE(12) | CHAR_BASE(3) | TEXTBG_SIZE_512x512;
	// 画像データ読み込み
	dmaCopy(chr003Tiles,	CHAR_BASE_ADR(3),	chr003TilesLen);
	dmaCopy(chr003Pal,		BG_COLORS,		chr003PalLen);
}

void finish(){
	free(lib);
}

// ファイル関数
void save(){}
void load(){}

// 画面基本設定関数
void visible(){}
void xscreen(){}

// コンソール制御関数
void print(const char* str, ...){
	va_list ap;
	va_start(ap, str);
	vsprintf(lib->con.out.buf, str, ap);
	print_str(lib->con.out.buf);
	va_end(ap);
}

void linput(char* str){
	char c;
	buf_init(&lib->con.in);
	while( 1 ){
		btn_update(&lib->btn);
		c = input_char();
		if(c != '\0') {
			buf_push(&lib->con.in, c);
			print_char(c);
			if( buf_get(&lib->con.in) == '\n')
				break;
		}
		VBlankIntrWait();
	}
	strcpy(str, lib->con.in.buf);
}

// スプライト関数
// バックグラウンド画面関数
// 各種入出力
// 音関係
// 通信



/*
 * データ一覧
 */


// 画像データ
//{{BLOCK(chr003)

//======================================================================
//
//	chr003, 128x128@4, 
//	+ palette 256 entries, not compressed
//	+ 256 tiles not compressed
//	Total size: 512 + 8192 = 8704
//
//	Time-stamp: 2017-06-18, 17:12:11
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.14
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

const unsigned int chr003Tiles[2048] __attribute__((aligned(4))) __attribute__((visibility("hidden")))=
{
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,
	0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,
	0x00000001,0x00000011,0x00000111,0x00001111,0x00011111,0x00111111,0x01111111,0x11111111,
	0x00000002,0x00000022,0x00000222,0x00002222,0x00022222,0x00222222,0x02222222,0x22222222,
	0x00000003,0x00000033,0x00000333,0x00003333,0x00033333,0x00333333,0x03333333,0x33333333,
	0x00011000,0x00011000,0x00111100,0x00111100,0x01111110,0x01111110,0x11111111,0x11111111,

	0x00022000,0x00022000,0x00222200,0x00222200,0x02222220,0x02222220,0x22222222,0x22222222,
	0x00033000,0x00033000,0x00333300,0x00333300,0x03333330,0x03333330,0x33333333,0x33333333,
	0x00000011,0x00001111,0x00111111,0x11111111,0x11111111,0x00111111,0x00001111,0x00000011,
	0x00000022,0x00002222,0x00222222,0x22222222,0x22222222,0x00222222,0x00002222,0x00000022,
	0x00000033,0x00003333,0x00333333,0x33333333,0x33333333,0x00333333,0x00003333,0x00000033,
	0x00001111,0x00111111,0x01111111,0x01111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x00002222,0x00222222,0x02222222,0x02222222,0x22222222,0x22222222,0x22222222,0x22222222,
	0x00003333,0x00333333,0x03333333,0x03333333,0x33333333,0x33333333,0x33333333,0x33333333,

	0x11000000,0x11100000,0x11110000,0x11110000,0x11110000,0x11110000,0x11100000,0x11000000,
	0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,0x00000003,
	0x00000033,0x00000033,0x00000033,0x00000033,0x00000033,0x00000033,0x00000033,0x00000033,
	0x00000333,0x00000333,0x00000333,0x00000333,0x00000333,0x00000333,0x00000333,0x00000333,
	0x00003333,0x00003333,0x00003333,0x00003333,0x00003333,0x00003333,0x00003333,0x00003333,
	0x00033333,0x00033333,0x00033333,0x00033333,0x00033333,0x00033333,0x00033333,0x00033333,
	0x00333333,0x00333333,0x00333333,0x00333333,0x00333333,0x00333333,0x00333333,0x00333333,
	0x03333333,0x03333333,0x03333333,0x03333333,0x03333333,0x03333333,0x03333333,0x03333333,

	0x11111111,0x11111111,0x01111110,0x00111100,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x33333333,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x33333333,0x33333333,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x33333333,0x33333333,0x33333333,
	0x00000000,0x00000000,0x00000000,0x00000000,0x33333333,0x33333333,0x33333333,0x33333333,
	0x00000000,0x00000000,0x00000000,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,
	0x00000000,0x00000000,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,
	0x00000000,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,

	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00001000,0x00001000,0x00001000,0x00001000,0x00001000,0x00000000,0x00001000,0x00000000,
	0x00001010,0x00001010,0x00001010,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00010100,0x00010100,0x00111110,0x00010100,0x00111110,0x00010100,0x00010100,0x00000000,
	0x00001000,0x00111100,0x00001010,0x00011100,0x00101000,0x00011110,0x00001000,0x00000000,
	0x00000110,0x00100110,0x00010000,0x00001000,0x00000100,0x00110010,0x00110000,0x00000000,
	0x00000100,0x00001010,0x00001010,0x00000100,0x00101010,0x00010010,0x00101100,0x00000000,
	0x00001100,0x00001000,0x00000100,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,

	0x00010000,0x00001000,0x00000100,0x00000100,0x00000100,0x00001000,0x00010000,0x00000000,
	0x00000100,0x00001000,0x00010000,0x00010000,0x00010000,0x00001000,0x00000100,0x00000000,
	0x00000000,0x00101010,0x00011100,0x00111110,0x00011100,0x00101010,0x00000000,0x00000000,
	0x00000000,0x00001000,0x00001000,0x00111110,0x00001000,0x00001000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00001100,0x00001000,0x00000100,
	0x00000000,0x00000000,0x00000000,0x00111110,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00001100,0x00001100,0x00000000,
	0x00000000,0x01000000,0x00100000,0x00010000,0x00001000,0x00000100,0x00000010,0x00000000,

	0x00011100,0x00100010,0x00110010,0x00101010,0x00100110,0x00100010,0x00011100,0x00000000,
	0x00001000,0x00001110,0x00001000,0x00001000,0x00001000,0x00001000,0x00111110,0x00000000,
	0x00011100,0x00100010,0x00100000,0x00010000,0x00001000,0x00000100,0x00111110,0x00000000,
	0x00011100,0x00100010,0x00100000,0x00011000,0x00100000,0x00100010,0x00011100,0x00000000,
	0x00010000,0x00011000,0x00010100,0x00010010,0x00010010,0x00111110,0x00010000,0x00000000,
	0x00111110,0x00000010,0x00011110,0x00100010,0x00100000,0x00100010,0x00011100,0x00000000,
	0x00011000,0x00000100,0x00000010,0x00011110,0x00100010,0x00100010,0x00011100,0x00000000,
	0x00111110,0x00100010,0x00010000,0x00001000,0x00001000,0x00001000,0x00001000,0x00000000,

	0x00011100,0x00100010,0x00100010,0x00011100,0x00100010,0x00100010,0x00011100,0x00000000,
	0x00011100,0x00100010,0x00100010,0x00111100,0x00100000,0x00100000,0x00011100,0x00000000,
	0x00000000,0x00000000,0x00001000,0x00000000,0x00000000,0x00001000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00001000,0x00000000,0x00000000,0x00001000,0x00001000,0x00000100,
	0x00100000,0x00010000,0x00001000,0x00000100,0x00001000,0x00010000,0x00100000,0x00000000,
	0x00000000,0x00000000,0x00111110,0x00000000,0x00111110,0x00000000,0x00000000,0x00000000,
	0x00000010,0x00000100,0x00001000,0x00010000,0x00001000,0x00000100,0x00000010,0x00000000,
	0x00011100,0x00100010,0x00100000,0x00010000,0x00001000,0x00000000,0x00001000,0x00000000,

	0x00011000,0x00100100,0x00110010,0x00101010,0x00010010,0x00000100,0x00111000,0x00000000,
	0x00001000,0x00010100,0x00100010,0x00100010,0x00111110,0x00100010,0x00100010,0x00000000,
	0x00011110,0x00100010,0x00100010,0x00011110,0x00100010,0x00100010,0x00011110,0x00000000,
	0x00011100,0x00100010,0x00000010,0x00000010,0x00000010,0x00100010,0x00011100,0x00000000,
	0x00001110,0x00010010,0x00100010,0x00100010,0x00100010,0x00010010,0x00001110,0x00000000,
	0x00111110,0x00000010,0x00000010,0x00011110,0x00000010,0x00000010,0x00111110,0x00000000,
	0x00111110,0x00000010,0x00000010,0x00011110,0x00000010,0x00000010,0x00000010,0x00000000,
	0x00111100,0x00000010,0x00000010,0x00111010,0x00100010,0x00100010,0x00111100,0x00000000,

	0x00100010,0x00100010,0x00100010,0x00111110,0x00100010,0x00100010,0x00100010,0x00000000,
	0x00011100,0x00001000,0x00001000,0x00001000,0x00001000,0x00001000,0x00011100,0x00000000,
	0x00111000,0x00010000,0x00010000,0x00010000,0x00010000,0x00010010,0x00001100,0x00000000,
	0x00100010,0x00010010,0x00001010,0x00000110,0x00001010,0x00010010,0x00100010,0x00000000,
	0x00000010,0x00000010,0x00000010,0x00000010,0x00000010,0x00000010,0x00111110,0x00000000,
	0x00100010,0x00110110,0x00101010,0x00100010,0x00100010,0x00100010,0x00100010,0x00000000,
	0x00100010,0x00100110,0x00101010,0x00110010,0x00100010,0x00100010,0x00100010,0x00000000,
	0x00011100,0x00100010,0x00100010,0x00100010,0x00100010,0x00100010,0x00011100,0x00000000,

	0x00011110,0x00100010,0x00100010,0x00011110,0x00000010,0x00000010,0x00000010,0x00000000,
	0x00011100,0x00100010,0x00100010,0x00100010,0x00101010,0x00010010,0x00101100,0x00000000,
	0x00011110,0x00100010,0x00100010,0x00011110,0x00001010,0x00010010,0x00100010,0x00000000,
	0x00011100,0x00100010,0x00000010,0x00011100,0x00100000,0x00100010,0x00011100,0x00000000,
	0x00111110,0x00001000,0x00001000,0x00001000,0x00001000,0x00001000,0x00001000,0x00000000,
	0x00100010,0x00100010,0x00100010,0x00100010,0x00100010,0x00100010,0x00011100,0x00000000,
	0x00100010,0x00100010,0x00100010,0x00010100,0x00010100,0x00001000,0x00001000,0x00000000,
	0x00100010,0x00100010,0x00100010,0x00100010,0x00101010,0x00110110,0x00100010,0x00000000,

	0x00100010,0x00100010,0x00010100,0x00001000,0x00010100,0x00100010,0x00100010,0x00000000,
	0x00100010,0x00100010,0x00010100,0x00001000,0x00001000,0x00001000,0x00001000,0x00000000,
	0x00111110,0x00100000,0x00010000,0x00001000,0x00000100,0x00000010,0x00111110,0x00000000,
	0x00011100,0x00000100,0x00000100,0x00000100,0x00000100,0x00000100,0x00011100,0x00000000,
	0x00100010,0x00010100,0x00001000,0x00111110,0x00001000,0x00111110,0x00001000,0x00000000,
	0x00011100,0x00010000,0x00010000,0x00010000,0x00010000,0x00010000,0x00011100,0x00000000,
	0x00001000,0x00010100,0x00100010,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x01111111,0x00000000,

	0x00000100,0x00001000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00011100,0x00100000,0x00111100,0x00100010,0x00111100,0x00000000,
	0x00000010,0x00000010,0x00011110,0x00100010,0x00100010,0x00100010,0x00011110,0x00000000,
	0x00000000,0x00000000,0x00111100,0x00000010,0x00000010,0x00000010,0x00111100,0x00000000,
	0x00100000,0x00100000,0x00111100,0x00100010,0x00100010,0x00100010,0x00111100,0x00000000,
	0x00000000,0x00000000,0x00011100,0x00100010,0x00111110,0x00000010,0x00011100,0x00000000,
	0x00110000,0x00001000,0x00001000,0x00111110,0x00001000,0x00001000,0x00001000,0x00000000,
	0x00000000,0x00000000,0x00111100,0x00100010,0x00100010,0x00111100,0x00100000,0x00011100,

	0x00000010,0x00000010,0x00011010,0x00100110,0x00100010,0x00100010,0x00100010,0x00000000,
	0x00001000,0x00000000,0x00001100,0x00001000,0x00001000,0x00001000,0x00011100,0x00000000,
	0x00100000,0x00000000,0x00110000,0x00100000,0x00100000,0x00100000,0x00100010,0x00011100,
	0x00000010,0x00000010,0x00100010,0x00010010,0x00001010,0x00010110,0x00100010,0x00000000,
	0x00001100,0x00001000,0x00001000,0x00001000,0x00001000,0x00001000,0x00011100,0x00000000,
	0x00000000,0x00000000,0x00011110,0x00101010,0x00101010,0x00101010,0x00101010,0x00000000,
	0x00000000,0x00000000,0x00011110,0x00100010,0x00100010,0x00100010,0x00100010,0x00000000,
	0x00000000,0x00000000,0x00011100,0x00100010,0x00100010,0x00100010,0x00011100,0x00000000,

	0x00000000,0x00000000,0x00011110,0x00100010,0x00100010,0x00011110,0x00000010,0x00000010,
	0x00000000,0x00000000,0x00111100,0x00100010,0x00100010,0x00111100,0x00100000,0x00100000,
	0x00000000,0x00000000,0x00011010,0x00100110,0x00000010,0x00000010,0x00000010,0x00000000,
	0x00000000,0x00000000,0x00111100,0x00000010,0x00011100,0x00100000,0x00011110,0x00000000,
	0x00001000,0x00001000,0x00111110,0x00001000,0x00001000,0x00001000,0x00110000,0x00000000,
	0x00000000,0x00000000,0x00100010,0x00100010,0x00100010,0x00100010,0x00111100,0x00000000,
	0x00000000,0x00000000,0x00100010,0x00100010,0x00100010,0x00010100,0x00001000,0x00000000,
	0x00000000,0x00000000,0x00100010,0x00101010,0x00101010,0x00101010,0x00010100,0x00000000,

	0x00000000,0x00000000,0x00100010,0x00010100,0x00001000,0x00010100,0x00100010,0x00000000,
	0x00000000,0x00000000,0x00100010,0x00100010,0x00100010,0x00111100,0x00100000,0x00011100,
	0x00000000,0x00000000,0x00111110,0x00010000,0x00001000,0x00000100,0x00111110,0x00000000,
	0x00110000,0x00001000,0x00001000,0x00000100,0x00001000,0x00001000,0x00110000,0x00000000,
	0x00001000,0x00001000,0x00001000,0x00000000,0x00001000,0x00001000,0x00001000,0x00000000,
	0x00000110,0x00001000,0x00001000,0x00010000,0x00001000,0x00001000,0x00000110,0x00000000,
	0x00000100,0x00101010,0x00010000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000010,0x00000100,0x00001000,0x00010000,0x00100000,0x01000000,0x00000000,

	0x11111111,0x21111111,0x22000011,0x22000011,0x22000011,0x22000011,0x22222211,0x22222221,
	0x11111111,0x21111111,0x22333311,0x22333311,0x22333311,0x22333311,0x22222211,0x22222221,
	0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x11111111,
	0x33333331,0x33333331,0x33333331,0x33333331,0x33333331,0x33333331,0x33333331,0x11111111,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111,
	0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,
	0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x11111111,
	0x00000001,0x00000010,0x00000100,0x00001000,0x00010000,0x00100000,0x01000000,0x10000000,

	0x00000001,0x00000010,0x00000100,0x00001000,0x00011000,0x00100100,0x01000010,0x10000001,
	0x10000001,0x01000010,0x00100100,0x00011000,0x00011000,0x00100100,0x01000010,0x10000001,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00001111,0x00001331,0x00001331,0x00001111,
	0x00000000,0x00000000,0x00000000,0x00000000,0x11111111,0x13311331,0x13311331,0x11111111,
	0x00001111,0x00001331,0x00001331,0x00001111,0x00001111,0x00001331,0x00001331,0x00001111,
	0x00001111,0x00001331,0x00001331,0x00001111,0x11110000,0x13310000,0x13310000,0x11110000,
	0x00001111,0x00001331,0x00001331,0x00001111,0x11111111,0x13311331,0x13311331,0x11111111,
	0x11111111,0x13311331,0x13311331,0x11111111,0x11111111,0x13311331,0x13311331,0x11111111,

	0x00000000,0x00111100,0x01000010,0x01000010,0x01000010,0x01000010,0x00111100,0x00000000,
	0x00000000,0x01000010,0x00100100,0x00011000,0x00011000,0x00100100,0x01000010,0x00000000,
	0x00000000,0x00011000,0x00011000,0x00100100,0x00100100,0x01000010,0x01111110,0x00000000,
	0x00000000,0x01111110,0x01000010,0x01000010,0x01000010,0x01000010,0x01111110,0x00000000,
	0x00000000,0x01111110,0x01000010,0x01011010,0x01000010,0x01011010,0x01011010,0x00000000,
	0x00000000,0x01111110,0x01100010,0x01011010,0x01100010,0x01011010,0x01100010,0x00000000,
	0x00000000,0x00001000,0x00001100,0x01111110,0x01111110,0x00001100,0x00001000,0x00000000,
	0x00000000,0x00011000,0x00011000,0x00011000,0x01111110,0x00111100,0x00011000,0x00000000,

	0x00000000,0x01100000,0x01110000,0x00111010,0x00011110,0x00001110,0x00011110,0x00000000,
	0x00000000,0x00033000,0x00033000,0x03333330,0x00333300,0x00333300,0x03300330,0x00000000,
	0x00000000,0x00000000,0x00000000,0x11111111,0x11111111,0x00000000,0x00000000,0x00000000,
	0x00011000,0x00011000,0x00011000,0x00011000,0x00011000,0x00011000,0x00011000,0x00011000,
	0x00000000,0x00000000,0x00000000,0x00001111,0x00011111,0x00011000,0x00011000,0x00011000,
	0x00000000,0x00000000,0x00000000,0x11111111,0x11111111,0x00011000,0x00011000,0x00011000,
	0x00011000,0x00011000,0x00011000,0x00011111,0x00011111,0x00011000,0x00011000,0x00011000,
	0x00011000,0x00011000,0x00011000,0x11111111,0x11111111,0x00011000,0x00011000,0x00011000,

	0x00000000,0x00011000,0x00111100,0x01111110,0x01111110,0x00011000,0x00111100,0x00000000,
	0x00000000,0x00011000,0x00011000,0x01100110,0x01100110,0x00011000,0x00111100,0x00000000,
	0x00000000,0x03300330,0x03333330,0x03333330,0x03333330,0x00333300,0x00033000,0x00000000,
	0x00000000,0x00033000,0x00333300,0x03333330,0x03333330,0x00333300,0x00033000,0x00000000,
	0x00000000,0x00000000,0x01111110,0x01133110,0x01133110,0x01111110,0x00000000,0x00000000,
	0x00000000,0x00000000,0x03331330,0x03331330,0x01111110,0x03331330,0x00000000,0x00000000,
	0x00000000,0x00000000,0x02211330,0x02211330,0x02211330,0x02211330,0x00000000,0x00000000,
	0x00000000,0x00000000,0x03333330,0x01111110,0x01111110,0x02222220,0x00000000,0x00000000,

	0x00000000,0x00111100,0x01111110,0x01111110,0x01111110,0x01111110,0x00111100,0x00000000,
	0x00000000,0x00333300,0x03333330,0x03333330,0x03333330,0x03333330,0x00333300,0x00000000,
	0x01111111,0x01111111,0x01133311,0x01133311,0x01133311,0x01111111,0x01111111,0x00000000,
	0x01111111,0x01111121,0x01111111,0x01111111,0x01111111,0x01211111,0x01111111,0x00000000,
	0x01111111,0x01111121,0x01111111,0x01112111,0x01111111,0x01211111,0x01111111,0x00000000,
	0x01111111,0x01211121,0x01111111,0x01111111,0x01111111,0x01211121,0x01111111,0x00000000,
	0x01111111,0x01211121,0x01111111,0x01112111,0x01111111,0x01211121,0x01111111,0x00000000,
	0x01111111,0x01211121,0x01111111,0x01211121,0x01111111,0x01211121,0x01111111,0x00000000,

	0x01111111,0x01000000,0x01001000,0x00101000,0x00001000,0x00000100,0x00000010,0x00000000,
	0x00100000,0x00100000,0x00010000,0x00001100,0x00001011,0x00001000,0x00001000,0x00000000,
	0x00001000,0x01111111,0x01000001,0x01000001,0x01000000,0x00100000,0x00011100,0x00000000,
	0x00000000,0x00111110,0x00001000,0x00001000,0x00001000,0x00001000,0x01111111,0x00000000,
	0x00010000,0x01111111,0x00010000,0x00011000,0x00010100,0x00010010,0x00010001,0x00000000,
	0x00000100,0x00111111,0x00100100,0x00100100,0x00100010,0x00100010,0x00011001,0x00000000,
	0x00001000,0x01111111,0x00001000,0x00001000,0x01111111,0x00001000,0x00001000,0x00000000,
	0x00000010,0x00111110,0x00100010,0x00100001,0x00100000,0x00010000,0x00001110,0x00000000,

	0x00000010,0x01111110,0x00010010,0x00010001,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00000000,0x00111111,0x00100000,0x00100000,0x00100000,0x00100000,0x00111111,0x00000000,
	0x00100010,0x01111111,0x00100010,0x00100010,0x00100000,0x00010000,0x00001110,0x00000000,
	0x00000011,0x00000100,0x01000011,0x01000100,0x00100000,0x00010000,0x00001111,0x00000000,
	0x00111111,0x00100000,0x00010000,0x00001000,0x00001100,0x00010010,0x00100001,0x00000000,
	0x00000010,0x00111111,0x00100010,0x00010010,0x00000010,0x00000010,0x00111100,0x00000000,
	0x00100001,0x00100001,0x00100010,0x00100010,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00000000,0x00111111,0x00100001,0x00100001,0x00100000,0x00010000,0x00001100,0x00000000,

	0x00111110,0x00100010,0x00100110,0x00111001,0x00100000,0x00010000,0x00001110,0x00000000,
	0x00111000,0x00001110,0x00001000,0x01111111,0x00001000,0x00000100,0x00000011,0x00000000,
	0x01000101,0x01001010,0x01001010,0x00100000,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00111110,0x00000000,0x01111111,0x00001000,0x00001000,0x00000100,0x00000011,0x00000000,
	0x00000001,0x00000001,0x00000001,0x00000111,0x00011001,0x00000001,0x00000001,0x00000000,
	0x00001000,0x00001000,0x00111111,0x00001000,0x00001000,0x00000100,0x00000011,0x00000000,
	0x00000000,0x00011110,0x00000000,0x00000000,0x00000000,0x00000000,0x00111111,0x00000000,
	0x00111110,0x00100000,0x00100000,0x00010100,0x00001000,0x00010100,0x00100011,0x00000000,

	0x00001000,0x00111111,0x00010000,0x00011100,0x00101010,0x01001001,0x00001000,0x00000000,
	0x00000000,0x00100000,0x00100000,0x00010000,0x00001000,0x00000100,0x00000011,0x00000000,
	0x00000000,0x00001000,0x00010010,0x00010010,0x00100010,0x00100010,0x00100001,0x00000000,
	0x00000000,0x00000001,0x00110001,0x00001111,0x00000001,0x00000001,0x00111110,0x00000000,
	0x00000000,0x00111111,0x00100000,0x00100000,0x00100000,0x00010000,0x00001110,0x00000000,
	0x00000000,0x00000100,0x00001010,0x00010001,0x00100000,0x01000000,0x00000000,0x00000000,
	0x00001000,0x01111111,0x00001000,0x00101010,0x01001001,0x01001001,0x00001000,0x00000000,
	0x00000000,0x00111111,0x00100000,0x00111110,0x00100000,0x00010000,0x00001110,0x00000000,

	0x00000000,0x01111111,0x01000000,0x00100000,0x00010010,0x00001100,0x00001000,0x00000000,
	0x00001110,0x00010000,0x00000110,0x00011000,0x00000000,0x00000111,0x00111000,0x00000000,
	0x00001000,0x00001000,0x00000100,0x00100100,0x00100010,0x01110010,0x01001111,0x00000000,
	0x00100000,0x00100010,0x00010100,0x00001000,0x00010100,0x00100010,0x00000001,0x00000000,
	0x00111110,0x00000100,0x00000100,0x01111111,0x00000100,0x00000100,0x01111000,0x00000000,
	0x00011110,0x00000000,0x00111111,0x00100000,0x00100000,0x00010000,0x00001110,0x00000000,
	0x00010001,0x00010001,0x00010001,0x00010001,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00001000,0x00001010,0x00001010,0x00001010,0x01001010,0x00101010,0x00011001,0x00000000,

	0x00000001,0x00000001,0x00000001,0x00100001,0x00100001,0x00010001,0x00001111,0x00000000,
	0x00000000,0x00111111,0x00100001,0x00100001,0x00100001,0x00100001,0x00111111,0x00000000,
	0x00000100,0x00000100,0x01111111,0x01000100,0x00100100,0x00010100,0x00000100,0x00000000,
	0x00000000,0x00011110,0x00010000,0x00010000,0x00010000,0x00010000,0x01111111,0x00000000,
	0x00111111,0x00100000,0x00100000,0x00111111,0x00100000,0x00100000,0x00111111,0x00000000,
	0x00000000,0x00000010,0x00011111,0x00010010,0x00001010,0x00000010,0x00000010,0x00000000,
	0x00000000,0x00000000,0x00001110,0x00001000,0x00001000,0x00001000,0x00111111,0x00000000,
	0x00000000,0x00000000,0x00011110,0x00010000,0x00011110,0x00010000,0x00011110,0x00000000,

	0x00000000,0x00000000,0x00011111,0x00010000,0x00001100,0x00000100,0x00000010,0x00000000,
	0x00000000,0x00010000,0x00001000,0x00001100,0x00001010,0x00001000,0x00001000,0x00000000,
	0x00000000,0x00000100,0x00011111,0x00010001,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00000000,0x00000000,0x00011111,0x00000100,0x00000100,0x00000100,0x00011111,0x00000000,
	0x00000000,0x00001000,0x00011111,0x00001000,0x00001100,0x00001010,0x00001001,0x00000000,
	0x10100100,0x10111111,0x00100100,0x00100100,0x00100010,0x00100010,0x00011001,0x00000000,
	0x10101000,0x11111111,0x00001000,0x00001000,0x01111111,0x00001000,0x00001000,0x00000000,
	0x10100010,0x10111110,0x00100010,0x00100001,0x00100000,0x00010000,0x00001110,0x00000000,

	0x10100010,0x11111110,0x00010010,0x00010001,0x00010000,0x00001000,0x00000110,0x00000000,
	0x10100000,0x10111111,0x00100000,0x00100000,0x00100000,0x00100000,0x00111111,0x00000000,
	0x10100010,0x11111111,0x00100010,0x00100010,0x00100000,0x00010000,0x00001110,0x00000000,
	0x10100011,0x10100100,0x01000011,0x01000100,0x00100000,0x00010000,0x00001111,0x00000000,
	0x10111111,0x10100000,0x00010000,0x00001000,0x00001100,0x00010010,0x00100001,0x00000000,
	0x10100010,0x10111111,0x00100010,0x00010010,0x00000010,0x00000010,0x00111100,0x00000000,
	0x10100001,0x10100001,0x00100010,0x00100010,0x00010000,0x00001000,0x00000110,0x00000000,
	0x00000000,0x00000000,0x00010101,0x00010101,0x00010000,0x00001000,0x00000110,0x00000000,

	0x10111110,0x10100010,0x00100110,0x00111001,0x00100000,0x00010000,0x00001110,0x00000000,
	0x10111000,0x10101110,0x00001000,0x01111111,0x00001000,0x00000100,0x00000011,0x00000000,
	0x10100101,0x10101010,0x01001010,0x00100000,0x00010000,0x00001000,0x00000110,0x00000000,
	0x10111110,0x10100000,0x01111111,0x00001000,0x00001000,0x00000100,0x00000011,0x00000000,
	0x00000001,0x10100001,0x10100001,0x00000111,0x00011001,0x00000001,0x00000001,0x00000000,
	0x11000000,0x11001000,0x00010010,0x00010010,0x00100010,0x00100010,0x00100001,0x00000000,
	0x11000000,0x11000001,0x00110001,0x00001111,0x00000001,0x00000001,0x00111110,0x00000000,
	0x11000000,0x11111111,0x00100000,0x00100000,0x00100000,0x00010000,0x00001110,0x00000000,

	0x11000000,0x11000100,0x00001010,0x00010001,0x00100000,0x01000000,0x00000000,0x00000000,
	0x11001000,0x11111111,0x00001000,0x00101010,0x01001001,0x01001001,0x00001000,0x00000000,
	0x10100000,0x10101000,0x00010010,0x00010010,0x00100010,0x00100010,0x00100001,0x00000000,
	0x10100000,0x10100001,0x00110001,0x00001111,0x00000001,0x00000001,0x00111110,0x00000000,
	0x10100000,0x10111111,0x00100000,0x00100000,0x00100000,0x00010000,0x00001110,0x00000000,
	0x10100000,0x10100100,0x00001010,0x00010001,0x00100000,0x01000000,0x00000000,0x00000000,
	0x10101000,0x10111111,0x00001000,0x00101010,0x01001001,0x01001001,0x00001000,0x00000000,
	0x00000000,0x00000001,0x00100010,0x00100000,0x00010000,0x00001000,0x00000111,0x00000000,
};

const unsigned short chr003Pal[256] __attribute__((aligned(4))) __attribute__((visibility("hidden")))=
{
	0x0000,0x77BD,0x35AD,0x5294,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x77BD,0x35AD,0x29FB,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0400,0x0401,0x0421,0x0421,
	0x0421,0x77BD,0x35AD,0x4B09,0x0421,0x0421,0x0421,0x0421,
	0x0421,0x0421,0x0421,0x0421,0x0421,0x0421,0x0421,0x0421,
	0x0421,0x77BD,0x35AD,0x66C9,0x0421,0x0421,0x0421,0x0421,
	0x0421,0x0421,0x0821,0x0822,0x0842,0x0842,0x0842,0x0842,

	0x0842,0x294A,0x35AD,0x5294,0x0842,0x0842,0x0842,0x0842,
	0x0842,0x0842,0x0842,0x0842,0x0842,0x0842,0x0842,0x0842,
	0x0842,0x5B1E,0x35AD,0x5294,0x0842,0x0842,0x0842,0x0842,
	0x0C42,0x0C43,0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,
	0x0C63,0x6395,0x35AD,0x5294,0x0C63,0x0C63,0x0C63,0x0C63,
	0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,0x0C63,
	0x0C63,0x7375,0x35AD,0x5294,0x0C63,0x0C63,0x1063,0x1064,
	0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,

	0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,
	0x1084,0x1084,0x1084,0x1084,0x1084,0x1084,0x1484,0x1485,
	0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,
	0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,
	0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x14A5,0x18A5,0x18A6,
	0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,
	0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,
	0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x18C6,0x1CC6,0x1CC7,

	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
	0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,0x1CE7,
};

//}}BLOCK(chr003)