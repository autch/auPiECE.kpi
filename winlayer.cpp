///////////////////////////////////////////////////////////
// auPiECE.kpi
// muslib <--> kpi 間レイヤー
// by Yui N. / S. Kino.
//
// v1.00	2003.2.21		Yui N.		初版
// v1.01	2003.2.23		S. Kino.	可変サンプリングレート対応
//															（Kb の再生周波数要求に適応するわけではない）
// v1.02			〃			Yui N,		ループしない曲で，演奏終了を検出できるようにした
//															（しかしまだ曲長計算は実装されていない）
// v1.03	2003.2.26		Yui N,		Kb の再生周波数要求に応じるようになった
// v1.04	2003.3.07		Yui N,		簡単なフォーマットチェックを行うようにした
///////////////////////////////////////////////////////////


// Yui: 2 度しか呼ばれない log() のためにこいつを使うのはいやじゃｗ
// #include <math.h> // SK : extern "C" の中では読み込めない
#include <shlwapi.h>

extern "C"
{
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "muslib/musdefwin.h"
#include "muslib/muslib.h"
#include "kmp_pi.h"

double auLog(double x);					// Yui: 自前 Log()

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
	BOOL Open(const char *cszFileName, SOUNDINFO *pInfo);
	void Close();
	void SetpBuffer(signed short *pNew);
	signed short *GetpBuffer();
	void Start();
	void Stop();
	DecoderStatus GetStatus();
};

class AuPPXDecoder
{
};

// 唯一のインスタンス
AuPIECEKPI *Decoder;
PCEWAVEINFO *pWave;

double conv_num;							// SK : 16KHz に対する設定周波数の割合（muslib も参照する）
long FS, FS2, MCOUNT, FOFS;		// Yui: musdef.h から移してきたグローバル変数
long defaultFS = 44100;				// Yui: ini にも Kb からも周波数指定がなかったときのデフォルト
															// オーバーライドされる可能性がある。
HANDLE myhModule;
char INIfilename[1024];
// SK : muslib の可変周波数化に伴う、各種グローバル変数
extern long VOLS, ENVSTART, ENVSTOP, ENVRR;

extern unsigned char finish;	// mus.c

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

	GetModuleFileName((HMODULE)myhModule, INIfilename, 1024);
	PathRenameExtension(INIfilename, ".ini");
}

void WINAPI kpiDeinit(void)
{
	if(Decoder != NULL)	delete Decoder;
}

HKMP WINAPI kpiOpen(const char *cszFileName, SOUNDINFO *pInfo)
{
	if(pInfo == NULL)	return NULL;
	if(Decoder->Open(cszFileName, pInfo) == TRUE)
		return Decoder;											// hKMP をまったく使わない超手抜き実装（ぉ
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
	if(hKMP == NULL)	return 0;
	//if(Decoder->GetStatus() == emOpened)	Decoder->Start();
	if(Decoder->GetpBuffer() == NULL) return 0;

	if(Buffer != NULL)	CopyMemory(Buffer, Decoder->GetpBuffer(), dwSize);
	pWave->pfEndProc(pWave);
	return (finish == 0) ? dwSize : 0;
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

BOOL AuPIECEKPI::Open(const char *cszFileName, SOUNDINFO *pInfo)
{
	HANDLE hFile;
	DWORD fsize, dwBytesRead;

	if(m_status != emClosed)
		return FALSE;

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
		return FALSE;

	{
		// フォーマットチェック
		BYTE first, second;

		first = *m_pseq;	second = *(m_pseq + 1);
		if(first == 0x00)
		{
			if(second > 0x0f)
				return FALSE;
		}
		else
			if(first > 0x0f)
				return FALSE;
	}

	defaultFS = GetPrivateProfileInt("auP/ECE", "SamplingFrequency", 44100, INIfilename);
	if(defaultFS <= 0)	defaultFS = 44100;
	// SK : muslib の可変周波数化に伴う、各種グローバル変数の初期化
	FS = pInfo->dwSamplesPerSec * 2;
	if(FS == 0) FS = defaultFS * 2;
	// FS = 88200;		// Yui: ini ファイルによる設定変更に対応させる
	FS2 = FS / 2;
	MCOUNT = (long)(FS2 / 128.0 * 2.5 * 16.0 + 0.5);
	FOFS = (long)(auLog(FS2) / auLog(2.0) * 12.0 * 256.0 + 0.5);		// Yui: 自前 Log()

	conv_num = 16000.0 / FS2;

	VOLS = 1;
	ENVSTART = (long)(((-4 * 12 * 256) << VOLS) * conv_num);
	ENVSTOP = (long)(((-10 * 12 * 256) << VOLS) * conv_num);
	ENVRR = (long)((4000 << VOLS) * conv_num);
	// SK : ここまで

	pInfo->dwSamplesPerSec = FS2;				//サンプリング周波数(44100, 22050 など)
	pInfo->dwChannels = 1;							//チャンネル数( mono = 1, stereo = 2)
	pInfo->dwBitsPerSample = 16;				//量子化ビット数( 8 or 16)
	pInfo->dwLength = 0xffffffff;				//曲の長さ（SPC のように計算不可能な場合は 0xFFFFFFFF）
	pInfo->dwSeekable = 0;							//シークをサポートしている場合は 1、しない場合は 0
	pInfo->dwUnitRender = 128 * sizeof(unsigned short);					//Render 関数の第３引数はこの値の倍数が渡される（どんな値でも良い場合は 0）
	pInfo->dwReserved1 = 0;							//常に 0
	pInfo->dwReserved2 = 0;							//常に 0
	Decoder->Start();

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

// Yui: 自家製 Log()
// source from: 平林雅英「ANSI C 言語辞典」
double auLog(double x)
{
	int n = 1;
	double a, r = 0, r2;
	const double LN2 = 0.69314718055994530941723212145818;

	if(x <= 0)	return 0;
	while(x >= 2)	{	x /= 2;	r += LN2;	}
	while(x < 1)	{	x *= 2;	r -= LN2;	}
	x = (x - 1) / (x + 1);
	a = 2 * x;	x = x * x;
	do
	{
		r2 = r;	r += a / n;
		a *= x;	n += 2;
	}while(r != r2);
	return r;
}

};			// extern "C"
