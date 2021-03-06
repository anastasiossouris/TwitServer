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
 * \file twitmanager.c
 *
 * File twitmanager.c contains the implementation of the twitmanager.h interface.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "twit.h"
#include "twitpool.h"
#include "twitpoollist.h"
#include "twitmanager.h"



/**
 * Implementation details:
 *
 * The twit manager has a struct twitlist list for each hearer registered. Each node of this list has a struct twitpool into 
 * which the twits are stored for that specific hearer. That means that when a twit is stored with puttwit() it is replicated
 * as many times as the number of hearers registered and thus the number of nodes in the list. The twit is being stored in each
 * struct twitpool. The image is like this:
 *
 *        +------------+
 *	  + Hearer id  +  --> struct twitpool for that hearer
 *        +------------+
 *             |
 *	  +------------+
 *	  +  Hearer id +  --> struct twitpool for that hearer
 *        +------------+
 *  	       |
 *	      ...
 *        +------------+
 *	  + Hearer id  +  --> struct twitpool for that hearer
 *	  +------------+
 */



/**
 * The acquire_twitmanager() function shall acquire ownership of the struct twitmanager object pointed to by parameter tm
 * which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param tm Pointer to the struct twitmanager object.
 */
static inline void acquire_twitmanager( struct twitmanager * restrict tm );

/**
 * The acquire_twitmanager() function shall release ownership of the struct twitmanager object pointed to by parameter tm
 * which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param tm Pointer to the struct twitmanager object.
 */
static inline void release_twitmanager( struct twitmanager * restrict tm );

/**
 * The acquire_twitpool() function shall acquire ownership of the struct twitpool object inside the twitpoollist_node object 
 * pointed to by parameter tpln which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param tpln Pointer to the struct twitpoollist_node object.
 */
static inline void acquire_twitpool( struct twitpoollist_node * restrict tpln );

/**
 * The acquire_twitpool() function shall release ownership of the struct twitpool object inside the twitpoollist_node object 
 * pointed to by parameter tpln which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param tpln Pointer to the struct twitpoollist_node object.
 */
static inline void release_twitpool( struct twitpoollist_node * restrict tpln ); 

/**
 * The broadcast_twit() function shall place the given twit in all twit pools inside the twitmanager.
 *
 * @return Refer to the puttwit() function.
 */
static inline int broadcast_twit( struct twitmanager * restrict tm, const char * restrict string, size_t string_len );



// Initialize the twit manager
int inittwitmanager( struct twitmanager * restrict tm ){
	// Validate the parameter
	if ( tm == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Initialize the list
	( void )inittwitpoollist( &tm->tm_list );

	// Set count to zero
	tm->tm_twitcount = 0;

	// Initialize the mutexes and conditions
	while ( pthread_mutex_init( &tm->tm_critical_lock, NULL ) ){ continue; }
	while ( pthread_cond_init( &tm->tm_critical_cond, NULL ) ){ continue; }

	return ( 0 );
}

// Allocate a identifier for a hearer
int registerintwitmanager( struct twitmanager * restrict tm, 
		twitmanagercursor_t * restrict cursor ){
	int status = 0; // Must initialize to zero

	// Validate the parameters
	if ( tm == NULL || cursor == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Acquire ownership of the twit manager
	acquire_twitmanager( tm );

	// To register a hearer in the twit manager i must give him
	// a place in the list for it to get the twits from
	errno = 0;		
	if ( newtwitpool( &tm->tm_list, cursor ) == -1 ){
		assert( errno == ENOMEM );
		status = -1;
		// CAUTION: Must **not** return here cause the twit manager will not be released
		break;
	}
 	
	// Release ownership of the twit manager
	release_twitmanager( tm );

	return ( status );
}

// Unregister a hearer's identifier from the twit manager
int removefromtwitmanager( struct twitmanager * restrict tm, 
		twitmanagercursor_t * restrict cursor ){
	// Validate the parameters
	if ( tm == NULL || cursor == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Acquire ownership of the twit manager
	acquire_twitmanager( tm );

	// To remove a hearer the node for its twitpool must be removed 
	( void )removefromtwitpoollist( &tm->tm_list, *cursor );

	// Release ownership of the twit manager
	release_twitmanager( tm );

	return ( 0 );
}

// Put the given twits so as all hearers will get it
int puttwit( struct twitmanager * restrict tm, 
		const char * restrict string, 
		size_t string_len ){
	int status = 0; // must initialize to zero

	// Validate the paramaters
	if ( tm == NULL || string == NULL || len == 0 ){	
		errno = EINVAL;
		return ( -1 );
	}

	// Acquire ownership of the twit manager
	acquire_twitmanager( tm );

	// This twit must be put in all twit pools so as 
	// the hearers will get it
	status = broadcast_twit( tm, string, string_len );

	// A twit was stored so increase count by one
	if ( status != -1 ){
		++tm->tm_twitcount;
	}

	// Release ownership of the twit manager
	release_twitmanager( tm );

	return ( status );
}

// Get a twit from the twit pool's identifier cursor
int gettwit( struct twitmanager * restrict tm, 
		struct twit * restrict t, 
		twitmanagercursor_t * restrict cursor ){
	int status = 0; // must initialize to zero 

	// Validate the parameters
	if ( tm == NULL || t == NULL || cursor == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Acquire ownership of the twit manager
	acquire_twitmanager( tm );

	// get the twit from the twit pool
	status = getfromtwitpool( &( *cursor )->tpln_twitpool, t );

	// Release ownership of the twit manager
	release_twitmanager( tm );

	return ( status );
}

// Return how many twits stored
int gettwitcount( const struct twitmanager * restrict tm ){
	int count = 0;
	
	// Validate the parameter
	if ( tm == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Acquire ownership of the twit manager
	acquire_twitmanager( tm );
	
	count = tm->tm_twitcount;

	// Release ownership of the twit manager
	release_twitmanager( tm );

	return ( count );
}

// Deallocate everything from the twitmanager
void deltwitmanager( struct twitmanager * restrict tm ){
	// Validate the parameter
	if ( tm == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Destroy the list
	deltwitpoollist( tm->tm_list );

	// Destroy the mutexes and conditions here
	while ( pthread_mutex_destroy( tm->tm_critical_lock ) ){ continue; }
	while ( pthread_cond_destroy( tm->tm_critical_cond ) ){ continue; }

	return ( 0 );
}



// Implementation of local functions...

// Send the twit to all twit pools
static inline int broadcast_twit( struct twitmanager * restrict tm, 
		const char * restrict string, 
		size_t string_len ){
	struct twitpoollist_node *node = NULL;
	int status = 0; // must initialize to zero

	assert( tm != NULL );
	assert( string != NULL );
	assert( string_len != 0 );
	
	// For each registered hearer 
	for ( node = tm->tm_list.tpl_head; node != NULL; node = node->tpln_next ){
		// Must lock the node before storing a twit inside it
		acquire_twitpool( node );
		// Put the twit in the twit pool
		if ( putintwitpool( &node->tpln_twitpool, string, string_len ) == -1 ){
			status = -1;
			break;		
		}
		// Now that the twit is stored in the twitpool signal it
		while ( pthread_cond_signal( &node->tpln_cond, &node->tpln_lock ) ){ continue; }
		// Release the node now
		release_twitpool( node );
	}

	return ( status );
}

// Lock (acquire) the manager for a thread
static inline void acquire_twitmanager( struct twitmanager * restrict tm ){
	assert( tm != NULL );

	while ( pthread_mutex_lock( &tm->tm_critical_lock ) ){ continue; }
	while ( tm->tm_is_critical ){
		while ( pthread_cond_wait( &tm->tm_critical_cond, &tm->tm_critical_lock ) ){ continue; }
	}
	tm->tm_is_critical = 1;
	while ( pthread_mutex_unlock( &tm->tm_critical_unlock ) ){ continue; }

	return ;
}

// Unlock (release) the manager for a thread
static inline void release_twitmanager( struct twitmanager * restrict tm ){
	assert( tm != NULL );

	while ( pthread_mutex_lock( &tm->tm_critical_lock ) ){ continue; }
	tm->tm_is_critical = 0;
	while ( pthread_cond_signal( &tm->tm_critical_cond ) ){ continue; }
	while ( pthread_mutex_unlock( &tm->tm_critical_lock ) ){ continue; }

	return ;
}

// Lock (acquire) the twitpool inside the node
static inline void acquire_twitpool( struct twitpoollist_node * restrict tpln ){
	assert( tpln != NULL );
	
	while ( pthread_mutex_lock( &tpln->tpln_lock ) ){ continue; }

	return ;
}


// Unlock (release) the twitpool inside the node
static inline void release_twitpool( struct twitpoollist_node * restrict tpln ){
	assert( tpln != NULL );
	
	while ( pthread_mutex_unlock( &tpln->tpln_lock ) ){ continue; }

	return ;
}
