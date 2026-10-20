#pragma once

#include <windows.h>

#include "kpi_decoder.h"
#include "kpi_impl.h"
#include "kpi_mingw_uuid.h"

class KpiPmdDecoderModule : public KbKpiUnknownImpl<IKpiDecoderModule>
{
public:
    void  WINAPI GetModuleInfo(const KPI_DECODER_MODULEINFO** ppInfo) override;

    DWORD WINAPI Open(const KPI_MEDIAINFO* cpRequest,   // 再生周波数等の要求値 (タグ取得用なら NULL)
        IKpiFile* pFile,                                // 音楽データ
        IKpiFolder* pFolder,                            // 音楽データがあるフォルダ
        IKpiDecoder** ppDecoder) override;

    BOOL  WINAPI EnumConfig(IKpiConfigEnumerator* pEnumerator) override { return FALSE; }

    DWORD WINAPI ApplyConfig(const wchar_t* cszSection, // セクション名
        const wchar_t* cszKey,                          // キー名
        INT64  nValue,                                  // 変更後の値 (BOOL/INT の場合)
        double dValue,                                  // 変更後の値 (FLOAT の場合)
        const wchar_t* cszValue) override               // 変更後の値 (文字列)
    {
        return KPI_CFGRET_OK;
    }
};
