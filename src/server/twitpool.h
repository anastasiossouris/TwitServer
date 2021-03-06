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
 * \file twitpool.h 
 *
 * File twitpool.h declares several function and defines a type for the handling of a set of twits.
 *
 * @author Tassos Souris
 */
#if !defined( TWITPOOL_H_IS_INCLUDED )
#define TWITPOOL_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#include <sys/types.h>
#include <stddef.h>
#include "twit.h"

/**
 * \struct twitpool 
 *
 * The twitpool structure represents a set of twits.
 */
struct twitpool{
	struct twitpool_node *tp_head;
	struct twitpool_node *tp_tail;
	size_t tp_count;
};

/**
 * \struct twitpool_node
 *
 * The twitpool_node structure stores each individual twit inside the set.
 */
struct twitpool_node{
	struct twitpool_node *tpn_next;
	struct twitpool_node *tpn_previous;
	struct twit tpn_twit;
};


/**
 * The inittwitpool() function shall initialize the struct twitpool object pointed to by parameter tp. It is undefined behavior for all other functions
 * declared in this interface if initwitpool() has not been called first.
 *
 * @return Upon successful completion zero shall be returned; otherwise, -1 shall be returned and errno shall be set to indicate the error.
 * @param tp Pointer to the struct twitpool object to be initialized.
 * @exception EINVAL Parameter tp is a NULL pointer.
 * @exception ENOBUFS No resources available to perform the operation.
 */
int inittwitpool( struct twitpool * restrict tp );

/**
 * The putintwitpool() function shall store in the set of twits represented by the struct twitpool object pointed to by parameter tp
 * a copy of the string pointed to by parameter string. No more than string_len bytes shall be copied.
 *
 * @return Upon successful completion zero shall be returned; otherwise, -1 shall be returned and errno shall be set to indicate the error.
 * @param tp Pointer to the struct twitpool object.
 * @param string Pointer to the string to be copied.
 * @param string_len Number of bytes to be copied from the string.
 * @exception EINVAL Parameters tp or string is a NULL pointer or parameter string_len is zero.
 * @exception ENOMEM Insufficiet storage space to perform the operation.
 */
int putintwitpool( struct twitpool * restrict tp, const char * restrict string, size_t string_len );

/**
 * The getfromtwitpool() function shall retrieve a twit from the set of twits represented by the struct twitpool object pointed to by
 * parameter tp and store the results in the struct twit object pointed to by parameter t. Note that the client is responsible for deallocating
 * the resources reserved for the struct twit object that was stored. 
 *
 * @return The getfromtwitpool() function shall return zero if successful; otherwise, -1 shall be returned and errno shall be set to indicate the error.
 * @param tp Pointer to the struct twitpool object. 
 * @param t Pointer to the struct twit object.
 * @exception EINVAL Parameters tp or t is a NULL pointer.
 * @exception EPERM There is no twit available in the set.
 */
int getfromtwitpool( struct twitpool * restrict tp, struct twit * restrict t );

/**
 * The twitpoolisempty() function shall test whether the struct twitpool object pointed to by parameter tp is empty or not.
 *
 * @return The twitpoolisempty() function shall return 1 if it is empty, zero if not and -1 in case of an failure in which case
 *	errno is set to indicate the error.
 * @param tp Pointer to the struct twitpool object.
 * @exception EINVAL If parameter tp is a NULL pointer.
 */
int twitpoolisempty( const struct twitpool * restrict tp );

/**
 * The twitpoolcount() function shall return the number of twits stored in the twitpool object pointed to by parameter tp.
 *
 * @return The twitpoolcount() function shall return the count if successful; otherwise, -1 shall be returned and errno
 *	shall be set to indicate the error.
 * @exception EINVAL parameter tp is a NULL pointer.
 */
ssize_t twitpoolcount( const struct twitpool * restrict tp );

/**
 * The deltwitpool() function shall deallocate all the resources that were reserved for the struct twitpool object pointed to by parameter tp.
 * If parameter is a NULL pointer no action shall occur. It is undefined whether the struct twitpool object can be used with putintwitpool(),
 * getfromtwitpool() and twitpoolisempty() functions before function inittwitpool() is called.
 * 
 * @return Nothing.
 * @param tp Pointer to the struct twitpool object.
 */
void deltwitpool( struct twitpool * restrict tp );

#if defined( __cplusplus )
}
#endif

#endif
