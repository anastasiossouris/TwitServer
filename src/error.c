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
 * \file error.c
 *
 * File error.c contains the implementation of the error.h interface.
 *
 * @author Tassos Souris
 */
#include <stdarg.h>
#include <stdio.h>
#include "error.h"

// fprintf() like error message function
void error( const char * restrict format, ... ){
	va_list ap;

	va_start( ap, format );

	( void )vfprintf( stderr, format, ap );
	( void )fflush( stderr );

	va_end( ap );

	return ;
}
