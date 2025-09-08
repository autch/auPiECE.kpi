
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DIVN 12

unsigned short expt[DIVN];
unsigned short expb[DIVN];

void mktbl( void )
{
	int i;
	for ( i = 0; i < DIVN; i++ ) {
		double a = 32768.0 * pow( 2.0, i/(double)DIVN );
		double b = 32768.0 * pow( 2.0, (i+1)/(double)DIVN );
		expt[i] = a + 0.5;
		expb[i] = ( b - a ) + 0.5;
	}

	for ( i = 0; i < DIVN; i++ ) printf( "\t%d,\n", expt[i] );
	for ( i = 0; i < DIVN; i++ ) printf( "\t%d,\n", expb[i] );
}


int calc( unsigned short n )
{
	int a = (n >> 8);
	int s = a%12;
	int b = (n & 0xff);
	a /= 12;
	return ( expt[s] + ((expb[s] * b) >> 8) ) >> (15-a);
}



void cktbl( void )
{
	int i;
	for ( i = 256*12*8; i < 256*12*16; i++ ) {
		int a = pow( 2.0, i/(12.0*256.0) ) + 0.5;
		int b = calc( (unsigned short)i );
		double e = (log((double)b/a) * log(2.0)) * 1200.0;
		if ( fabs(e) > 0.3 ) printf( "%d %d %d %5.1f\n", i, a, b, e );
	}
}


void main( void )
{
	mktbl();
	cktbl();
}
