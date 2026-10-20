#pragma once

#include <windows.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

#include "kpi_decoder.h"
#include "kpi_impl.h"
#include "kpi_mingw_uuid.h"
#include "muslib.h"

class KpiPmdDecoder : public KbKpiDecoderImpl
{
public:
    DWORD Open(const KPI_MEDIAINFO* cpRequest, IKpiFile* pFile);

    DWORD  WINAPI Select(DWORD dwNumber,    // 曲番号
        const KPI_MEDIAINFO** ppMediaInfo,  // may be NULL
        IKpiTagInfo* pTagInfo,              // may be NULL
        DWORD dwTagGetFlags) override;      // どのフィールドのタグを取得するかのフラグ
    DWORD  WINAPI Render(BYTE* pBuffer, DWORD dwSizeSample) override;
    UINT64 WINAPI Seek(UINT64 qwPosSample, DWORD dwFlag) override;
    DWORD  WINAPI UpdateConfig(void* pvReserved) override { return 0; }

    static bool isThisPmd(const std::vector<uint8_t>& pmd);

private:
    // Kb から再生周波数の要求がないとき (1.x の auPiECE.ini の SamplingFrequency の既定値と同じ)
    static constexpr int32_t DEFAULT_SAMPLE_RATE = 44100;

    struct FreeDeleter { void operator()(void* p) const { std::free(p); } };

    std::unique_ptr<muslib_t, FreeDeleter> muslib_;
    std::vector<uint8_t> pmd_;
    int32_t request_rate_{ DEFAULT_SAMPLE_RATE };
    KPI_MEDIAINFO mediainfo_{};

    // muslib は 128 サンプル単位でしか出力しないので、Render() の端数はここに残す
    int16_t block_[MUSLIB_BLOCK_SAMPLES]{};
    DWORD block_pos_{ MUSLIB_BLOCK_SAMPLES };   // block_ の次に出すサンプル。MUSLIB_BLOCK_SAMPLES なら空

    bool fillBlock();
    void fillMediaInfo(int32_t rate);
    void extractTags(IKpiTagInfo* pTagInfo, DWORD dwTagGetFlags) const;
};
