# auPiECE.kpi

P/ECE の標準音楽ドライバ muslib の曲データ (pmd) を KbMedia Player で再生するデコーダプラグイン。
利用者向けの説明は [doc/readme.txt](doc/readme.txt) を参照。

Version 2.00 から KbMedia Player の新しいプラグイン仕様 (`kpi_CreateInstance`) で作り直し、
音源は [libmuslib](https://github.com/autch/libmuslib) (submodule) を使う。
旧仕様 (`kmp_GetTestModule`) の最終版は `v1.12` タグ。

## ソース構成

- `auPiECE.cpp` — `DllMain` と `kpi_CreateInstance`
- `KpiPmdDecoderModule.*` — `IKpiDecoderModule` (プラグイン情報、デコーダの生成)
- `KpiPmdDecoder.*` — `IKpiDecoder` (フォーマットチェック、タグ、Render、Seek)
- `kpisdk/` — KbMedia Player プラグイン SDK のヘッダ (UTF-8 に変換したコピー。`kpisdk/README.txt` 参照)
- `kpi_mingw_uuid.h` — MinGW でビルドするときの `__uuidof` 用シム (MSVC では空)
- `libmuslib/` — 音源 (submodule)
- `tools/kpi-render.cpp` — 検証用ホスト。ビルドした .kpi をロードして PCM を書き出す

## ビルド

リリースビルドは Windows の MSVC (Visual Studio 2022) で行う。

```
git clone --recursive https://github.com/autch/auPiECE.kpi.git
cd auPiECE.kpi
cmake -S . -B build-x64 -G "Visual Studio 17 2022" -A x64
cmake --build build-x64 --config Release
cmake -S . -B build-x86 -G "Visual Studio 17 2022" -A Win32
cmake --build build-x86 --config Release
```

`build-*/bin/Release/auPiECE.kpi` ができる。C/C++ ランタイムは静的リンク (`/MT`) なので、
VC++ 再頒布可能パッケージは要らない。

### MinGW (開発中の確認用)

Linux 上で MinGW でクロスビルドし、Wine で確かめることもできる。

```
cmake -S . -B build-mingw -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
cmake --build build-mingw
```

## 検証

`kpi-render` は libmuslib の `muslib-render` と同じ形式 (`-r RATE -n MAXBLOCKS file.pmd` で
int16le の raw PCM を stdout に、`blocks=N finished=0|1` を stderr に出す) なので、
libmuslib の golden とそのまま照合できる。MSVC (cl) には `-fwrapv` に当たるオプションが
ないので、MSVC でビルドしたものは必ず照合すること。

```
cd build-x64
ctest -C Release
```

- `golden-fixtures` — libmuslib 同梱の 14 曲を 16000 / 44100 / 48000Hz で golden と照合する (Node.js が要る)
- `seek-check` — いろいろな位置にシークしてから半端な長さで Render した結果が、
  先頭から続けて Render した結果と一致するかを確かめる

ほかのテストセット (libmuslib の `test/public` や手元のテストセット) も同じように照合できる。

```
cd libmuslib
node tools/golden.mjs check --set <テストセット> --pmd-dir <pmd のディレクトリ> --cli ../build-x64/bin/Release/kpi-render.exe
```

Wine で試すときは、`wine kpi-render.exe "$@"` を実行するシェルスクリプトを `--cli` に渡す。
