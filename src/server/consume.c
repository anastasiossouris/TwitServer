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
 * \file consume.c
 *
 * File consume.c contains the implementation of the consume.h header file.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "serverinfo.h"
#include "consume.h"
#include "twitpool.h"
#include "twitpoollist.h"
#include "twit.h"



/**
 * The broadcast_twit() function shall send the twit pointed to by parameter t to the struct twitpool objects in the struct twitpoollist
 * object inside the struct serverinfo object pointed to by parameter si. Neither parameter shall be a NULL pointer.
 *
 * @return Nothing.
 */
static void broadcast_twit( struct serverinfo * restrict si, const struct twit * restrict t );



/**
 * twitpoolConsumer() is responsible for getting the twits from the twitpool where the server stores the twits
 * send by the sayers and broadcasting those twits to all the hearers. 
 */
void *twitpoolConsumer( void *arg ){
	struct serverinfo *si = ( struct serverinfo * )arg;
	struct twit t;

	assert( arg != NULL );

	// Prepared
	signal_prepared_status( si, 1 );

	// Start consuming twits
	while ( 1 ){
		// Get a twit from the twitpool
		acquire_twitpool( si );
		while ( twitpoolisempty( &si->si_twitpool ) ){
			while ( pthread_cond_wait( &si->si_twitpool_cond, &si->si_twitpool_lock ) ){ continue; }
		}
		errno = 0;
		( void )getfromtwitpool( &si->si_twitpool, &t );
		assert( errno == 0 );
		release_twitpool( si );

		// Send the twit to all the hearers
		broadcast_twit( si, &t );

		// Free the twit 
		free( t.t_twit );
	}

	pthread_exit( NULL );
}



// Implementation of local functions...

// Send the twit to all twitpools
static void broadcast_twit( struct serverinfo * restrict si, const struct twit * restrict t ){
	struct twitpoollist_node *tpln = NULL;

	assert( si != NULL );
	assert( t != NULL );

	// Acquire ownership of the twitpoollist
	acquire_twitpool_list( si );

	// For each twitpool
	for ( tpln = si->si_twitpool_list.tpl_head; tpln != NULL; tpln = tpln->tpln_next ){
		struct twitpool *tp = &tpln->tpln_twitpool;

		// Acquire ownership of the current twitpool
		acquire_twitpool_in_twitpoollist_node( tpln );

		// Store the twit in the twitpool
		( void )putintwitpool( tp, t->t_twit, t->t_twitlen );

		// Signal that a twit just inserted in the twitpool
		while ( pthread_cond_signal( &tpln->tpln_cond ) ){ continue; }

		// Release ownership of the current twitpool
		release_twitpool_in_twitpoollist_node( tpln );
	}

	// Release ownership of the twitpoollist
	release_twitpool_list( si );
	

	return ;
}
