#include <windows.h>

#include "kpi.h"
#include "kpi_decoder.h"

#include "KpiPmdDecoderModule.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
    if (dwReasonForCall == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(static_cast<HMODULE>(hModule));
    return TRUE;
}

extern "C" HRESULT WINAPI kpi_CreateInstance(REFIID riid, void** ppvObject, IKpiUnknown* pUnknown)
{
    if (IsEqualIID(riid, IID_IKpiDecoderModule))
    {
        const auto pModule = new KpiPmdDecoderModule();
        *ppvObject = static_cast<IKpiDecoderModule*>(pModule);
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}
