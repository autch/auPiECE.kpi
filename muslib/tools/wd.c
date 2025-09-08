
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct _WINWAV {
	char riff_id[4];	/* "RIFF"			*/
	long all_len;		/* fmt ïîÇ∆ data ïîÇÃëÂÇ´Ç≥(Byte)	*/
	char name[4];		/* "WAVE"			*/

	char fmt_id[4];		/* "fmt "			*/
	long fmt_len;		/* fmt ïîÇÃëÂÇ´Ç≥ (Byte)	*/
	short format;		/* Ã´∞œØƒÅE∂√∫ﬁÿ∞	*/
	short channel;		/* ¡¨›»Ÿêî			*/
	long sample;		/* ª›Ãﬂÿ›∏ﬁé¸îgêî		*/
	long sample2;		/* 1 ïbÇ†ÇΩÇËÇÃ ﬁ≤ƒêî		*/
	short bytecnt;		/* 1Ãﬁ€Ø∏ìñÇΩÇËÇÃ ﬁ≤ƒêî	*/
	short quant;		/* ó éqâªÀﬁØƒêî		*/

	char data_id[4];	/* "data"			*/
	long data_len;		/* data ïîÇÃëÂÇ´Ç≥ (Byte)	*/
} WINWAV;



static WINWAV wwh = {
	"RIFF", 0, "WAVE", "fmt ", 16,  1, 0, 0, 0, 0, 16,
	"data", 0 };


static char outfname[32];

static int total;

static int SaveWaveFile( char *fname, void *buff, int size, int freq, int ch )
{
	FILE *fp;

	wwh.channel = ch;
	wwh.sample = freq;
	wwh.sample2 = freq*ch*2;
	wwh.bytecnt = ch*2;

	wwh.data_len = size;
	wwh.all_len = size + sizeof(wwh)-8;

	fp = fopen( fname, "wb" );
	if ( fp == NULL ) return 1;

	fwrite( &wwh, 1, sizeof(wwh), fp );
	fwrite( buff, 1, size, fp );

	return 0;
}

void makesub( FILE *fp, int len, int loop_st, int loop_ed, int type )
{
	signed short *tmp;
	signed short *out;
	FILE *ofp;
	int len2 = 22050*2*10;
	int i, o, vol = (1<<20);
	int loop_w = loop_ed - loop_st;

	tmp = malloc( len );
	out = malloc( len2 );

	fread( tmp, 1, len, fp );

	len >>= 1;
	len2 >>= 1;

	if ( 1 ) {
		//                       0     1     2     3     4     5     6    *7
		static float xd[8] = { .720, .748, .727, .652, .659, .691, .721, .320 };
		static int xc = 0;
		int n;

		if ( xc == 7 ) loop_w /= 569;

		n = ( loop_ed * xd[xc++] / loop_w );
		n *= loop_w;

		loop_st -= n;
		loop_ed = loop_st + loop_w;
	}

	if ( type & 2 ) for ( i = 0; i < len; i++ ) tmp[i]^=0x8000;

	//for ( i = 0; i < len; i++ ) tmp[i]&=0xff00;

	for ( o = i = 0; o < len2; ) {
		int v1 = tmp[i>>12];
		int v2 = tmp[(i>>12)+1];
		int of = i & 0xfff;
		v1 += ((v2-v1)*of)>>12;
		v1 = ( v1 * (vol>>4) ) >> 16;
		//if ( (vol-=3) < 0 ) vol = 0;
		out[o++] = v1;
		i += 4096;
		if ( i >= loop_ed ) i -= loop_w;
	}

	SaveWaveFile( outfname, out, len2*2, 22050, 1 );

	total += (loop_ed>>12);

	free( out );
	free( tmp );
}


void makesub2( FILE *fp, int len, int loop_st, int loop_ed, int type )
{
	signed short *tmp;
	FILE *ofp;
	int i, o;

	tmp = malloc( len );

	fread( tmp, 1, len, fp );

	len >>= 1;

	if ( type & 2 ) for ( i = 0; i < len; i++ ) tmp[i]^=0x8000;

	SaveWaveFile( outfname, tmp, len*2, 22050, 1 );

	free( tmp );
}



char *df( double f )
{
	static char buff[32];
	static char *note[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	double a = 69.0 + log(f/440.0) / log(2.0) * 12.0;
	int n = (int)( a + 0.5 );

	sprintf( buff, "%8.3f Hz(%s%d %+5.1f)", f, note[n%12], (n/12)-1, (a-n)*100 );

	return buff;
}


void load_w( char *fname )
{
	FILE *fp = fopen( fname, "rb" );
	unsigned char tmpbuff[240], *tmp = tmpbuff;
	int samples;
	int i;

	if ( fp == NULL ) return;

	fread( tmp, 1, 239, fp );

	samples = tmp[198];

	for ( i = 0; i < samples; i++ ) {
		int len, loop_start, loop_end, sample_rate;
		double root_freq, low_freq, high_freq;
		double f;
		unsigned char fractions;
		tmp = tmpbuff;
		fread( tmp, 1, 7+1+28+1+18+1+40, fp );
		printf( "name = %s\n", tmp );
		fractions = tmp[7];
		printf( "fraction = 0x%x\n", fractions );
		len = *(long *)(tmp+8);
		loop_start = (*(long *)(tmp+12) <<3) + (fractions & 15);
		loop_end   = (*(long *)(tmp+16) <<3) + (fractions >> 4);
		sample_rate = *(unsigned short *)(tmp+20);
		printf( "data_length = 0x%x\n", len );
		printf( "loop_start = 0x%x.%x\n", loop_start>>4, loop_start&15 );
		printf( "loop_end = 0x%x.%x\n", loop_end>>4, loop_end&15 );
		printf( "sample_rate = %d\n", sample_rate );
		low_freq = *(long *)(tmp+22) / 1000.0;
		high_freq = *(long *)(tmp+26) / 1000.0;
		root_freq = *(long *)(tmp+30) / 1000.0;
		printf( "low_freq  = %s\n", df(low_freq) );
		printf( "high_freq = %s\n", df(high_freq) );
		printf( "root_freq = %s\n", df(root_freq) );
		{
			int n;
			f = (double)sample_rate / ((loop_end - loop_start)/16.0);
			n = (int)( root_freq / f + 0.5 );
			f *= n;
			printf( "** adjust = %s %d\n", df(f), n );
		}
		printf( "??? = %d\n", *(unsigned short *)(tmp+34) );
		printf( "panning = 0x%x\n", tmp[36] );
		tmp += 37;
		tmp += 18;
		printf( "modes = 0x%x\n", tmp[0] );
		sprintf( outfname, "tmp%d.wav", i );
		makesub( fp, len, loop_start<<8, loop_end<<8, tmp[0] );
		printf( "\n" );
	}

	fclose( fp );
}



int main( void )
{
	//load_w( "e:\\timidity\\timidity\\inst\\GUS\\acpiano.pat" );
	load_w( "e:\\timidity\\timidity\\inst\\release\\acpiano.pat" );
	printf( "total=%d\n", total );
	return 0;
}
