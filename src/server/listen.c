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
 * \file listen.c
 *
 * File listen.c contains the implementation of the listen.h interface.
 *
 * @author Tassos Souris
 */
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include "serverinfo.h"
#include "listen.h"
#include "conn.h"
#include "config.h"
#include "util.h"
#include "error.h"

struct listenerinfo{
	struct serverinfo *li_serverinfo;
	pthread_attr_t li_threadattr;
	int li_sockfd;
};

/**
 * The setupHearersListener() shall perform all the necessary actions for the preparation of the hearersListener thread.
 * The setupHearersListener() function shall receive as argument a pointer to a struct listenerinfo object.
 *
 * @return Nothing.
 */
static void setupHearersListener( void *arg );

/**
 * The setupSayersListener() shall perform all the necessary actions for the preparation of the sayersListener thread.
 * The setupSayersListener() function shall receive as argument a pointer to a struct listenerinfo object.
 *
 * @return Nothing.
 */
static void setupSayersListener( void *arg );

/**
 * The cleanupHearersListener() function is responsible for cleaning up the resources associated with the hearersListener thread.
 * The cleanupHearersListener() function shall receive as argument a pointer to a struct listener info object.
 *
 * @return Nothing.
 */
static void cleanupHearersListener( void *arg );

/**
 * The cleanupSayersListener() function is responsible for cleaning up the resources associated with the sayersListener thread.
 * The cleanupSayersListener() function shall receive as argument a pointer to a struct listener info object.
 *
 * @return Nothing.
 */
static void cleanupSayersListener( void *arg );



// Create a socket to listen in the local machine at the specified port
int prepareListenerSocket( int port ){
	struct addrinfo *infop = NULL; // Must be initialized to NULL cause it is used as an error flag
	struct addrinfo hint;
	int sockfd = -1; // Must be initialized to -1 cause it is used as an error flag
	int cleanup = 0; // Must be initialized to 0. If it becomes 1 then cleanup must be performed before return from the function
	int status = -1; // Must be initialized to -1. If all ok then it will get the value of the sockfd variable.
	char hostnamebuffer[ 256 ]; // POSIX defines _POSIX_HOST_NAME_MAX as 255 without the terminating nul byte so +1 here
	char portbuffer[ 10 ]; // 10 seems too much but just to be safe...
	const size_t hostnamebuffersize = sizeof( hostnamebuffer ) / sizeof( hostnamebuffer[ 0 ] );
	const size_t portbuffersize = sizeof( portbuffer ) / sizeof( portbuffer[ 0 ] );

	do{
		// Create the socket
		errno = 0;
		if ( ( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ){ 
			error( "socket() failed in prepareListenerSocket() (%s).\n", strerror( errno ) );
			cleanup = 1; 
			break; 
		}

		// Bind the socket with the address
		( void )memset( portbuffer, 0, portbuffersize );
		( void )memset( hostnamebuffer, 0, hostnamebuffersize );
		( void )memset( &hint, 0, sizeof( hint ) );
		hint.ai_flags = AI_PASSIVE;
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = 0;
		if ( gethostname( hostnamebuffer, hostnamebuffersize ) == -1 ){ 
			error( "gethostname() failed in prepareListenerSocket().\n" );
			cleanup = 1; 
			break; 
		}
		errno = 0;
		if ( snprintf( portbuffer, portbuffersize, "%d", port ) == -1 ){ 
			error( "snprintf() failed in prepareListenerSocket() (%s).\n", strerror( errno ) );
			cleanup = 1; 
			break; 
		}
		if ( ( errno = getaddrinfo( hostnamebuffer, portbuffer, &hint, &infop ) ) ){ 
			error( "getaddrinfo() failed in prepareListenerSocket() (%s).\n", gai_strerror( errno ) );
			cleanup = 1; 
			break; 
		}
		errno = 0;
		if ( bind( sockfd, infop->ai_addr, infop->ai_addrlen ) == -1 ){ 
			error( "bind() failed in prepareListenerSocket() (%s).\n", strerror( errno ) );
			cleanup = 1; 
			break; 
		}

		// Make the socket listening for connections
		errno = 0;
		if ( listen( sockfd, SOCKET_BACKLOG ) == -1 ){ 
			error( "listen() failed in prepareListenerSocket() (%s).\n", strerror( errno ) );
			cleanup = 1; 
			break; 
		}

		// If we reach here then all is ok and we must change the value of status to reflect the socket
 		status = sockfd;
	}while ( 0 );

	// Look here for cleaning up
	if ( cleanup ){
		if ( sockfd != -1 ){
			// Not interested in safe_close() here so save errno
			int saved_errno = errno;
			( void )safe_close( sockfd );
			errno = saved_errno;
		}
	}
	// Note that infop must be cleared even if the function was successful	
	if ( infop != NULL ){
		freeaddrinfo( infop );
	}

	// Return the socket. Note that if the socket was not created it has a value of -1
	return ( status );
}

/**
 * sayersListener() runs on its own thread and is responsible for accepting connections from sayers. 
 * The port to which the sayersListener() function will listen for sayers is obtained from config.h (SAYERS_PORT).
 * The steps the sayersListener() function takes are:
 *	1) The socket that will listen for sayers at the port SAYERS_PORT is created.
 *	2) If successfull (the above step) the sayersListener() function notifies through the serverinfo structure
 *	passed as parameter that is prepared.
 *	3) It waits for a connection from a sayer and if a connection arrives: 
 *		1) Start a new thread that handles the connection with the sayer sayerConnectionHandler().
 *		2) Update the statistics structure (a new sayer arrived).
 */
void *sayersListener( void *arg ){
	struct serverinfo *si = ( struct serverinfo * )arg;
	struct connserverinfo *csi = NULL; // Malloced each time a connection arrives
	pthread_t threadid; // Used for the threads created to handle the connections
	int connsockfd = -1; // The socket from each connection arriving
	struct listenerinfo li = {
		.li_serverinfo = si
	};

	assert( si != NULL );

	// Setup the listener
	// Setup the cleanup
	// POSIX says that pthread_cleanup_push() and pthread_cleanup_pop() must appear as statements
	// and in pairs within the same lexical scope so pthread_cleanup_push() must be put here
 	// and not in setupSayersListener()
	pthread_cleanup_push( &cleanupSayersListener, &li );
	setupSayersListener( &li );
	
	// Wait for connections
	while ( 1 ){
		// Do not accept any more connections if we are full of sayers
		acquire_statistics( si );
		assert( si->si_stats.stats_sayersNum <= SAYERS_MAXCOUNT );		
		while ( si->si_stats.stats_sayersNum == SAYERS_MAXCOUNT ){
			while ( pthread_cond_wait( &si->si_stats_sayers_cond, &si->si_stats_lock ) ){ continue; }
		}
		assert( si->si_stats.stats_sayersNum < SAYERS_MAXCOUNT );
		release_statistics( si );

		errno = 0;
		if ( ( connsockfd = accept( li.li_sockfd, NULL, NULL ) ) == -1 ){
			error( "accept() failed in sayersListener() (%s)\n", strerror( errno ) );
			continue;
		}
		// A connection arrived. Create a new struct connserverinfo object
 		// for the new thread
		errno = 0;
		if ( ( csi = malloc( sizeof( *csi ) ) ) == NULL ){
			error( "malloc() failed in sayersListener() (%s)\n", strerror( errno ) );
			// must close the socket here
			safe_close( connsockfd );
			continue;
		}
		csi->csi_serverinfo = si;
		csi->csi_sockfd = connsockfd;

		// CAUTION: the statistics must be locked before the thread is created cause in case the connection gets closed
		// before the nums are increased here and the created thread decreases the nums then we have an error.
 		// So lock the statistics beforehand to avoid any race condition		
		acquire_statistics( si );
		// Create the new thread to handle the connection
		if ( ( errno = pthread_create( &threadid, &li.li_threadattr, &sayerConnectionHandler, csi ) ) ){
			error( "pthread_create() failed in sayersListener() (%s)\n", strerror( errno ) );
			// must close the socket here
			safe_close( connsockfd );
			// note that if the thread failed to be created we must free the memory for csi here
			// otherwise, the thread must free the memory itself
			free( csi );
		}
		else{
			// Update the statistics that a new sayer was connected
			increaseSayersNum( &si->si_stats );
			increaseThreadsNum( &si->si_stats );
		}
		release_statistics( si );
	}

	// Perform cleanup
	pthread_cleanup_pop( 1 );

	// Not Reached
	pthread_exit( NULL );
}

/**
 * hearersListener() runs on its own thread and is responsible for accepting connections from hearers. 
 * The port to which the hearersListener() function will listen for hearers is obtained from config.h (HEARERS_PORT).
 * The steps the hearersListener() function takes are:
 *	1) The socket that will listen for hearers at the port HEARERS_PORT is created.
 *	2) If successfull (the above step) the hearersListener() function notifies through the serverinfo structure
 *	passed as parameter that is prepared.
 *	3) It waits for a connection from a hearer and if a connection arrives: 
 *		1) Start a new thread that handles the connection with the hearer hearerConnectionHandler().
 *		2) Update the statistics structure (a new hearer arrived).
 */
void *hearersListener( void *arg ){
	struct serverinfo *si = ( struct serverinfo * )arg;
	struct connserverinfo *csi = NULL; // Malloced each time a connection arrives
	struct twitpoollist_node *tpln = NULL; // used for the twitpoollist_node of each hearer
	pthread_t threadid; // Used for the threads created to handle the connections
	int connsockfd = -1; // The socket from each connection arriving
	struct listenerinfo li = {
		.li_serverinfo = si
	};

	assert( si != NULL );

	// Setup the listener
	// Setup the cleanup
	// POSIX says that pthread_cleanup_push() and pthread_cleanup_pop() must appear as statements
	// and in pairs within the same lexical scope so pthread_cleanup_push() must be put here
 	// and not in setupHearersListener()
	pthread_cleanup_push( &cleanupHearersListener, &li );
	setupHearersListener( &li );

	// Wait for connections
	while ( 1 ){
		// Do not accept any more connections if we are full of hearers
		acquire_statistics( si );
		assert( si->si_stats.stats_hearersNum <= HEARERS_MAXCOUNT );		
		while ( si->si_stats.stats_hearersNum == HEARERS_MAXCOUNT ){
			while ( pthread_cond_wait( &si->si_stats_hearers_cond, &si->si_stats_lock ) ){ continue; }
		}
		assert( si->si_stats.stats_hearersNum < SAYERS_MAXCOUNT );
		release_statistics( si );

		errno = 0;
		if ( ( connsockfd = accept( li.li_sockfd, NULL, NULL ) ) == -1 ){
			error( "accept() failed in hearersListener() (%s)\n", strerror( errno ) );
			continue;
		}
		// A connection arrived. Create a new struct connserverinfo object
 		// for the new thread
		errno = 0;
		if ( ( csi = malloc( sizeof( *csi ) ) ) == NULL ){
			error( "malloc() failed in hearersListener() (%s)\n", strerror( errno ) );
			// must close the socket here
			safe_close( connsockfd );
			continue;
		}
		
		// Acquire ownership of the twitpool list
		acquire_twitpool_list( si );
		errno = 0;
		// Create a twitpool for that hearer
		if ( newtwitpool( &si->si_twitpool_list, &tpln ) == -1  ){
			error( "newtwitpool() failed in hearersListener() (%s)\n", strerror( errno ) );
			// do cleanup work
			// close the socket
			safe_close( connsockfd );
			// free structure
			free( csi );

			// CAUTION: do **not** forget to unlock the list here
			// Release ownership of the twitpool list
			release_twitpool_list( si );

			continue;
		}
		// Release ownership of the twitpool list
		release_twitpool_list( si );

		csi->csi_serverinfo = si;
		csi->csi_sockfd = connsockfd;
		csi->csi_tpln = tpln;

		// CAUTION: the statistics must be locked before the thread is created cause in case the connection gets closed
		// before the nums are increased here and the created thread decreases the nums then we have an error.
 		// So lock the statistics beforehand to avoid any race condition		
		acquire_statistics( si );
		// Create the new thread to handle the connection
		if ( ( errno = pthread_create( &threadid, &li.li_threadattr, &hearerConnectionHandler, csi ) ) ){
			error( "pthread_create() failed in hearersListener() (%s)\n", strerror( errno ) );
			// must close the socket here
			safe_close( connsockfd );
			// note that if the thread failed to be created we must free the memory for csi here
			// otherwise, the thread must free the memory itself
			free( csi );

			// must also remove the twitpool created for the hearer
			( void )removefromtwitpoollist( &si->si_twitpool_list, tpln );
		}
		else{
			// Update the statistics that a new hearer was connected
			increaseHearersNum( &si->si_stats );
			increaseThreadsNum( &si->si_stats );
		}
		release_statistics( si );
	}

	// Perform cleanup
	pthread_cleanup_pop( 1 );

	// Not Reached
	pthread_exit( NULL );
}

// Prepare the socket for listening for sayers
int prepareSayersListenerSocket( void ){
	// Obtain the port from config.h and delegate to prepareListenerSocket()
	return ( prepareListenerSocket( SAYERS_PORT ) );
}

// Prepare the socket for listening for hearers
int prepareHearersListenerSocket( void ){
	// Obtain the port from config.h and delegate to prepareListenerSocket()
	return ( prepareListenerSocket( HEARERS_PORT ) );
}



// Implementation of local functions...

// Setup the hearer listener 
static void setupHearersListener( void *arg ){
	struct listenerinfo *li = ( struct listenerinfo * )arg;

	assert( li != NULL );

	// Prepare the socket to listen for hearers
	errno = 0;
	if ( ( li->li_sockfd = prepareHearersListenerSocket() ) == -1 ){
		error( "failed to prepare the socket for hearers in hearersListener() (%s)\n", strerror( errno ) );
		// Could not prepare so now inform about the status and terminate
		signal_prepared_status( li->li_serverinfo, 0 );
		pthread_exit( NULL );
	}
	// The preparation is successful
	signal_prepared_status( li->li_serverinfo, 1 );
	
	// Set up the thread attribute
	while ( pthread_attr_init( &li->li_threadattr ) ){ continue; }
	while ( pthread_attr_setdetachstate( &li->li_threadattr, PTHREAD_CREATE_DETACHED ) ){ continue; }

	return ;
}

// Setup the sayer listener
static void setupSayersListener( void *arg ){
	struct listenerinfo *li = ( struct listenerinfo * )arg;

	assert( li != NULL );

	// Prepare the socket to listen for sayers
	if ( ( li->li_sockfd = prepareSayersListenerSocket() ) == -1 ){
		error( "failed to prepare the socket for sayers in sayersListener() (%s)\n", strerror( errno ) );
		// Could not prepare so now inform about the status and terminate
		signal_prepared_status( li->li_serverinfo, 0 );
		pthread_exit( NULL );
	}
	// The preparation is successful
	signal_prepared_status( li->li_serverinfo, 1 );

	// Set up the thread attribute
	while ( pthread_attr_init( &li->li_threadattr ) ){ continue; }
	while ( pthread_attr_setdetachstate( &li->li_threadattr, PTHREAD_CREATE_DETACHED ) ){ continue; }

	return ;
}

// Cleanup the hearers listener
static void cleanupHearersListener( void *arg ){
	struct listenerinfo *li = ( struct listenerinfo * )arg;

	assert( li != NULL );
	
	// Cleanup code
	if ( li->li_sockfd != -1 ){
		( void )safe_close( li->li_sockfd );
	}
	while ( pthread_attr_destroy( &li->li_threadattr ) ){ continue; }


	return ;
}

// Cleanup the sayers listener
static void cleanupSayersListener( void *arg ){
	struct listenerinfo *li = ( struct listenerinfo * )arg;

	assert( li != NULL );

	// Cleanup code
	if ( li->li_sockfd != -1 ){
		( void )safe_close( li->li_sockfd );
	}
	while ( pthread_attr_destroy( &li->li_threadattr ) ){ continue; }

	return ;
}
