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
 * \file twitmanager.h
 *
 * File twitmanager.h declares all the necessary functions for the sayers to put twits
 * and the hearers to retrieve those twits.
 *
 * The functions declared in this interface for a specific struct twitmanager object are a monitor; this means
 * that for the same struct twitmanager object the functions are "given" for only one thread each time.
 *
 * The interface works as:
 *	Sayers are free to insert twits in the manager of twits, which is represented by a struct twitmanager object,
 *	with a call to the puttwit() function.
 *	For a hearer to retrieve twits inserted by sayers it must first register itself in the struct twitmanager object
 *	by obtaining some sort of an identifier and later retrieve twits from the struct twitmanager object using
 *	that identifier (called cursor in this interface) using the gettwit() function. Then the hearer is about to terminate
 *	it calls removefromtwitmanager() to "unregister" itself from the manager.
 *
 * @author Tassos Souris
 */
#if !defined( TWITMANAGER_H_IS_INCLUDED )
#define TWITMANAGER_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#include <stddef.h>
#include <pthread.h>
#include "twit.h"
#include "twitpoollist.h"



/**
 * \struct twitmanager
 *
 * The twitmanager structure is an object that is capable of storing, retrieving and handling twits for the sayers and
 * hearers.
 */
struct twitmanager{
	struct twitpoollist tm_list;
	pthread_mutex_t tm_critical_lock;
	pthread_cond_t tm_critical_cond;
	int tm_is_critical;
	int tm_twitcount;
};

/**
 * \typedef twitmanagercursor_t
 *
 * twitmanagercursor_t is used as an identifier for a hearer and it is assigned by means of a call to the 
 * registerintwitmanager() function.
 */
typedef struct twitpoollist_node * twitmanagercursor_t;



/**
 * The inittwitmanager() function shall initialize the struct twitmanager object pointed to by parameter tm.
 * It is undefined behavior if the other functions declared in this interface are called for the same object
 * before the inittwitmanager() function is called.
 *
 * @return The inittwitmanager() function shall return zero if successful; otherwise, -1 shall be returned and errno
 *	shall be set to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @exception EINVAL Parameter tm is a NULL pointer.\
 * @exception ENOBUFS Insufficient resources for the operation to be completed.
 */
int inittwitmanager( struct twitmanager * restrict tm );

/**
 * The registerintwitmanager() function shall register a hearer in the manager of twits represented by the struct twitmanager
 * object pointed to by parameter tm and assign a twitmanagercursor_t object for that hearer in the twitmanagercursor_t object
 * pointed to by parameter cursor. The hearer is represented by that cursor which is used in the gettwit() function.
 *
 * @return The registerintwitmanager() function shall return zero if successful; otherwise, -1 shall be returned and errno shall
 *	set to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @param cursor Pointer to the twitmanagercursor_t object.
 * @exception EINVAL At least one of the parameters is a NULL pointer.
 * @exception ENOBUFS Insufficient resources for the function to complete the operation. The hearer is not registered.
 */
int registerintwitmanager( struct twitmanager * restrict tm, twitmanagercursor_t * restrict cursor );

/**
 * The removefromtwitmanager() function shall unregister a hearer represented by the twitmanagercursor_t object pointed to by parameter
 * cursor from the struct twitmanager object pointed to by parameter tm.
 *  It is undefined behavior if the twitmanagercursor_t object has not been obtained by a call to the registerintwitmanager() function.
 *
 * @return The removefromtwitmanager() function shall return zero if successful; otherwise, -1 shall be returned and errno shall be set
 *	to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @param cursor Pointer to the twitmanagercursor_t object.
 * @exception EINVAL At least one of the parameters is a NULL pointer.
 */
int removefromtwitmanager( struct twitmanager * restrict tm, twitmanagercursor_t * restrict cursor );

/**
 * The puttwit() function shall insert a copy of the given string in the twitmanager structure pointed to by parameter tm for all 
 * registered hearers to retrieve later by means of a call to the gettwit() function.
 *
 * @return The puttwit() function shall return zero if successful; otherwise, -1 shall be returned and errno shall be set to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @param string Pointer to the string (twit).
 * @param string_len Number of bytes of the string.
 * @exception EINVAL Parameters tm or string is a NULL pointer or parameter string_len is zero.
 * @exception ENOMEM The twit could not be stored for at least one registered hearer.
 * @exception ENOBUFS Insufficient resources for the function to perform the operation (stored for no hearer).
 */
int puttwit( struct twitmanager * restrict tm, const char * restrict string, size_t string_len );

/**
 * The gettwit() function shall retrieve a twit from the specified twitmanagercursor_t object, as allocated for a registered hearer
 * by means of a call to the registerintwitmanager() function. The result shall be stored in the struct twit object pointed to by 
 * parameter t. It is undefined behavior if the twitmanagercursor_t object has not been obtained by a call to the registerintwitmanager() function.
 * Note that a client is responsible for deallocating the resources stored in the struct twit object pointed to by parameter t.
 *
 * @return The gettwit() function shall return zero if successful; otherwise -1 shall be returned and errno shall be set to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @param t Pointer to the struct twit object.
 * @param cursor Pointer to the twitmanagercursor_t object.
 * @exception EINVAL At least one of the parameters is a NULL pointer.
 * @exception EPERM No twit is available for the registered hearer.
 */
int gettwit( struct twitmanager * restrict tm, struct twit * restrict t, twitmanagercursor_t * restrict cursor );

/**
 * The gettwitcount() function shall retrieve the number of twits stored in the struct twitmanager object pointed to by parameter tm.
 *
 * @return The gettwitcount() function shall return the number of twits if successful; otherwise, -1 shall be returned and errno shall be set
 *	to indicate the error.
 * @param tm Pointer to the struct twitmanager object.
 * @exception EINVAL Parameter tm is a NULL pointer.
 */
int gettwitcount( const struct twitmanager * restrict tm );

/**
 * The deltwitmanager() function shall deallocate all the resources that were reserved for the struct twitmanager
 * object pointed to by parameter tm but not the object itself. If parameter tm is a NULL pointer no action shall occur.
 * It is undefined whether the struct twitmanager object can be used without a call to the inittwitmanager() function after
 * deltwitmanager() has been called for the same object.
 *
 * @return The deltwitmanager() function shall not return any value.
 * @param tm Pointer to the struct twitmanager object.
 */
void deltwitmanager( struct twitmanager * restrict tm );

#if defined( __cplusplus )
}
#endif

#endif
