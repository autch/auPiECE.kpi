#include "KpiPmdDecoderModule.h"
#include "KpiPmdDecoder.h"
#include "version.h"

// {4FB93F76-177C-4238-B30F-6ACC75CE6628}
static constexpr GUID AUPIECE_GUID =
{ 0x4fb93f76, 0x177c, 0x4238, { 0xb3, 0x0f, 0x6a, 0xcc, 0x75, 0xce, 0x66, 0x28 } };

static constexpr KPI_DECODER_MODULEINFO decoder_info =
{
    .cb = sizeof(KPI_DECODER_MODULEINFO),
    .dwModuleVersion = KPI_DECODER_MODULE_VERSION,
    .dwPluginVersion = AUPIECE_KPI_VERSION,
    .dwMultipleInstance = KPI_MULTINST_INFINITE,
    .guid = AUPIECE_GUID,
    .cszDescription = L"P/ECE pmd sequence decoder plugin for KbMedia Player",
    .cszCopyright = L"Copyright (c) 2001 - 2026, AQUAPLUS Co., Ltd. / OeRSTED, Inc. / Yui N. / S. Kino. / Kobarin.  All rights reserved.",
    .cszSupportExts = L".pmd",
    .cszMultiSongExts = nullptr,
    .pvReserved1 = nullptr,
    .pvReserved2 = nullptr,
    .dwSupportTagInfo = KPI_DECODER_MODULEINFO::TAG_TITLE,
    .dwSupportConfig = 0,
    .dwReserved = { 0, 0, 0, 0 }
};

void KpiPmdDecoderModule::GetModuleInfo(const KPI_DECODER_MODULEINFO** ppInfo)
{
    *ppInfo = &decoder_info;
}

DWORD KpiPmdDecoderModule::Open(const KPI_MEDIAINFO* cpRequest, IKpiFile* pFile, IKpiFolder* pFolder, IKpiDecoder** ppDecoder)
{
    auto* decoder = new KpiPmdDecoder();
    const DWORD dwCount = decoder->Open(cpRequest, pFile);
    if (dwCount == 0)
    {
        decoder->Release();
        *ppDecoder = nullptr;
        return 0;
    }
    *ppDecoder = static_cast<IKpiDecoder*>(decoder);
    return dwCount;
}
