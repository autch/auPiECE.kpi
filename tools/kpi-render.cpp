// kpi-render - auPiECE.kpi をロードして pmd を raw PCM に書き出す (検証用ホスト)
//
// usage: kpi-render [-k PLUGIN] [-r RATE] [-n MAXBLOCKS] file.pmd > out.raw
//        kpi-render [-k PLUGIN] [-r RATE] --seek-check file.pmd...
//
// 前者は libmuslib の muslib-render と同じ形式で出力するので、libmuslib の
// tools/golden.mjs の --cli にそのまま渡せる。
//   stdout: int16 little-endian mono
//   stderr: "blocks=N finished=0|1"
// --seek-check は、いろいろな位置にシークしてから半端な長さで Render() した結果が
// 先頭から続けて Render() した結果と一致するかを確かめる。
// PLUGIN の既定値は kpi-render.exe と同じディレクトリの auPiECE.kpi。

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <vector>

#include "kpi.h"
#include "kpi_decoder.h"
#include "kpi_impl.h"
#include "kpi_mingw_uuid.h"
#include "muslib.h"

namespace
{
    class MemoryFile : public KbKpiFileImpl
    {
    public:
        explicit MemoryFile(std::vector<BYTE> data) : data_(std::move(data)) {}

        DWORD WINAPI Read(void* pBuffer, DWORD dwSize) override
        {
            if (pos_ >= data_.size())
                return 0;
            const DWORD n = static_cast<DWORD>(std::min<UINT64>(dwSize, data_.size() - pos_));
            std::memcpy(pBuffer, data_.data() + pos_, n);
            pos_ += n;
            return n;
        }
        UINT64 WINAPI Seek(INT64 i64Pos, DWORD dwOrigin) override
        {
            const INT64 base = dwOrigin == FILE_BEGIN ? 0 : dwOrigin == FILE_CURRENT ? static_cast<INT64>(pos_) : static_cast<INT64>(data_.size());
            if (base + i64Pos < 0 || static_cast<UINT64>(base + i64Pos) > data_.size())
                return KPI_FILE_EOF;
            pos_ = static_cast<UINT64>(base + i64Pos);
            return pos_;
        }
        UINT64 WINAPI GetSize() override { return data_.size(); }

    private:
        std::vector<BYTE> data_;
        UINT64 pos_ = 0;
    };

    void usage()
    {
        std::fputs("usage: kpi-render [-k PLUGIN] [-r RATE] [-n MAXBLOCKS] file.pmd > out.raw\n"
                   "       kpi-render [-k PLUGIN] [-r RATE] --seek-check file.pmd...\n", stderr);
        std::exit(2);
    }

    bool readFile(const wchar_t* path, std::vector<BYTE>& data)
    {
        FILE* fp = _wfopen(path, L"rb");
        if (!fp)
            return false;
        BYTE buf[4096];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), fp)) > 0)
            data.insert(data.end(), buf, buf + n);
        std::fclose(fp);
        return true;
    }

    // Open + Select まで。失敗したら nullptr
    IKpiDecoder* openDecoder(IKpiDecoderModule* module, const wchar_t* path, DWORD rate)
    {
        std::vector<BYTE> data;
        if (!readFile(path, data))
        {
            std::fwprintf(stderr, L"%ls: cannot read\n", path);
            return nullptr;
        }
        auto* file = new MemoryFile(std::move(data));

        KPI_MEDIAINFO request;
        kpi_InitMediaInfo(&request);
        request.dwSampleRate = rate;
        IKpiDecoder* decoder = nullptr;
        const DWORD count = module->Open(&request, file, nullptr, &decoder);
        file->Release();
        if (count == 0 || decoder == nullptr)
        {
            std::fwprintf(stderr, L"%ls: cannot open\n", path);
            return nullptr;
        }

        const KPI_MEDIAINFO* info = nullptr;
        if (decoder->Select(1, &info, nullptr, 0) != 1 || info == nullptr)
        {
            std::fwprintf(stderr, L"%ls: cannot select\n", path);
            decoder->Release();
            return nullptr;
        }
        if (info->dwSampleRate != rate || info->dwChannels != 1 || info->nBitsPerSample != 16)
        {
            std::fwprintf(stderr, L"%ls: unexpected format %u Hz %u ch %d bit\n", path,
                info->dwSampleRate, info->dwChannels, info->nBitsPerSample);
            decoder->Release();
            return nullptr;
        }
        return decoder;
    }

    int render(IKpiDecoderModule* module, const wchar_t* path, DWORD rate, int maxblocks)
    {
        IKpiDecoder* decoder = openDecoder(module, path, rate);
        if (!decoder)
            return 1;

        _setmode(_fileno(stdout), _O_BINARY);
        int16_t buf[MUSLIB_BLOCK_SAMPLES];
        int blocks = 0;
        bool finished = false;
        decoder->Seek(0, KPI_MEDIAINFO::SEEK_FLAGS_SAMPLE);
        while (blocks < maxblocks)
        {
            const DWORD n = decoder->Render(reinterpret_cast<BYTE*>(buf), MUSLIB_BLOCK_SAMPLES);
            if (n < MUSLIB_BLOCK_SAMPLES)
            {
                // muslib は 128 サンプル単位で終わるので、端数が出たらおかしい
                if (n != 0)
                    std::fwprintf(stderr, L"%ls: partial block (%u samples)\n", path, n);
                finished = true;
                break;
            }
            std::fwrite(buf, sizeof(int16_t), MUSLIB_BLOCK_SAMPLES, stdout);  // x86/x64 はリトルエンディアン
            blocks++;
        }
        // 最大ブロック数ちょうどで終わる曲は、もう 1 回 Render() して確かめる
        if (!finished && decoder->Render(reinterpret_cast<BYTE*>(buf), MUSLIB_BLOCK_SAMPLES) == 0)
            finished = true;
        std::fflush(stdout);

        std::fprintf(stderr, "blocks=%d finished=%d\n", blocks, finished ? 1 : 0);
        decoder->Release();
        return 0;
    }

    int seekCheck(IKpiDecoderModule* module, const wchar_t* path, DWORD rate)
    {
        constexpr DWORD REF_SAMPLES = MUSLIB_BLOCK_SAMPLES * 3000;
        IKpiDecoder* decoder = openDecoder(module, path, rate);
        if (!decoder)
            return 1;

        std::vector<int16_t> ref(REF_SAMPLES);
        decoder->Seek(0, KPI_MEDIAINFO::SEEK_FLAGS_SAMPLE);
        const DWORD total = decoder->Render(reinterpret_cast<BYTE*>(ref.data()), REF_SAMPLES);
        ref.resize(total);
        const bool finished = total < REF_SAMPLES;

        int fails = 0;
        const UINT64 positions[] = { 0, 1, 127, 128, 129, 1000, 54321, MUSLIB_BLOCK_SAMPLES * 2000 + 77,
                                     total, static_cast<UINT64>(total) + 500 };
        for (const UINT64 pos : positions)
        {
            // 終わる曲は終わった位置より先には進まない
            const UINT64 expect = (finished && pos > total) ? total : pos;
            const UINT64 got = decoder->Seek(pos, KPI_MEDIAINFO::SEEK_FLAGS_SAMPLE);
            if (got != expect)
            {
                std::fwprintf(stderr, L"%ls: Seek(%llu) returned %llu, expected %llu\n", path, pos, got, expect);
                fails++;
                continue;
            }

            // 半端な長さで Render() して、基準の続きと比べる
            static constexpr DWORD sizes[] = { 1, 333, 128, 7, 4096, 50 };
            std::vector<int16_t> out;
            for (size_t k = 0; got + out.size() < ref.size(); k++)
            {
                std::vector<int16_t> buf(sizes[k % std::size(sizes)]);
                const DWORD n = decoder->Render(reinterpret_cast<BYTE*>(buf.data()), static_cast<DWORD>(buf.size()));
                out.insert(out.end(), buf.begin(), buf.begin() + n);
                if (n < buf.size())
                    break;
            }
            const size_t compare = got < ref.size() ? ref.size() - static_cast<size_t>(got) : 0;
            if (out.size() < compare || std::memcmp(out.data(), ref.data() + got, compare * sizeof(int16_t)) != 0)
            {
                std::fwprintf(stderr, L"%ls: PCM after Seek(%llu) differs\n", path, pos);
                fails++;
            }
        }
        std::fwprintf(stderr, L"%ls: %ls (%u samples%ls)\n", path, fails ? L"NG" : L"ok", total, finished ? L", finished" : L"");
        decoder->Release();
        return fails ? 1 : 0;
    }

    std::wstring defaultPluginPath()
    {
        wchar_t path[MAX_PATH];
        const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring s(path, len);
        return s.substr(0, s.find_last_of(L"\\/") + 1) + L"auPiECE.kpi";
    }
}

int wmain(int argc, wchar_t** argv)
{
    std::wstring plugin = defaultPluginPath();
    DWORD rate = 44100;
    int maxblocks = 10000;
    bool seek_check = false;
    std::vector<const wchar_t*> files;

    for (int i = 1; i < argc; i++)
    {
        if (!std::wcscmp(argv[i], L"-k") && i + 1 < argc) plugin = argv[++i];
        else if (!std::wcscmp(argv[i], L"-r") && i + 1 < argc) rate = static_cast<DWORD>(_wtoi(argv[++i]));
        else if (!std::wcscmp(argv[i], L"-n") && i + 1 < argc) maxblocks = _wtoi(argv[++i]);
        else if (!std::wcscmp(argv[i], L"--seek-check")) seek_check = true;
        else if (argv[i][0] != L'-') files.push_back(argv[i]);
        else usage();
    }
    if (files.empty() || (!seek_check && files.size() != 1) || rate == 0 || maxblocks < 0)
        usage();

    HMODULE dll = LoadLibraryW(plugin.c_str());
    const auto create = dll ? reinterpret_cast<pfn_kpiCreateInstance>(GetProcAddress(dll, "kpi_CreateInstance")) : nullptr;
    IKpiDecoderModule* module = nullptr;
    if (!create || create(IID_IKpiDecoderModule, reinterpret_cast<void**>(&module), nullptr) != S_OK)
    {
        std::fwprintf(stderr, L"%ls: cannot load plugin\n", plugin.c_str());
        return 1;
    }

    int ret = 0;
    if (seek_check)
    {
        for (const wchar_t* f : files)
            ret |= seekCheck(module, f, rate);
    }
    else
        ret = render(module, files[0], rate, maxblocks);

    module->Release();
    FreeLibrary(dll);
    return ret;
}
