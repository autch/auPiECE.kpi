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
// Copyright (c)2003, 2026 Yui N. (now Autch)
//
// Coded by MIO.H (OeRSTED) / Yui N. (now Autch) / S. Kino. / Kobarin
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

P/ECE -mobile equipment- の標準音楽ドライバ muslib が使用する pmd 形式フ
ァイルを KbMedia Player で再生させるためのプラグインです。対応する拡張子
は pmd です。

音源には、muslib のソースをライブラリとして整理し直した libmuslib を使っ
ています。

v2.00 から、KbMedia Player の新しいプラグイン仕様に対応しています。x64 
版と x86 版どちらにも対応します。

v2.00 から、再生中の曲の詳細情報に、pmd ファイルのパート数を 
PMD_NumParts として表示します。7 パート以上使っている曲には "(extended)" 
を付けます。そのようなデータは拙作 mucc-next で作成できます。KbMedia 
Player 本体のタグ読み込み機能ではパート拡張に対応していないので、このプ
ラグインが取り扱います。

再生周波数は本体が指定するレートで演奏します。指定がないときは 44100Hz 
です。上限は 96000Hz で、それより高い値を指定しても 96000Hz で演奏します。

曲の長さとフェードアウト時間も本体の設定に従います。

pmd データが標準音色セットにない音色を要求しているときは、その音色を無視
して直前の音色のまま演奏を続けます。


更新履歴
--------


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
Version 2.00	KbMedia Player の新しいプラグイン仕様で作り直し
		muslib を libmuslib に置き換え
		Unicode のファイル名とアーカイブ内の pmd に対応
		パート拡張した pmd のタイトルも表示するようにした
		パート数を詳細情報 (PMD_NumParts) に表示するようにした
		サンプル単位のシークに対応
		パート数の上限を 32 に引き上げ
		存在しない音色番号は無視するようにした
		フォーマットチェックで各パートの開始位置も確かめるようにした
		auPiECE.ini を読まなくなった



ライセンス
----------

このプログラムは、P/ECE 標準音楽ライブラリ muslib のソースを改変したもの 
(libmuslib) を使用しています。アクアプラスによる「付属ソースを元にしたプ
ログラムの再配布の扱いについて」に基づいたライセンス表記は 
LICENSE-PIECE.md に、それ以外については MIT ライセンスとします。

このプラグインの libmuslib 以外の部分も MIT ライセンスとします。



---
Autch
autch@autch.net
https://www.autch.net/
