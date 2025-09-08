
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


void calc( double f )
{
	double n = log(f) / log(2.0) * 12.0 * 256.0;
	int i = (int)(n + 0.5);
	printf( "%8.2f %d(0x%x)\n", f, i, i );
}


int main( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		calc( atof( argv[1] ) );
	}
	return 0;
}
