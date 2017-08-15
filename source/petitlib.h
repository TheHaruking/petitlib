#ifndef _petitlib_h_
#define _petitlib_h_

/* ****************
 * 関数一覧
 * ****************
 */
// 必須関数
extern void init();
extern void finish();
extern void update();
// ファイル関数
extern void save();
extern void load();
// 画面基本設定関数
extern void xscreen();
extern void visible();
// コンソール制御関数
extern void linput();
extern void print(const char* str, ...);
// スプライト関数
// バックグラウンド画面関数
// 各種入出力
// 音関係
// 通信

/*
 * データ一覧
 */

// 画像データ
#define chr003TilesLen 8192
#define chr003PalLen 512
extern const unsigned int chr003Tiles[2048];
extern const unsigned short chr003Pal[256];

#endif