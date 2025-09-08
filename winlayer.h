#include "kmp_pi.h"

void WINAPI kpiInit(void);
void WINAPI kpiDeinit(void);
HKMP WINAPI kpiOpen(const char *cszFileName, SOUNDINFO *pInfo);
HKMP WINAPI kpiOpenFromBuffer(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo);
void WINAPI kpiClose(HKMP hKMP);
DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize);
DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos);
