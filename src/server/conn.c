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
 * \file conn.c
 *
 * The conn.c file contains the implementation of the conn.h interface.
 *
 * @author Tassos Souris
 */
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pthread.h>
#include "serverinfo.h"
#include "statistics.h"
#include "twitpool.h"
#include "twitpoollist.h"
#include "config.h"
#include "conn.h"
#include "util.h"
#include "error.h"



/**
 * The cleanupSayerConnectionHandler() function is responsible for cleaning up the resources associated with a sayerConnectionHandler thread.
 * The cleanupSayerConnectionHandler() function shall receive as argument a pointer to a struct connserverinfo structure.
 *
 * @return Nothing.
 */
static void cleanupSayerConnectionHandler( void *arg );

/**
 * The cleanupHearerConnectionHandler() function is responsible for cleaning up the resources associated with a hearerConnectionHandler thread.
 * The cleanupHearerConnectionHandler() function shall receive as argument a pointer to a struct connserverinfo structure.
 *
 * @return Nothing.
 */
static void cleanupHearerConnectionHandler( void *arg );

/**
 * The setupSayerConnectionHandler() shall perform all the necessary actions for the preparation of a sayerConnectionHandler thread.
 * The setupSayerConnectionHandler() function shall receive as argument a pointer to a struct connserverinfo structure.
 *
 * @return Nothing.
 */
static void setupSayerConnectionHandler( void *arg );

/**
 * The setupHearerConnectionHandler() shall perform all the necessary actions for the preparation of a hearerConnectionHandler thread.
 * The setupHearerConnectionHandler() function shall receive as argument a pointer to a struct connserverinfo structure.
 *
 * @return Nothing.
 */
static void setupHearerConnectionHandler( void *arg );

/**
 * The receivetwit() function shall receive a twit from the socket given as parameter into the buffer pointed to by parameter twit which shall not
 * be a NULL pointer. No more than nbytes shall be read into the buffer.
 *
 * @return The receivetwit() function shall return the number of bytes read; otherwise, -1 shall be returned meaning that 
 *	the connection must be closed with the socket.
 */
static ssize_t receivetwit( int sockfd, char *twit, size_t nbytes );

/**
 * The sendtwit() function shall send the specified twit to the hearer at the specified sockfd.
 *
 * @return The sendtwit() function shall return the number of bytes send if successful; otherwise, -1 shall be returned
 *	and errno shall be set to indicate the error.
 */
static ssize_t sendtwit( int sockfd, struct twit * restrict t );



/**
 * sayerConnectionHandler() is responsible for handling the connection with a sayer.
 * The design of sayerConnectionHandler() includes the following issues:
 *	+ Each sayer can send up to SAYER_TWIT_MAXCOUNT twits.
 *	+ To receive a byte from the sayer up to SAYER_WAIT_NSEC seconds will be elapsed.
 *	If this timeunit passes the connection is closed. Just assume that the sayer is "bad".
 *	+ If an error occurs while reading the connection is closed.
 */
void *sayerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;
	int howmanytwits = 0; // how many twits arrived. Must be intialized to zero
	ssize_t nread = 0; // how many bytes read from sayer
	size_t totaltwitcount; // how many twits in twitpool
	// If TWIT_MAXLEN gets too large it would be better if twit is malloced and got allocated on the heap
	// rather on the stack. 
	char twit[ TWIT_MAXLEN + 1 ];
	const size_t twitlen = sizeof( twit ) / sizeof( twit[ 0 ] );

	assert( csi != NULL );
	
	// Setup the connection handler
	// POSIX says that pthread_cleanup_push() and pthread_cleanup_pop() must appear as statements
	// and in pairs within the same lexical scope so pthread_cleanup_push() must be put here
 	// and not in setupSayerConnectionHandler()
	pthread_cleanup_push( &cleanupSayerConnectionHandler, csi );
	setupSayerConnectionHandler( csi );

	// Start receiving twits from the sayer
	while ( 1 ){
		// Get a single twit
		if ( howmanytwits >= SAYER_TWIT_MAXCOUNT ){
			// If the sayer exceeded the limit of twits it can send end here and close the connection
			break;
		}
		if (  ( nread = receivetwit( csi->csi_sockfd, twit, twitlen ) ) == -1 ){	
			break;
		}
		++howmanytwits;

		// Update the statistics; a twit arrived
		acquire_statistics( csi->csi_serverinfo );
		increaseArrivedTwitsNum( &csi->csi_serverinfo->si_stats );
		release_statistics( csi->csi_serverinfo );

		// Store the twit for the hearers to get
		acquire_twitpool( csi->csi_serverinfo );
		totaltwitcount = twitpoolcount( &csi->csi_serverinfo->si_twitpool );
		assert( totaltwitcount <= TWIT_MAXCOUNT );
		// Store the twit only if it is inside the limit set as TWIT_MAXCOUNT
		if ( totaltwitcount < TWIT_MAXCOUNT ){
			( void )putintwitpool( &csi->csi_serverinfo->si_twitpool, twit, ( size_t )nread );
			while ( pthread_cond_signal( &csi->csi_serverinfo->si_twitpool_cond ) ){ continue; }
		}
		release_twitpool( csi->csi_serverinfo );
	}

	// Cleanup code
	pthread_cleanup_pop( 1 );

	pthread_exit( NULL );
}

void *hearerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;
	struct twit t;
	int stop = 0;

	assert( csi != NULL );
	
	// Setup the connection handler
	// POSIX says that pthread_cleanup_push() and pthread_cleanup_pop() must appear as statements
	// and in pairs within the same lexical scope so pthread_cleanup_push() must be put here
 	// and not in setupHearerConnectionHandler()
	pthread_cleanup_push( &cleanupHearerConnectionHandler, csi );
	setupHearerConnectionHandler( csi );

	// Start sending twits
	while ( !stop ){
		// Wait for a twit
		acquire_twitpool_in_twitpoollist_node( csi->csi_tpln );
		while ( twitpoolisempty( &csi->csi_tpln->tpln_twitpool ) ){
			while ( pthread_cond_wait( &csi->csi_tpln->tpln_cond, &csi->csi_tpln->tpln_lock ) ){ continue; }
		}
		errno = 0;
		( void )getfromtwitpool( &csi->csi_tpln->tpln_twitpool, &t );
		assert( errno == 0 );
		release_twitpool_in_twitpoollist_node( csi->csi_tpln );

		// send the twit
		if ( sendtwit( csi->csi_sockfd, &t ) == -1 ){
			stop = 1;
		}

		// Update statistics; a twit was send
		acquire_statistics( csi->csi_serverinfo );
		increaseDeliveredTwitsNum( &csi->csi_serverinfo->si_stats );
		release_statistics( csi->csi_serverinfo );

		// free the twit
		free( t.t_twit );
	}

	// Cleanup code
	pthread_cleanup_pop( 1 );

	pthread_exit( NULL );
}



// Here follows the implementation of the local functions...

/** 
 * Cleanup everything from the sayer connection handler.
 * It must:
 *	1) Close the socket
 *	2) Update the statistics
 *		--> Decrease number of sayers since one hearer got away
 *		--> Decrease number of threads cause the thread is to be terminated
 *		--> Signal that a sayer was disconnected
 *	3) Free the csi we got from sayersListener().
 */
static void cleanupSayerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;

	assert( csi != NULL );

	// Cleanup code

	// Close the connection
	while ( shutdown( csi->csi_sockfd, SHUT_RD ) == -1 ){ continue; }
	( void )safe_close( csi->csi_sockfd );
	// Update the statistics that a sayer was disconnected
	acquire_statistics( csi->csi_serverinfo );
	decreaseSayersNum( &csi->csi_serverinfo->si_stats );
	decreaseThreadsNum( &csi->csi_serverinfo->si_stats );
	// Must also signal that a sayer was disconnected
	while ( pthread_cond_signal( &csi->csi_serverinfo->si_stats_sayers_cond ) ){ continue; }
	release_statistics( csi->csi_serverinfo );
	// Free the memory
	free( csi );

	return ;
}

/** 
 * Cleanup everything from the hearer connection handler.
 * It must:
 *	1) Close the socket
 *	2) Update the statistics
 *		--> Decrease number of hearers since one hearer got away
 *		--> Decrease number of threads cause the thread is to be terminated
 *		--> Signal that a hearer was disconnected
 *	3) Remove the twitpool from this hearer
 *	4) Free the csi we got from hearersListener().
 */
static void cleanupHearerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;

	assert( csi != NULL );

	// Cleanup code

	// Close the connection
	while ( shutdown( csi->csi_sockfd, SHUT_WR ) == -1 ){ continue; }
	( void )safe_close( csi->csi_sockfd );
	// Update the statistics that a hearer was disconnected
	acquire_statistics( csi->csi_serverinfo );
	decreaseHearersNum( &csi->csi_serverinfo->si_stats );
	decreaseThreadsNum( &csi->csi_serverinfo->si_stats );
	// Must also signal that a hearer was disconnected
	while ( pthread_cond_signal( &csi->csi_serverinfo->si_stats_hearers_cond ) ){ continue; }
	release_statistics( csi->csi_serverinfo );
	// Remove twitpool
	acquire_twitpool_list( csi->csi_serverinfo );
	( void )removefromtwitpoollist( &csi->csi_serverinfo->si_twitpool_list, csi->csi_tpln );
	release_twitpool_list( csi->csi_serverinfo );
	// Free the memory
	free( csi );

	return ;
}

/** 
 * Do the necessary preparations for the sayer connection handler.
 * These include:
 *	1) Setting a timeout for reading from the sayer. Obtain the timeunit from config.h
 */
static void setupSayerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;
	struct timeval timeout;

	assert( csi != NULL );

	// Set a timeout for reading from the sayer
	// Since POSIX says that this option may not be supported by all implementations i chose to ignore possible
 	// failure from setsockopt()
	( void )memset( &timeout, 0, sizeof( timeout ) );
	timeout.tv_sec = ( time_t )SAYER_WAIT_NSEC;
	timeout.tv_usec = ( suseconds_t )0;
	( void )setsockopt( csi->csi_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
	
	return ;
}

/** 
 * Do the necessary preparations for the hearer connection handler.
 * These include:
 *	1) Setting a timeout for writing to the hearer. Obtain the timeunit from config.h
 */
static void setupHearerConnectionHandler( void *arg ){
	struct connserverinfo *csi = ( struct connserverinfo * )arg;
	struct timeval timeout;

	assert( csi != NULL );

	// Set a timeout for writing to a hearer
	// Since POSIX says that this option may not be supported by all implementations i chose to ignore possible
 	// failure from setsockopt()
	( void )memset( &timeout, 0, sizeof( timeout ) );
	timeout.tv_sec = ( time_t )HEARER_WAIT_NSEC;
	timeout.tv_usec = ( suseconds_t )0;
	( void )setsockopt( csi->csi_sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof( timeout ) );	

	return ;
}

/**
 * Receive a twit from the socket.
 * These limitations must be taken into consideration:
 *	+  The timeout for read
 *	+ The limit of nbytes
 *	+ A twit can end with a nul byte
 */
static ssize_t receivetwit( int sockfd, char *twit, size_t nbytes ){
	size_t nread_total = 0; // How many bytes are read totally
	ssize_t nread_cur = 0; // return from recv
	int putnulbyte = 1; // whether a nul byte must be put at the end or not

	assert( twit != NULL );
	assert( nbytes > 0 );

	nread_total = 0;
	do{
		// Receive a byte from the sayer
		errno = 0;
		nread_cur = recv( sockfd, twit + nread_total, 1, 0 );
		if ( nread_cur == -1 ){
			// If we got interrupted just continue
			if ( errno == EINTR ){			
				continue;
			}
			// In any other case stop here and close the connection later
			return ( -1 );
		}
		else if ( nread_cur == 0 ){
			// No byte available and the peer has performed an orderly shutdown so stop here
			return ( -1 );
		}
		assert( nread_cur == 1 );
		// If this is '\0' the twit is ended here
		if ( *( twit + nread_total ) == '\0' ){
			putnulbyte = 0;
			break;
		}
		nread_total += nread_cur;
	}while ( nread_total < nbytes -1 ); // careful to put nbytes - 1 here cause we must put the nul byte at the end of the string

	// Place the nul byte if needed
	if ( putnulbyte ){
		*( twit + nread_total ) = '\0';
	}

	// Return how many bytes received
	return ( nread_total );
}

static ssize_t sendtwit( int sockfd, struct twit * restrict t ){
	ssize_t nsend_total;
	ssize_t nsend_cur;

	assert( t != NULL );

	nsend_total = 0;
	do{
		errno = 0;
		nsend_cur = send( sockfd, ( char * )t->t_twit + nsend_total, t->t_twitlen - nsend_total, 0 );
		if ( nsend_cur == -1 ){
			if ( errno == EINTR ){
				continue;
			}
			return ( -1 );
		}
		else if ( nsend_cur == 0 ){
			return ( -1 );
		}
		nsend_total += nsend_cur;
	}while ( nsend_total < t->t_twitlen );

	return ( nsend_total );
}
