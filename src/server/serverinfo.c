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
 * \file serverinfo.c
 *
 * File serverinfo.c contains the implementation of the serverinfo.h interface.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <pthread.h>
#include "serverinfo.h"



void acquire_statistics( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_lock( &si->si_stats_lock ) ){ continue; }

	return ;
}

void release_statistics( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_unlock( &si->si_stats_lock ) ){ continue; }

	return ;
}

void acquire_twitpool( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_lock( &si->si_twitpool_lock ) ){ continue; }

	return ;
}

void release_twitpool( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_unlock( &si->si_twitpool_lock ) ){ continue; }

	return ;
}

void acquire_twitpool_list( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_lock( &si->si_twitpool_list_lock ) ){ continue; }

	return ;
}

void release_twitpool_list( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_unlock( &si->si_twitpool_list_lock ) ){ continue; }

	return ;
}

void acquire_preparation_status( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_lock( &si->si_prepared_lock ) ){ continue; }

	return ;
}

void release_preparation_status( struct serverinfo * restrict si ){
	assert( si != NULL );

	while ( pthread_mutex_unlock( &si->si_prepared_lock ) ){ continue; }

	return ;
}

void signal_prepared_status( struct serverinfo * restrict si, int status ){
	assert( si != NULL );
	assert( status == 0 || status == 1 );

	acquire_preparation_status( si );
	assert( si->si_prepared == -1 );
	si->si_prepared = status;
	while ( pthread_cond_signal( &si->si_prepared_cond ) ){ continue; }
	release_preparation_status( si );

	return ;
}
