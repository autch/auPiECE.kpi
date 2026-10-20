このディレクトリは Kobarin 氏の KbMedia Player プラグイン SDK (kpisdk.7z、
2017/03/06 版) のうち、デコーダプラグインのビルドに必要なヘッダのコピーです。
原本は KbMedia Player 公式サイトから入手できます。

文字コードを CP932 (Shift_JIS) から UTF-8 に変換していますが、内容は変更して
いません (MSVC は /utf-8 でビルドします)。

SDK を更新するときは、新しい kpisdk.7z を展開してから、このディレクトリで

  for f in kpi.h kpi_decoder.h kpi_impl.h kmp_pi.h; do iconv -f CP932 -t UTF-8 <展開先>/$f > $f; done

として変換し直してください。
