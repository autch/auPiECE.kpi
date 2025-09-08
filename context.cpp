//coded by Kobarin
//KMPMODULE::dwReentrant == 1 にする
//（クロスフェード時に kbrunkpi.exe を介さないようにする）
//グローバル変数群を piece_Context で保持
//非const なグローバル変数を追加した場合は
//  ・struct piece_Context にデータメンバを追加
//  ・piece_Context_Save を修正
//  ・piece_Context_Restore を修正
//  ・初期値が 0 でない（ゼロクリアされていない）変数がある場合は
//    piece_Context_Create で初期化
//する必要がある

// modified by Yui N., 2003.08.03
// conv_num の値のコピーを素直にした
// 2003.08.07 Yui N.  conv_num を追放

extern "C"
{
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "muslib/musdef.h"
#include "muslib/musdefwin.h"
#include "muslib/muslib.h"
#include "kmp_pi.h"
#include "context.h"

//winlayer.cpp
extern AuPIECEKPI *Decoder;
extern PCEWAVEINFO *pWave;
extern long FS, FS2, MCOUNT, FOFS;	
extern long VOLS, ENVSTART, ENVSTOP, ENVRR;
//mus.c
#define BLKN 4
#define BLKS 128
extern signed short wavebuff[BLKN][BLKS];
extern volatile unsigned char blkip;
extern volatile unsigned char blkop;
extern PCEWAVEINFO waveinfo[BLKN];
extern MDEVICE *music[MAXCH];
extern unsigned char finish;
//seq.c
#define MAXSEQ MAXCH
extern char *title;
extern char *title2;
extern SEQWK seqwk[MAXSEQ];
extern unsigned char *datatop;
extern unsigned char *drumsbase;
extern signed short tempo;
extern signed short tempowk;
extern char runf;

//context.cpp
static CRITICAL_SECTION g_cs;
struct piece_Context
{
    //winlayer.cpp
    AuPIECEKPI* Decoder;
    PCEWAVEINFO *pWave;
    long FS, FS2, MCOUNT, FOFS;	
    long VOLS, ENVSTART, ENVSTOP, ENVRR;
    // mus.c
    unsigned char finish;
    volatile unsigned char blkip;
    volatile unsigned char blkop;
    unsigned char music_wch;
    signed short wavebuff[BLKN][BLKS];
    PCEWAVEINFO waveinfo[BLKN];
    MDEVICE *music[MAXCH];
    // seq.c
    char *title;
    char *title2;
    SEQWK seqwk[MAXSEQ];
    unsigned char *datatop;
    unsigned char *drumsbase;
    signed short tempo;
    signed short tempowk;
    char runf;
    // instdef.c
    // なし（すべて const）
    // exp12.c
    // なし（すべて const）
    //wavetable.c
    // なし（すべて const）
};
static piece_Context *g_pContext = NULL;

static void piece_Context_Save(piece_Context *g);
static void piece_Context_Restore(piece_Context *g);

void piece_Context_Init(void)
{
    InitializeCriticalSection(&g_cs);
}
void piece_Context_Deinit(void)
{
    DeleteCriticalSection(&g_cs);
}
void* piece_Context_Create(AuPIECEKPI *decoder)
{
    piece_Context *g = new piece_Context;
    ZeroMemory(g, sizeof(piece_Context));
    g->Decoder = decoder;
    return g;
}
void piece_Context_Delete(void *pv)
{
    if(pv){
        EnterCriticalSection(&g_cs);
        piece_Context *context = (piece_Context*)pv;
        if(context == g_pContext){
            g_pContext = NULL;
        }
        delete context;
        LeaveCriticalSection(&g_cs);
    }
}
void piece_Context_Lock(void *pv)
{
    EnterCriticalSection(&g_cs);
    piece_Context *context = (piece_Context*)pv;
    //コピー回数を最小限に抑えるため、
    //ロックされている Context を保存しておき、次回ロック時に
    //Context が変わってなければコピーしないようにする
    if(context == g_pContext){//前回と同じ場合は何もしない
        return;
    }
    if(g_pContext){
        piece_Context_Save(g_pContext);
    }
    piece_Context_Restore(context);
    g_pContext = context;
}
void piece_Context_Unlock(void *pv)
{
    LeaveCriticalSection(&g_cs);
}
static void piece_Context_Save(piece_Context *context)
{
    //各グローバル変数群の内容を Context にコピー
    //winlayer.cpp
    context->Decoder = Decoder;
    context->pWave = pWave;
    context->FS = FS;
    context->FS2 = FS2;
    context->MCOUNT = MCOUNT;
    context->FOFS = FOFS;
    context->VOLS = VOLS;
    context->ENVSTART = ENVSTART;
    context->ENVSTOP = ENVSTOP;
    context->ENVRR = ENVRR;
    //mus.c
    context->finish = finish;
    context->blkip = blkip;
    context->blkop = blkop;
    context->music_wch = music_wch;
    memcpy(&context->wavebuff, wavebuff, sizeof(wavebuff));
    memcpy(&context->waveinfo, waveinfo, sizeof(waveinfo));
    memcpy(&context->music, music, sizeof(music));
    //seq.c
    context->title = title;
    context->title2 = title2;
    memcpy(&context->seqwk, &seqwk, sizeof(seqwk));
    context->datatop = datatop;
    context->drumsbase = drumsbase;
    context->tempo = tempo;
    context->tempowk = tempowk;
    context->runf = runf;
}
static void piece_Context_Restore(piece_Context *context)
{
    //各グローバル変数群に Context をコピー
    //winlayer.cpp
    Decoder = context->Decoder;
    pWave = context->pWave;
    FS = context->FS;
    FS2 = context->FS2;
    MCOUNT = context->MCOUNT;
    FOFS = context->FOFS;
    VOLS = context->VOLS;
    ENVSTART = context->ENVSTART;
    ENVSTOP = context->ENVSTOP;
    ENVRR = context->ENVRR;
    //mus.c
    finish = context->finish;
    blkip = context->blkip;
    blkop = context->blkop;
    music_wch = context->music_wch;
    memcpy(&wavebuff, context->wavebuff, sizeof(wavebuff));
    memcpy(&waveinfo, context->waveinfo, sizeof(waveinfo));
    memcpy(&music, context->music, sizeof(music));
    //seq.c
    title = context->title;
    title2 = context->title2;
    memcpy(&seqwk, &context->seqwk, sizeof(seqwk));
    datatop = context->datatop;
    drumsbase = context->drumsbase;
    tempo = context->tempo;
    tempowk = context->tempowk;
    runf = context->runf;
}

}//extern "C"