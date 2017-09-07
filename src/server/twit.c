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
/**
 * \file twit.c
 *
 * File twit.c contains the implementation of the twit.h interface. 
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "twit.h"

// Create a copy of the string and store a pointer to it in the twit structure
int filltwit( struct twit * restrict t, 
		const char * restrict str, 
		size_t len ){
	char *twit = NULL;

	// Validate the parameters
	if ( t == NULL || str == NULL || len == 0 ){
		errno = EINVAL;
		return ( -1 );
	}

	// Allocate storage for the string to be copied
	if ( ( twit = malloc( ( len + 1 ) * sizeof( char ) ) ) == NULL ){
		assert( errno == ENOMEM );
		return ( -1 );
	}

	// Copy the string
	( void )memcpy( twit, str, len + 1 );

	// Store results back in the twit structure
	t->t_twit = twit;
	t->t_twitlen = len;

	return ( 0 );
}

// Free the string pointed to by t_twit
void deltwit( struct twit * restrict t ){
	if ( t != NULL ){
		free( t->t_twit );
		t->t_twit = NULL;
	}
}
