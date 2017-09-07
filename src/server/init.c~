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
 * \file init.c
 *
 * File init.c contains the implementation of the init.c interface.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "serverinfo.h"
#include "statistics.h"
#include "twitpool.h"
#include "consume.h"
#include "listen.h"
#include "init.h"
#include "error.h"



/**
 * The startStatisticsUpdater() function shall initialize and start the thread that runs the statisticsUpdater() function.
 *
 * @return The startStatisticsUpdater() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int startStatisticsUpdater( struct serverinfo * restrict si );

/**
 * The startHearersListener() function shall initialize and start the thread that runs the hearersListener() function.
 *
 * @return The startHearersListener() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int startHearersListener( struct serverinfo * restrict si );

/**
 * The startSayersListener() function shall initialize and start the thread that runs the sayersListener() function.
 *
 * @return The startSayersListener() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int startSayersListener( struct serverinfo * restrict si );

/**
 * The startTwitpoolConsumer() function shall initialize and start the thread that runs the twitpoolConsumer() function.
 *
 * @return The startTwitpoolConsumer() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int startTwitpoolConsumer( struct serverinfo * restrict si );

/**
 * The initServerinfo() function shall initialize the struct serverinfo object pointed to by parameter si.
 *
 * @return The initServerinfo() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int initServerinfo( struct serverinfo * restrict si );



/**
 * initializeServer() is used to initialize all things in the server.
 * First it must initialize the members of the struct serverinfo pointed to by parameter si.
 * Then it must start the following threads:
 *	1) The one that updates the statistics
 *	2) The one that listens for hearers
 *	3) The one that listens for sayers
 *
 * Note that the following must be done in that order or otherwise information might get lost.
 * For example, if the listeners get started before the statistics updater and messages get exchanged
 * then the statistics updater will not update successfully the statistics structure.
 */
int initializeServer( struct serverinfo * restrict si ){
	
	assert( si != NULL );
	
	if ( initServerinfo( si ) == -1 ){
		return ( -1 );
	}

	if ( startTwitpoolConsumer( si ) == -1 ){
		return ( -1 );
	}

	if ( startStatisticsUpdater( si ) == -1 ){
		return ( -1 );
	}

	if ( startHearersListener( si ) == -1 ){
		return ( -1 );
	}

	if ( startSayersListener( si ) == -1 ){
		return ( -1 );
	}

	return ( 0 );
}



// Here follows the implementation of local functions...

// Start statisticsUpdater()
static int startStatisticsUpdater( struct serverinfo * restrict si ){
	int prepared;

	assert( si != NULL );

	// Start the thread that updates the statistics	
	acquire_preparation_status( si );
	si->si_prepared = -1;	
	release_preparation_status( si );
	if ( ( errno = pthread_create( &si->si_statistics_updater_threadid, NULL, &statisticsUpdater, si ) ) ){
		error( "Failed to start the thread that updates the statistics (%s).\n", strerror( errno ) );
		return ( -1 );
	}
	// Wait for the preparation status
	acquire_preparation_status( si );
	while ( si->si_prepared == -1 ){
		while ( pthread_cond_wait( &si->si_prepared_cond, &si->si_prepared_lock ) ){ continue; }
	}
	prepared = si->si_prepared;
	release_preparation_status( si );
	if ( prepared == 0 ){
		error( "The thread that updates the statistics failed to be initialized.\n" );
		return ( -1 );
	}
	// Must update the statistics here cause one more thread got created
	// We must lock here cause statisticsUpdater() runs in the background
	acquire_statistics( si );
	increaseThreadsNum( &si->si_stats );
	release_statistics( si );

	return ( 0 );
}

// Start hearersListener()
static int startHearersListener( struct serverinfo * restrict si ){
	int prepared;

	assert( si != NULL );

	// Start the thread that listens for hearers
	acquire_preparation_status( si );
	si->si_prepared = -1;	
	release_preparation_status( si );
	if ( ( errno = pthread_create( &si->si_hearers_listener_threadid, NULL, &hearersListener, si ) ) ){
		error( "Failed to start the thread that listens for hearers (%s).\n", strerror( errno ) );
		return ( -1 );
	}
	// Wait for the preparation status
	acquire_preparation_status( si );
	while ( si->si_prepared == -1 ){
		while ( pthread_cond_wait( &si->si_prepared_cond, &si->si_prepared_lock ) ){ continue; }
	}
	prepared = si->si_prepared;
	release_preparation_status( si );
	if ( prepared == 0 ){
		error( "The thread that listens for hearers failed to be initialized.\n" );
		return ( -1 );
	}
	// Must update the statistics here cause one more thread got created
	acquire_statistics( si );
	increaseThreadsNum( &si->si_stats );
	release_statistics( si );
	
	return ( 0 );
}

// Start sayersListener()
static int startSayersListener( struct serverinfo * restrict si ){
	int prepared;

	assert( si != NULL );

	// Start the thread that listens for sayers
	acquire_preparation_status( si );
	si->si_prepared = -1;	
	release_preparation_status( si );
	if ( ( errno = pthread_create( &si->si_sayers_listener_threadid, NULL, &sayersListener, si ) ) ){
		error( "Failed to start the thread that listens for sayers (%s).\n", strerror( errno ) );
		return ( -1 );
	}
	// Wait for the preparation status
	acquire_preparation_status( si );
	while ( si->si_prepared == -1 ){
		while ( pthread_cond_wait( &si->si_prepared_cond, &si->si_prepared_lock ) ){ continue; }
	}
	prepared = si->si_prepared;
	release_preparation_status( si );
	if ( prepared == 0 ){
		error( "The thread that listens for sayers failed to be initialized.\n" );
		return ( -1 );
	}
	// Must update the statistics here cause one more thread got created
	acquire_statistics( si );
	increaseThreadsNum( &si->si_stats );
	release_statistics( si );

	return ( 0 );
}	

// Start the twitpool consumer
static int startTwitpoolConsumer( struct serverinfo * restrict si ){
	int prepared;

	assert( si != NULL );

	// Start the thread that runs twitpoolConsumer()
	acquire_preparation_status( si );
	si->si_prepared = -1;
	release_preparation_status( si );
	if ( ( errno = pthread_create( &si->si_twitpool_consumer_threadid, NULL, &twitpoolConsumer, si ) ) ){
		error( "Failed to start the thread that consumes the twits (%s).\n", strerror( errno ) );
		return ( -1 );
	}
	// Wait for the preparation status
	acquire_preparation_status( si );
	while ( si->si_prepared == -1 ){
		while ( pthread_cond_wait( &si->si_prepared_cond, &si->si_prepared_lock ) ){ continue; }
	}
	prepared = si->si_prepared;
	release_preparation_status( si );
	if ( prepared == 0 ){
		error( "The thread that consumes the twits failed to be initialized.\n" );
		return ( -1 );
	}
	// Must update the statistics cause one more thread got created
	acquire_statistics( si );
	increaseThreadsNum( &si->si_stats );
	release_statistics( si );

	return ( 0 );
}

// Set up the fields in *si
static int initServerinfo( struct serverinfo * restrict si ){
	struct statistics *st = NULL;

	assert( si != NULL );

	// Initialize the serverinfo structure. Note there is no need to lock the various fields
	// here cause only one thread exists
	while ( pthread_mutex_init( &si->si_stats_lock, NULL ) ){ continue; }
	while ( pthread_cond_init( &si->si_stats_sayers_cond, NULL ) ){ continue; }
	while ( pthread_cond_init( &si->si_stats_hearers_cond, NULL ) ){ continue; }
	while ( pthread_mutex_init( &si->si_prepared_lock, NULL ) ){ continue; }
	while ( pthread_cond_init( &si->si_prepared_cond, NULL ) ){ continue; }
	while ( pthread_mutex_init( &si->si_twitpool_lock, NULL ) ){ continue; }
	while ( pthread_cond_init( &si->si_twitpool_cond, NULL ) ){ continue; }
	while ( pthread_mutex_init( &si->si_twitpool_list_lock, NULL ) ){ continue; }

	// Init statistics
	st = &si->si_stats;
	st->stats_storedTwitsNum = 0;
	st->stats_threadsNum = 1; // one for the main thread
	st->stats_hearersNum = 0;
	st->stats_sayersNum = 0;
	st->stats_arrivedTwitsNum = 0;
	st->stats_deliveredTwitsNum = 0;
	st->stats_averageTwitsIncomingRate = 0.0;
	st->stats_averageTwitsOutcomingRate = 0.0;

	// Init twitpool
	if ( inittwitpool( &si->si_twitpool ) == -1 ){
		return ( -1 );
	}

	// Init twitpool list
	if ( inittwitpoollist( &si->si_twitpool_list ) == -1 ){
		return ( -1 );
	}

	return ( 0 );
}
