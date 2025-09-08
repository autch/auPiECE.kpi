/* muslib <--> kpi 間レイヤー */

extern "C"
{
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "muslib/musdefwin.h"
#include "muslib/muslib.h"
#include "kmp_pi.h"

typedef enum
{
	emClosed, emOpened, emRunning
}DecoderStatus;

// クラス定義は最後に
class AuPIECEKPI
{
private:
	unsigned char *m_pseq;			// シーケンスデータ
	char *m_filename;						// ファイル名
	signed short *m_pbuffer;		// 次の Render() で使うバッファへのポインタ
	DecoderStatus m_status;
public:
	AuPIECEKPI();
	~AuPIECEKPI();
	BOOL Open(const char *cszFileName);
	void Close();
	void SetpBuffer(signed short *pNew);
	signed short *GetpBuffer();
	void Start();
	void Stop();
	DecoderStatus GetStatus();
};

// 唯一のインスタンス
AuPIECEKPI *Decoder;
PCEWAVEINFO *pWave;

// P/ECE カーネル API エントリポイント
int pceWaveDataOut(int ch, PCEWAVEINFO *pwave)
{
	if(Decoder != NULL /*&& Decoder->GetStatus() == emRunning*/)
	{
		Decoder->SetpBuffer((signed short *)pwave->pData);
	}
	pWave = pwave;
	return 0;
}

// 以下，kpi エントリポイント
void WINAPI kpiInit(void)
{
	Decoder = new AuPIECEKPI;
}

void WINAPI kpiDeinit(void)
{
	if(Decoder != NULL)	delete Decoder;
}

HKMP WINAPI kpiOpen(const char *cszFileName, SOUNDINFO *pInfo)
{
	if(pInfo == NULL)	return NULL;
	if(Decoder->Open(cszFileName) == TRUE)
	{
		pInfo->dwSamplesPerSec = 16000;			//サンプリング周波数(44100, 22050 など)
		pInfo->dwChannels = 1;							//チャンネル数( mono = 1, stereo = 2)
		pInfo->dwBitsPerSample = 16;				//量子化ビット数( 8 or 16)
		pInfo->dwLength = 0xffffffff;				//曲の長さ（SPC のように計算不可能な場合は 0xFFFFFFFF）
		pInfo->dwSeekable = 0;							//シークをサポートしている場合は 1、しない場合は 0
		pInfo->dwUnitRender = 128 * sizeof(unsigned short);					//Render 関数の第３引数はこの値の倍数が渡される（どんな値でも良い場合は 0）
		pInfo->dwReserved1 = 0;							//常に 0
		pInfo->dwReserved2 = 0;							//常に 0
		Decoder->Start();
		return Decoder;											// hKMP をまったく使わない超手抜き実装（ぉ
	}
	else
		return NULL;
}

// エントリポイントには記述していない
HKMP WINAPI kpiOpenFromBuffer(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo)
{
	return NULL;
}

void WINAPI kpiClose(HKMP hKMP)
{
	Decoder->Stop();
	Decoder->Close();
}

DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize)
{
	if(Buffer == NULL || hKMP == NULL)	return 0;
	//if(Decoder->GetStatus() == emOpened)	Decoder->Start();
	if(Decoder->GetpBuffer() == NULL) return 0;

	memcpy(Buffer, Decoder->GetpBuffer(), dwSize);
	pWave->pfEndProc(pWave);
	return dwSize;
}

DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos)
{
	if(hKMP == NULL)	return 0;
	Decoder->Stop();
	Decoder->Start();
	return 0;
}

// 以下クラス定義

AuPIECEKPI::AuPIECEKPI()
{
	m_pseq = NULL;
	m_filename = NULL;
	m_pbuffer = NULL;
	m_status = emClosed;
	InitMusic();
}

AuPIECEKPI::~AuPIECEKPI()
{
	Close();
}

void AuPIECEKPI::Start()
{
	if(m_pseq == NULL)	return;
	//if(m_status != emOpened) return;
	PlayMusic(m_pseq);
	m_status = emRunning;
}

void AuPIECEKPI::Stop()
{
	StopMusic();
	m_status = emOpened;
	m_pbuffer = NULL;
}

void AuPIECEKPI::Close()
{
	Stop();
	if(m_pseq != NULL)
	{
			delete [] m_pseq;
			m_pseq = NULL;
	}
	if(m_filename != NULL)
	{
		delete [] m_filename;
		m_filename = NULL;
	}
	m_status = emClosed;
}

BOOL AuPIECEKPI::Open(const char *cszFileName)
{
	FILE *fp;
	DWORD fsize;

	if(m_status != emClosed)
		return FALSE;

	m_filename = new char[strlen(cszFileName) + 1];
	strcpy(m_filename, cszFileName);

	fp = fopen(m_filename, "rb");
	{// ファイルサイズの取得
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);
	}
	m_pseq = new unsigned char[fsize + 1];
	fread(m_pseq, sizeof(unsigned char), fsize, fp);
	fclose(fp);

	m_pbuffer = NULL;
	m_status = emOpened;
	return TRUE;
}

void AuPIECEKPI::SetpBuffer(signed short *pNew)
{
	m_pbuffer = pNew;
}

signed short *AuPIECEKPI::GetpBuffer()
{
	return m_pbuffer;
}

DecoderStatus AuPIECEKPI::GetStatus()
{
	return m_status;
}

};			// extern "C"
