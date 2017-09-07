/**
	Copyright (c) 2009,2010, Tassos Souris
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of the <organization> nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY Tassos Souris ''AS IS'' AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL Tassos Souris BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "twit.h"
#include "twitpool.h"

#define LIMIT 10

int main( void ){
	struct twitpool tp;
	const char msg[] = "Hello World!";
	size_t msglen = strlen( msg );

	( void )inittwitpool( &tp );

	printf( "%d\n", ( int )msglen );
	fflush( stdout );

	for ( int i = 0; i < LIMIT; ++i ){
		int result = putintwitpool( &tp, msg, msglen );
		if ( result == -1 ){
			perror( "putintwitpool() failed" );
			exit( EXIT_FAILURE );
		}
	}

	
	for ( int i = 0; i < LIMIT; ++i ){
		struct twit t;
		int result = getfromtwitpool( &tp, &t );
		if ( result == -1 ){
			perror( "getfromtwitpool() failed" );
			exit( EXIT_FAILURE );
		}
		( void )printf( "Retrieved: %s\n", t.t_twit );
		( void )fflush( stdout );
		deltwit( &t );
	}

	deltwitpool( &tp );

	exit( EXIT_SUCCESS );
}
