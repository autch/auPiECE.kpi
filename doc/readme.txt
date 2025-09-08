
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              Third Party Programs
//
//
// PIECE LIBRARY : auPiECE.kpi : Ver 1.01
//
// Copyright (C)2001 AQUAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
// Copyright (c)2003 Yui N.
//
// Coded by MIO.H (OeRSTED) / Yui N. (Autch.net) / S. Kino.
//
// Comments:
//
// PIECE 標準音楽ドライバーの KbMedia Player 移植版
//
// 取扱説明書
//
//  v1.00 2003.02.22 Yui N.
//  v1.01 2003.02.23 Yui N.
//

P/ECE -mobile equipment- の標準音楽ドライバが使用する pmd 形式ファイルを
KbMedia Player で再生させるためのプラグインです。対応する拡張子は pmd です。

P/ECE muslib のソースコードをほとんどそのまま使用していますので，再現度は 100%
であるはずです。S. Kino. さんによる 44kHz 化パッチを当ててありますので，pm.exe
よりも綺麗に鳴るはずです。

再生周波数を任意に設定することができるようになりました。
auPiECE.kpi と同じディレクトリに，auPiECE.ini なるテキストファイルを作成し，
その中に以下のように書き込みます。
	[auP/ECE]
	SamplingFrequency=<再生周波数>
再生周波数のデフォルトは 44100 です。

pmd データが標準音色セットにない音色を要求している場合は再生できません。

コンパイルに成功しても無効なデータは，実機および pm.exe と同様にクラッシュする
ことがあります（ぉ

更新履歴
	Version 1.00	初版
	Version 1.01	S. Kino. さんによる可変周波数対応化パッチを取り込み
			コンパイラを Borland C++ Compiler 5.5.1 に変更
			C ランタイムライブラリを使わないようにした
			ini ファイルによる再生周波数指定に対応

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
