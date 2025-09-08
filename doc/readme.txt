
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              Third Party Programs
//
//
// PIECE LIBRARY : auPiECE.kpi : Ver 1.08
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
//  v1.02 2003.02.23 Yui N.
//  v1.03 2003.02.26 Yui N.
//  v1.04 2003.03.04 Yui N.
//  v1.05 2003.03.07 Yui N.
//  v1.06 2003.03.24 Yui N.
//  v1.07 2003.05.03 Yui N.
//  v1.08 2003.05.07 Yui N.
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
KbMedia Player からの再生周波数要求に応えるようになりました。上記 ini ファイル
の設定よりも KbMedia からの要求が優先されます。

pmd データが標準音色セットにない音色を要求している場合は再生できません。

コンパイルに成功しても無効なデータは，実機および pm.exe と同様にクラッシュする
ことがあります（ぉ

更新履歴
	Version 1.00	初版
	Version 1.01	S. Kino. さんによる可変周波数対応化パッチを取り込み
			コンパイラを Borland C++ Compiler 5.5.1 に変更
			C ランタイムライブラリを使わないようにした
			ini ファイルによる再生周波数指定に対応
	Version 1.02	ループしない曲の演奏終了を検出できるようになった
	Version 1.03	KbMedia Player 本体からの再生周波数要求に対応
	Version 1.04	簡単なフォーマットチェックを行うようにした
			（着メロの pmd ファイルとの競合を回避する）
	Version 1.05	SOUNDINFO::dwUnitRender の倍数渡しが行われたら
			そこで演奏終了するようにした（ぉ
	Version 1.06	のこも Decoder でデコードできないバグを退治
	Version 1.07	ヴィブラートとデチューンが初期化されない仕様を改良
	Version 1.08	はらやんさん（http://hara-yan-web.hp.infoseek.co.jp/）
			のプチノイズ対策を取り込み

ライセンスについて
	このプログラムは，P/ECE 標準音楽ライブラリ muslib のソースにパッチを当
	てたものが使用されています。
	パッチの当てられたファイルは以下のとおりです。
		/usr/piece/sysdev/music/mus.c
		/usr/piece/sysdev/music/seq.c
		/usr/piece/sysdev/music/musdef.h
	オリジナルの muslib ライブラリの著作権表示は以下のとおりです。
		Copyright (C)2001 AQUAPLUS Co., Ltd. / OeRSTED, Inc.
		all rights reserved.
		Coded by MIO.H (OeRSTED)
	これらの改変についてアクアプラスはなんら関係なく，プログラムの内容およ
	びそれの実行結果について，アクアプラスは一切の責任を負いません。このプ
	ログラムに関する一切の事柄についてアクアプラスにサポートを求めてはいけ
	ません。

	私およびアクアプラスは，このプログラムを使用したこと，または使用しな
	かった，使用できなかった事によって発生する，または発生する恐れのある一
	切の損害について，それらが発生することをあらかじめ知っていたか，あるい
	は知ることができたとしても，私およびアクアプラスは何らの法的および道義
	的責任を負いません。

	作者は，このプログラムに存在する，または存在する可能性のある，一切のバ
	グや欠陥について何ら修正する義務を負いません。使用者は，このプログラム
	の欠陥によって自らの財産等に損害が出る可能性があることを了承したうえで
	このプログラムを使用しなければなりません。

	これら以外については，P/ECE デフォルトのフリーソフトの扱いに準じます。

----------------------------------------
Yui N. (a.k.a. Autch)
yui@autch.net
http://www.autch.net/
