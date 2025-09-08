///////////////////////////////////////////////////////////
// auPiECE.kpi
// muslib <--> kpi 間レイヤー
// by Yui N. / S. Kino.
//
// v1.00	2003.2.21		Yui N.		初版
// v1.01	2003.2.23		S. Kino.	可変サンプリングレート対応
//															（Kb の再生周波数要求に適応するわけではない）
// v1.02			〃				Yui N,		ループしない曲で，演奏終了を検出できるようにした
//															（しかしまだ曲長計算は実装されていない）
// v1.03	2003.2.26		Yui N.		Kb の再生周波数要求に応じるようになった
// v1.04	2003.3.07		Yui N.		簡単なフォーマットチェックを行うようにした
// v1.05	2003.3.14		Yui N.		SOUNDINFO::dwUnitRender の倍数渡しが行われたら
//															そこで演奏終了するようにした（ぉ
// v1.06	2003.3.24		Yui N.		のこも Decoder でデコードできないバグを退治
// v1.07	2003.5.03		Yui N.		ヴィブラートとデチューンが初期化されない仕様を改良
// v1.08  2003.8.03   Yui N., Kobarin
//															コンパイラを Visual C++ .NET 2003 に移行
//															Kobarin さんによる re-entrant 化パッチを取り込み
//															Kernel32.dll にだけ依存するようにした
// v1.09  2003.08.07  Yui N.		ヴィブラートとエンヴェロープの調整が正しくなかった
// v1.10  2003.09.05  Yui N.		SOUNDINFO::dwUnitRender の倍数渡しに対応
//															シーク速度が向上
///////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

// muslib の仕様によって定まる、一度に render() 出来るバッファサイズ
#define RENDERSIZE (128 * sizeof(unsigned short))

#if 0	// VS.NET だと不要 *******************************************

//msvcrt.dll がなくても動作するようにする
//double 等を使うと浮動小数点関係のリンカエラーが発生してしまうので、
//浮動小数点関係はすべて ASM 化する
//こんなことしなくても回避する方法があるのかもしれないけど…。
//このために修正したのは
//#if(n)def NOUSE_MSVCRT
//の部分
//  ・mus.c::SetEnvSpeed2
//  ・mus.c::SetVibSpeed2
//  ・winlayer.cpp::AuPIECEKPI::Open
//  ・context.cpp の conv_num の値のコピー方法
//
//ASM 化は高速化のためにやってるわけではないです
//msvcrt.dll に依存しないようにしたからといって何が変わるわけでもないけど（ぉ

#endif //***********************************************************

/* yui: VS.NET だと不要**************************
//msvcrt.dll::malloc/free が使われないようにする

	void *operator new(size_t Size)
	{
		return ::GlobalAlloc(GMEM_FIXED, Size);
	}
	void operator delete(void *p)
	{
		if(p){ ::GlobalFree(p); }
	}
**************************************************/

static BOOL kbPathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt)
{//拡張子を置き換える
 //汎用的に使うわけではないので、ここでは
 //あまり厳密なチェックはしない
	char *p = pszPath;
	char *ext = NULL;

	while(*p)
	{
		if(IsDBCSLeadByte((BYTE)*p))
			p += 2;
		else
		{
			if(*p == '.')
				ext = p;
			else if(*p == '\\')
				ext = NULL;
			p++;
		}
	}
	if(ext)		
		lstrcpy(ext, pszExt);			//拡張子がある
	else
		lstrcat(pszPath, pszExt);		//拡張子がない

	return TRUE;
}

extern "C"
{
#include "muslib/musdef.h"
#include "muslib/musdefwin.h"
#include "muslib/muslib.h"
#include "kmp_pi.h"
#include "context.h"

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
	void *m_pContext;
public:
	AuPIECEKPI();
	~AuPIECEKPI();
	BOOL  Open(const char *cszFileName, SOUNDINFO *pInfo);
	DWORD Render(BYTE* Buffer, DWORD dwSize);
	DWORD SetPosition(DWORD dwPos);
	void  Close();

	//インライン関数化 by Kobarin
	void	SetpBuffer(signed short *pNew){m_pbuffer = pNew;}
	signed short *GetpBuffer(){return m_pbuffer;}
	DecoderStatus GetStatus(){return m_status;}
	//ここまで

	void Start();
	void Stop();
};

// 唯一のインスタンス
AuPIECEKPI *Decoder;
PCEWAVEINFO *pWave;

long FS, FS2, MCOUNT, FOFS;					// Yui: musdef.h から移してきたグローバル変数
static const int DEFAULT_FS = 44100;// Yui: ini にも Kb からも周波数指定がなかったときのデフォルト

HANDLE myhModule;
char INIfilename[1024];

// SK : muslib の可変周波数化に伴う、各種グローバル変数
extern long VOLS, ENVSTART, ENVSTOP, ENVRR;

extern unsigned char finish;	// mus.c

// P/ECE カーネル API エントリポイント
int pceWaveDataOut(int ch, PCEWAVEINFO *pwave)
{
	if(Decoder != NULL)
		Decoder->SetpBuffer((signed short *)pwave->pData);
	pWave = pwave;
	return 0;
}

// 以下，kpi エントリポイント
static BOOL g_bInit;

void WINAPI kpiInit(void)
{
	if(!g_bInit)
	{
		GetModuleFileName((HMODULE)myhModule, INIfilename, 1024);
		kbPathRenameExtension(INIfilename, ".ini");
		piece_Context_Init();
		g_bInit = TRUE;
	}
}

void WINAPI kpiDeinit(void)
{
	if(g_bInit)
	{
		piece_Context_Deinit();
		g_bInit = FALSE;
	}
}

HKMP WINAPI kpiOpen(const char *cszFileName, SOUNDINFO *pInfo)
{
	if(pInfo == NULL)	return NULL;

	AuPIECEKPI *Decoder = new AuPIECEKPI;
	if(Decoder->Open(cszFileName, pInfo))
		return Decoder;

	delete Decoder;
	return NULL;
}

// エントリポイントには記述していない
HKMP WINAPI kpiOpenFromBuffer(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo)
{
	return NULL;
}

void WINAPI kpiClose(HKMP hKMP)
{
	if(!hKMP)return;
	AuPIECEKPI *Decoder = (AuPIECEKPI*)hKMP;
	Decoder->Stop();
	Decoder->Close();
}

DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize)
{
	if(!hKMP)return 0;
	AuPIECEKPI *Decoder = (AuPIECEKPI*)hKMP;
	return Decoder->Render(Buffer, dwSize);
}

DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos)
{
	if(!hKMP)return 0;
	AuPIECEKPI *Decoder = (AuPIECEKPI*)hKMP;
	return Decoder->SetPosition(dwPos);
}

// 以下クラス定義

AuPIECEKPI::AuPIECEKPI()
{
	m_pseq = NULL;
	m_filename = NULL;
	m_pbuffer = NULL;
	m_status = emClosed;
	m_pContext = piece_Context_Create(this);
}

AuPIECEKPI::~AuPIECEKPI()
{
	Close();
	piece_Context_Delete(m_pContext);
}

void AuPIECEKPI::Start()
{
	if(m_pseq == NULL)	return;
	piece_Context_Lock(m_pContext);
	InitMusic();													// yui: 曲の再生時に detune と vibrato の初期化をする
	PlayMusic(m_pseq);
	m_status = emRunning;
	piece_Context_Unlock(m_pContext);
}

void AuPIECEKPI::Stop()
{
	piece_Context_Lock(m_pContext);
	StopMusic();
	piece_Context_Unlock(m_pContext);
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

BOOL AuPIECEKPI::Open(const char *cszFileName, SOUNDINFO *pInfo)
{
	HANDLE hFile;
	DWORD fsize, dwBytesRead;

	if(m_status != emClosed)
		return FALSE;

	piece_Context_Lock(m_pContext);
	// begin
		m_filename = new char[lstrlen(cszFileName) + 1];
		lstrcpy(m_filename, cszFileName);

		hFile = CreateFile(m_filename, GENERIC_READ, 0, NULL,
												OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			fsize = GetFileSize(hFile, NULL);

			m_pseq = new unsigned char[fsize + 1];
			ReadFile(hFile, m_pseq, fsize, &dwBytesRead, NULL);

			CloseHandle(hFile);
		}
		else
		{
			piece_Context_Unlock(m_pContext);
			return FALSE;
		}

		if(!GetPrivateProfileInt("auP/ECE", "DisableFormatCheck", 0, INIfilename))
		{
			// フォーマットチェック
			BYTE first, second;

			first = *m_pseq;	second = *(m_pseq + 1);
			if(first == 0x00)
			{
        if(second > MAXCH)
				{
					piece_Context_Unlock(m_pContext);
					return FALSE;
				}
			}
			else
        if(first > MAXCH)
				{
					piece_Context_Unlock(m_pContext);
					return FALSE;
				}
		}

		int defaultFS = GetPrivateProfileInt("auP/ECE", "SamplingFrequency", DEFAULT_FS, INIfilename);
		if(defaultFS <= 0)	defaultFS = DEFAULT_FS;

		// SK : muslib の可変周波数化に伴う、各種グローバル変数の初期化
			FS = pInfo->dwSamplesPerSec << 1;
			if(FS == 0)	FS = defaultFS << 1;

			FS2 = FS >> 1;
			//MCOUNT
			//(long)(FS2/128.0*2.5*16.0+0.5) == (long)((FS2*40)/128+0.5) == (long)(FS2*40.0+64.0)/128.0)
			//(FS2*40+64)/128
			MCOUNT = (FS2 * 40 + 64) >> 7;				// yui: 組み込み実数演算を利用

			//FOFS = (long)(auLog(FS2) / auLog(2.0) * 12.0 * 256.0 + 0.5)
			//     ==(long)(3072*auLog(FS2)/auLog(2.0)+1/2);
			FOFS = (long)(3072.0 * log((double)FS2) / log(2.0) + 0.5);

			VOLS = 1;

			// 2003.08.07 Yui N.
			// ENVSTART と ENVSTOP はレート値ではないので Freq. による調整は不要！
			ENVSTART = ((-4 * 12 * 256) << VOLS);
			ENVSTOP = ((-10 * 12 * 256) << VOLS);
			
			// ENVRR はその名のとおりレート値なので調整が必要
			// Freq. が上がったら増やすのではなくて減らす
			// X / (fs2 / 16k) = (X / 1) / (fs2 / 16k) = 16k * x / fs2 * 1
			//ENVRR = (double)(4000 << VOLS) / ((double)FS2 / 16000.0);
			ENVRR = MulDiv(4000 << VOLS, 16000, FS2);
		// SK : ここまで

		pInfo->dwSamplesPerSec = FS2;				//サンプリング周波数(44100, 22050 など)
		pInfo->dwChannels = 1;							//チャンネル数( mono = 1, stereo = 2)
		pInfo->dwBitsPerSample = 16;				//量子化ビット数( 8 or 16)
		pInfo->dwLength = 0xffffffff;				//曲の長さ（SPC のように計算不可能な場合は 0xFFFFFFFF）
		pInfo->dwSeekable = 0;							//シークをサポートしている場合は 1、しない場合は 0
		pInfo->dwUnitRender = RENDERSIZE * 32;					//Render 関数の第３引数はこの値の倍数が渡される（どんな値でも良い場合は 0）
		pInfo->dwReserved1 = 1;							//常に 0
		pInfo->dwReserved2 = 0;							//常に 0

		m_pbuffer = NULL;
		m_status = emOpened;

		Start();
		
	// end.
	piece_Context_Unlock(m_pContext);
	return TRUE;
}

DWORD AuPIECEKPI::Render(BYTE* Buffer, DWORD dwSize)
{
	DWORD dwBytesToRender = dwSize, dwBytesRendered = 0;

	piece_Context_Lock(m_pContext);
		// yui: SOUNDINFO::dwUnitRender の倍数が与えられたときに対応
		if(!finish)
		{
			do
			{
				if((Buffer != NULL) && (GetpBuffer() != NULL))
					CopyMemory(Buffer + dwBytesRendered, GetpBuffer(), RENDERSIZE);
				pWave->pfEndProc(pWave);
				dwBytesToRender -= RENDERSIZE;
				dwBytesRendered += RENDERSIZE;
			}while(dwBytesToRender > 0);
		}
	piece_Context_Unlock(m_pContext);

	return dwBytesRendered;
}

DWORD AuPIECEKPI::SetPosition(DWORD dwPos)
{
	Stop();
	Start();
	return 0;
}

}			// extern "C"

