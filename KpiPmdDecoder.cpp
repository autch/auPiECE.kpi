#include "KpiPmdDecoder.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "kmp_pi.h"

// pmd のヘッダを見て、それらしいデータか確かめる (着メロなど拡張子が同じ別形式を避ける)。
// 先頭はパート数 (0 の次の 1 バイトがパート数の形式もある) とパートごとの開始位置の表。
bool KpiPmdDecoder::isThisPmd(const std::vector<uint8_t>& pmd)
{
    if (pmd.size() < 2)
        return false;

    size_t table = 1;
    unsigned parts = pmd[0];
    if (parts == 0)
    {
        table = 2;
        parts = pmd[1];
    }
    if (parts == 0 || parts > MUSLIB_MAX_PARTS)
        return false;
    if (table + parts * 2 > pmd.size())
        return false;

    for (unsigned i = 0; i < parts; i++)
    {
        const size_t adrs = pmd[table + i * 2] | (pmd[table + i * 2 + 1] << 8);
        if (adrs >= pmd.size())
            return false;
    }
    return true;
}

DWORD KpiPmdDecoder::Open(const KPI_MEDIAINFO* cpRequest, IKpiFile* pFile)
{
    if (cpRequest != nullptr && cpRequest->dwSampleRate != 0)
        request_rate_ = static_cast<int32_t>(std::min<DWORD>(cpRequest->dwSampleRate, MUSLIB_MAX_RATE));

    const UINT64 size = pFile->GetSize();
    if (size == 0 || size > MUSLIB_MAX_PMD_SIZE)
        return 0;

    pmd_.resize(static_cast<size_t>(size));
    pFile->Seek(0, FILE_BEGIN);
    if (pFile->Read(pmd_.data(), static_cast<DWORD>(size)) != size)
        return 0;

    if (!isThisPmd(pmd_))
        return 0;

    muslib_.reset(static_cast<muslib_t*>(std::malloc(muslib_size())));
    if (!muslib_)
        return 0;

    return 1;
}

void KpiPmdDecoder::fillMediaInfo(int32_t rate)
{
    kpi_InitMediaInfo(&mediainfo_);
    mediainfo_.dwNumber = 1;
    mediainfo_.dwCount = 1;
    mediainfo_.dwFormatType = KPI_MEDIAINFO::FORMAT_PCM;
    mediainfo_.dwSampleRate = static_cast<DWORD>(rate);
    mediainfo_.nBitsPerSample = 16;
    mediainfo_.dwChannels = 1;
    // 演奏時間は分からず、ループする曲は終わらない。終わる曲は Render() が途中で止まる
    mediainfo_.qwLength = static_cast<UINT64>(-1);
    mediainfo_.qwLoop = static_cast<UINT64>(-1);
    mediainfo_.dwUnitSample = 0;
    // 先頭から演奏し直して読み捨てるので、サンプル単位で正確にシークできる
    mediainfo_.dwSeekableFlags = KPI_MEDIAINFO::SEEK_FLAGS_SAMPLE
        | KPI_MEDIAINFO::SEEK_FLAGS_ACCURATE | KPI_MEDIAINFO::SEEK_FLAGS_ROUGH;
}

void KpiPmdDecoder::extractTags(IKpiTagInfo* pTagInfo, DWORD dwTagGetFlags) const
{
    // pmd のタイトルは Shift_JIS
    auto set_tag = [pTagInfo](const wchar_t* name, const char* value)
    {
        if (*value == '\0')
            return;
        const int len = MultiByteToWideChar(932, 0, value, -1, nullptr, 0);
        if (len <= 0)
            return;
        std::wstring w(static_cast<size_t>(len), L'\0');
        MultiByteToWideChar(932, 0, value, -1, w.data(), len);
        pTagInfo->wSetValueW(name, -1, w.c_str(), -1);
    };

    if (dwTagGetFlags & KPI_TAGGET_FLAG_TITLE)
        set_tag(SZ_KMP_NAME_TITLE_W, muslib_title(muslib_.get()));
    // title2 は作曲者・編曲者のクレジットが入っていることが多い
    if (dwTagGetFlags & KPI_TAGGET_FLAG_ARTIST)
        set_tag(SZ_KMP_NAME_ARTIST_W, muslib_title2(muslib_.get()));
}

DWORD KpiPmdDecoder::Select(DWORD dwNumber, const KPI_MEDIAINFO** ppMediaInfo, IKpiTagInfo* pTagInfo, DWORD dwTagGetFlags)
{
    if (ppMediaInfo != nullptr)
        *ppMediaInfo = nullptr;
    if (dwNumber != 1)
        return 0;

    const int32_t rate = muslib_init(muslib_.get(), request_rate_);
    if (rate == 0 || muslib_play(muslib_.get(), pmd_.data(), pmd_.size()) < 0)
        return 0;
    block_pos_ = MUSLIB_BLOCK_SAMPLES;

    fillMediaInfo(rate);
    if (ppMediaInfo != nullptr)
        *ppMediaInfo = &mediainfo_;
    if (pTagInfo != nullptr)
        extractTags(pTagInfo, dwTagGetFlags);

    return dwNumber;
}

bool KpiPmdDecoder::fillBlock()
{
    if (!muslib_render(muslib_.get(), block_))
        return false;
    block_pos_ = 0;
    return true;
}

UINT64 KpiPmdDecoder::Seek(UINT64 qwPosSample, DWORD dwFlag)
{
    if (muslib_play(muslib_.get(), pmd_.data(), pmd_.size()) < 0)
        return 0;
    block_pos_ = MUSLIB_BLOCK_SAMPLES;

    // 曲がそこまで続かなければ、終わった位置を返す
    const UINT64 whole_blocks = qwPosSample - qwPosSample % MUSLIB_BLOCK_SAMPLES;
    UINT64 pos = 0;
    for (; pos < whole_blocks; pos += MUSLIB_BLOCK_SAMPLES)
    {
        if (!muslib_render(muslib_.get(), block_))
            return pos;
    }

    const DWORD rest = static_cast<DWORD>(qwPosSample - whole_blocks);
    if (rest > 0 && fillBlock())
    {
        block_pos_ = rest;
        pos += rest;
    }
    return pos;
}

DWORD KpiPmdDecoder::Render(BYTE* pBuffer, DWORD dwSizeSample)
{
    DWORD rendered = 0;

    while (rendered < dwSizeSample)
    {
        if (block_pos_ == MUSLIB_BLOCK_SAMPLES && !fillBlock())
            break;  // 演奏終了

        const DWORD n = std::min<DWORD>(dwSizeSample - rendered, MUSLIB_BLOCK_SAMPLES - block_pos_);
        if (pBuffer != nullptr)
            std::memcpy(pBuffer + rendered * sizeof(int16_t), block_ + block_pos_, n * sizeof(int16_t));
        block_pos_ += n;
        rendered += n;
    }
    return rendered;
}
