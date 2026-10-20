
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              Third Party Programs
//
//
// PIECE LIBRARY : auPiECE.kpi : Ver 2.00
//
// Copyright (C)2001 AQUAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
// Copyright (c)2003, 2026 Yui N. (a.k.a. Autch)
//
// Coded by MIO.H (OeRSTED) / Yui N. (Autch.net) / S. Kino. / Kobarin
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
//  v1.09 2003.08.03 Yui N.
//  v1.10 2003.08.07 Yui N.
//  v1.11 2003.09.05 Yui N.
//  v1.12 2026.09.02 Autch
//  v2.00 2026.10.20 Autch
//

P/ECE -mobile equipment- の標準音楽ドライバが使用する pmd 形式ファイルを
KbMedia Player で再生させるためのプラグインです。対応する拡張子は pmd です。

muslib は，P/ECE 標準音楽ライブラリ muslib のソースをライブラリとして整理し
直した libmuslib を使っています。

動作環境
	KbMedia Player の新しいプラグイン仕様 (kpi_CreateInstance) に対応した
	KbMedia Player が必要です。x64 版と x86 版があるので，KbMedia Player
	本体と同じほうを使ってください。

再生周波数
	KbMedia Player の設定の再生周波数で演奏します。指定がないときは 44100Hz
	です。上限は 96000Hz で，それより高い周波数を指定しても 96000Hz で演奏し
	ます。
	Version 1.x の auPiECE.ini (SamplingFrequency / DisableFormatCheck) は
	読まなくなりました。

タイトル
	pmd に埋め込まれたタイトルを「タイトル」に，サブタイトル (作曲者などの
	クレジットが入っていることが多い) を「アーティスト」に表示します。

演奏時間とシーク
	pmd には演奏時間の情報がないので，曲の長さは KbMedia Player の設定の
	デフォルトの曲の長さになります。最後まで演奏すると終わる曲は，そこで演
	奏を終えます。シークはサンプル単位で正確に行えます (曲の先頭から演奏し
	直すので，曲の後ろのほうへのシークは少し時間がかかります)。

pmd データが標準音色セットにない音色を要求しているときは，その音色を無視し
て直前の音色のまま演奏を続けます。データが壊れていても，pmd の外を読んでク
ラッシュすることはありません。

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
	Version 1.09	Kobarin さんの re-entrant 化パッチを取り込み
			クロスフェード時のパフォーマンスが向上
			コンパイラを Visual C++ .NET 2003 に変更し、
			kernel32.dll 以外には依存しないようにした
			Pentium Pro 以降用に最適化
	Version 1.10	可変周波数対応するときに、エンベロープ値の調整を
			誤っていたバグを修正（今頃気づくなよ）。
			100% 再現とか大嘘ぶっこいてましたすみません(;´Д｀)
	Version 1.11	SOUNDINFO::dwUnitRender の倍数渡しにちゃんと対応し、
			シーク速度が向上した。
			（KbMP 本体は倍数渡しをしないが、muslib 内部の実装に
			依存した小さすぎる値を返していたのでシークが遅かった）
	Version 1.12	ドラム音色のノイズ対策
			パート数の上限を mucc が出力しうる 26 まで引き上げた
			高音・ビブラート・ポルタメントの整数あふれを修正
			可変レートの精度を改善
			音量が上限を超えたときに波形が反転するのを修正
			ビルドを CMake に移行した
	Version 2.00	KbMedia Player の新しいプラグイン仕様で作り直した
			muslib を libmuslib に置き換えた
			  (グローバル変数をなくし，同時にいくつでも演奏できる)
			Unicode のファイル名とアーカイブ内の pmd に対応した
			pmd のタイトルを表示するようにした
			サンプル単位のシークに対応した
			パート数の上限を 32 に引き上げた (mucc-next の上限)
			存在しない音色番号は無視するようにした (以前はクラッシュ)
			フォーマットチェックで各パートの開始位置も確かめるようにした
			auPiECE.ini を読まなくなった

ライセンスについて
	このプログラムは，P/ECE 標準音楽ライブラリ muslib のソースを改変したも
	の (libmuslib) を使用しています。元にしたファイルは以下のとおりです
	(パスは P/ECE SDK のインストール先からの相対)。
		/usr/piece/sysdev/music/mus.c
		/usr/piece/sysdev/music/seq.c
		/usr/piece/sysdev/music/musdef.h
		/usr/piece/sysdev/music/exp12.c
		/usr/piece/sysdev/music/instdef.c
		/usr/piece/sysdev/music/wavetable.c
		/usr/piece/sysdev/music/wave/i_*.c
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
