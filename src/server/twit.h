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
 * File twit.h provides the twit structure that is used to store a twit.
 *
 * @author Tassos Souris
 */
#if !defined( TWIT_H_IS_INCLUDED )
#define TWIT_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#include <stddef.h>

/**
 * \struct twit
 *
 * The twit structure is an object capable of storing a single twit.
 */
struct twit{
	char * restrict t_twit; /**< Pointer to the twit */
	size_t t_twitlen; /**< Lenght of the twit */
};

/**
 * The filltwit() function shall store a pointer to a copy of the the first len bytes of the string pointed to by parameter str
 * in the t_twit member of the struct twit object pointed to by parameter t. 
 *
 * @return Upon successful completion zero shall be returned; otherwise, -1 shall be returned and errno shall be set to indicate the error.
 * @param Pointer to the struct twit object that will be made to point to the newly allocated and copied string.
 * @param str The string to be copied.
 * @param len The number of bytes to be copied from the string pointed to by parameter str.
 * @exception EINVAL Parameters t or str is a NULL pointer or parameter len is zero.
 * @exception ENOMEM Insufficient storage space to perform the operation.
 */
int filltwit( struct twit * restrict t, const char * restrict str, size_t len );

/**
 * The deltwit() function shall deallocate any resources that were allocated for the struct twit object pointed to by parameter t, except from  
 * that object itself. If parameter t is a NULL pointer no action shall occur.
 *
 * @return Nothing.
 * @param t Pointer to the struct twit object from which the resources shall be deallocated.
 */
void deltwit( struct twit * restrict t );

#if defined( __cplusplus )
}
#endif

#endif
