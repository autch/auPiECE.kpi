#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "kmp_pi.h"
#include "version.h"
#include "winlayer.h"

extern HANDLE myhModule;

/* DLL 用の main() */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			myhModule = hModule;
			DisableThreadLibraryCalls(hModule);
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

/* 唯一のエクスポート関数 */
KMPMODULE* WINAPI kmp_GetTestModule()
{
	static const char *Exts[] = {".pmd", NULL};

	static KMPMODULE kpiInfoBlock =
	{
		KMPMODULE_VERSION,				// DWORD dwVersion;
		AUPIECE_KPI_VERSION,			// DWORD dwPluginVersion;
		"Copyright (c) 2001 - 2003, AQUAPLUS Co., Ltd. / OeRSTED, Inc. / Yui N. / S. Kino. all rights reserved.  ",
															// const char	*pszCopyright;
		"P/ECE pmd sequence decoder plugin for KbMedia Player",
															// const char	*pszDescription;
		Exts,											// const char	**ppszSupportExts;
		0,												// DWORD dwReentrant;
		kpiInit,									// void (WINAPI *Init)(void);
		kpiDeinit,								// void (WINAPI *Deinit)(void);
		kpiOpen,									// HKMP (WINAPI *Open)(const char *cszFileName, SOUNDINFO *pInfo);
		NULL, //kpiOpenFromBuffer,				// HKMP (WINAPI *OpenFromBuffer)(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo);
		kpiClose,									// void (WINAPI *Close)(HKMP hKMP);
		kpiRender,								// DWORD (WINAPI *Render)(HKMP hKMP, BYTE* Buffer, DWORD dwSize);
		kpiSetPosition						// DWORD (WINAPI *SetPosition)(HKMP hKMP, DWORD dwPos);
	};
	return &kpiInfoBlock;
}

