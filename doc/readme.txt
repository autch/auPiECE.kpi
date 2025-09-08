
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              Third Party Programs
//
//
// PIECE LIBRARY : auPiECE.kpi : Ver 1.00
//
// Copyright (C)2001 AQUAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
// Copyright (c)2003 Yui N.
//
// Coded by MIO.H (OeRSTED) / Yui N. (Autch.net)
//
// Comments:
//
// PIECE 標準音楽ドライバーの KbMedia Player 移植版
//
// 取扱説明書
//
//  v1.00 2003.02.22 Yui N.
//

P/ECE -mobile equipment- の標準音楽ドライバが使用する pmd 形式ファイルを
KbMedia Player で再生させるためのプラグインです。対応する拡張子は pmd です。

P/ECE muslib のソースコードをほとんどそのまま使用していますので，再現度は 100%
であるはずです。P/ECE 開発環境添付の pm.exe と比べると音質が劣化しているように
聞こえますが，pm.exe は標準 muslib よりも高いサンプリングレートで音を生成してい
るためのようです。

pmd データが標準音色セットにない音色を要求している場合は再生できません。

コンパイルに成功しても無効なデータは，実機および pm.exe と同様にクラッシュする
ことがあります（ぉ

更新履歴
	Version 1.00	初版

ライセンスについて
	P/ECE デフォルトのフリーソフトの扱いに準じます。

	ソースコードは私のサイトから入手可能なはずですが，万一無かったりまたは
	古い場合には，メールをくだされば最新版のソースをお送りします。
	これは別にソースの扱いがどうということではなくて，単に面倒くさがりな
	だけです（ぉ

----------------------------------------
Yui N. (a.k.a. Autch)
yui@autch.net
http://www.autch.net/
