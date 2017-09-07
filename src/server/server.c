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
 * \file server.c
 *
 * File server.c contains the implementation of the server's main part.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "serverinfo.h"
#include "listen.h"
#include "init.h"
#include "sighandling.h"
#include "statistics.h"
#include "twitpool.h"
#include "config.h"
#include "util.h"
#include "error.h"



/**
 * The server is divided into several subsystems where each subsystem consists of one or more threads.
 *
 * The threads that exist in the server are:
 *	1) A thread responsible for accepting connections from hearers
 *	2) A thread responsible for accepting connections from sayers
 *	3) A thread responsible for handling signals
 *	4) A thread responsible for updating the statistics every N seconds
 *	5) One thread for each sayer and hearer connected to the server
 *	6) A thread that retrieves the twits that are stored "globally" and sends them to each of the hearers
 *
 * + The thread that is responsible for handling signals will inform the other threads that they must terminate normally
 * if requested so in the arrival of a SIGQUIT signal and after the user has confirmed termination of the server.
 * + Almost all threads must update the statistics; e.g the thread listening for sayers must update the statistics if a new
 * sayer connects to the server. 
 * + The threads related to the connections must share the messages data structure so as the sayers's threads will store them and the
 * hearers's threads will retrieve them to send them.
 * + Some threads inform their "parent" for when they have started successfully or not; e.g when the main thread starts the thread
 * that listens for sayers connections if the later fails to set up the socket etc etc it will inform the main thread that it
 * failed so as the main thread terminates the server. When the thread succeeded in all the preparations it will also inform the main thread.
 *
 * The information needed to be shared by the various threads is stored in an object of the serverinfo structure that is passed
 * to the threads as parameter when they are created.
 */

/**
 * The handle_termination() function shall handle the termination of the server in 
 * the arrival of various signals (e.g SIGKILL).
 *
 * @return The handle_termination() function shall return nonzero if termination must occur; otherwise, zero shall be returned.
 */
static int handle_termination( void );

/**
 * The cleanup_server() function shall perform the necessary cleanup operations for the server before exit.
 *
 * @return Nothing.
 */
static void cleanup_server( struct serverinfo * restrict si );

/**
 * The print_statistics() function shall print the statistics to stdout.
 *
 * @return Nothing.
 */
static void print_statistics( const struct statistics * restrict stats );

/**
 * The discardline() function shall consume bytes from fp until EOF or the newline character is encountered.
 *
 * @return Nothing.
 */
static void discardline( FILE *fp );



/**
 * main() is responsible for setting up the stuff in the server and waiting for signals that it handles
 * appropriately.
 *
 * Specifically it starts the following threads:
 *	1) The thread responsible for accepting connections from hearers
 *	2) The thread responsible for accepting connections from sayers
 *	3) The thread responsible for updating the statistics every N seconds
 * For each of the above threads it waits for their preparation status.
 *
 * The main thread also is responsible for handling some signals. On the arrival of a SIGQUIT signal it prints the statistics
 * and when it gets interrupted with Control-C (SIGINT) it asks user if it wants the server to terminate and continues
 * appropriately. The last approach is also applied to other signals.
 */
int main( int argc, char *argv[] ){	
	struct serverinfo si;
	sigset_t sigset;
	int signum;
	
	// Set up signal handling
	setup_signals( &sigset );

	// Protect initialization from signals
	block_signals();

	// Setup the threads
	if ( initializeServer( &si ) == -1 ){
		error( "Server failed to be initialized.\nExiting now...\n" );
		exit( EXIT_FAILURE );
	}
	printf( "Server got initialized successfully.\n" );
	fflush( stdout );

	// Unblock the signals
	unblock_signals();

	// Wait here for signals
	while ( 1 ){
		errno = sigwait( &sigset, &signum );
		assert( errno == 0 );
		printf( "\nSignal %d arrived.\n", signum );
		fflush( stdout );
		switch ( signum ){
		case SIGQUIT:
			acquire_statistics( &si );
			acquire_twitpool( &si );
			si.si_stats.stats_storedTwitsNum = twitpoolcount( &si.si_twitpool );
			release_twitpool( &si );
			print_statistics( &si.si_stats );
			release_statistics( &si );
			break;
		case SIGKILL:
			// Fall through
		default:
			// Ask for termination
			if ( handle_termination() ){
				// Do cleanup and exit
				cleanup_server( &si );
				exit( EXIT_SUCCESS );
			}
			break;
		}
	}

	// Not Reached
	exit( EXIT_SUCCESS );
}



// Print the statistics to stdout. This function assumes that the structure is locked.
static void print_statistics( const struct statistics * restrict stats ){
	
	assert( stats != NULL );

	printf( "Server statistics:\n"
		"------------------\n"
		"Active threads = %d\n"
		"Hear connections = %d\n"
		"Say connections = %d\n"
		"Twits currently stored = %d\n"
		"Total twits arrived = %d\n"
		"Total twits delivered = %d\n"
		"Average incoming rate = %f\n"
		"Average outcoming rate = %f\n"
		"\n\n",
		stats->stats_threadsNum,
		stats->stats_hearersNum,
		stats->stats_sayersNum,
		stats->stats_storedTwitsNum,
		stats->stats_arrivedTwitsNum,
		stats->stats_deliveredTwitsNum,
		stats->stats_averageTwitsIncomingRate,
		stats->stats_averageTwitsOutcomingRate
	);
	fflush( stdout );

	return ;
}

// Ask user if server is to be terminated or not
static int handle_termination( void ){
	char buffer[ 2 ];
	static const size_t buffersize = sizeof( buffer ) / sizeof( buffer[ 0 ] );
	int toTerminate = 0; // Must be initialized to zero

	printf( "Server it to be terminated. Are you sure? [y/n] " );
	fflush( stdout );

	errno = 0;
	fgets( buffer, buffersize, stdin );
	if ( feof( stdin ) ){
		error( "stdin at EOF.\nExiting now...\n" );
		// Return here cause discardline() will be called later and we are at eof now
		return ( 1 );
	}
	else if ( ferror( stdin ) ){
		error( "Error with stdin (%s).\nExiting now...\n", strerror( errno ) );
		// Return here cause discardline() will be called later and stdin is in error now
		return ( 1 );
	}
	else if ( !strcmp( buffer, "y" ) ){
		// Do not return here to call discardline() later
		toTerminate = 1;
	}
	else if ( !strcmp( buffer, "n" ) ){
		// Do not return here to call discardline() later
		 toTerminate = 0;
	}
	else{
		// Do not return here to call discardline() later
		printf( "Invalid character. Continuing now...\n" );
		fflush( stdout );
		toTerminate = 0;
	}

	discardline( stdin );

	// Do not terminate
	return ( toTerminate );
}

// Discard a line from fp
static void discardline( FILE *fp ){
	int ch;

	while ( ( ch = fgetc( fp ) ) !=  -1 ){
		if ( ch == '\n' ){
			break;
		}
	}

	return ;
}

// Do cleanup for the server
static void cleanup_server( struct serverinfo * restrict si ){
	assert( si != NULL );

	// Stop the threads
	( void )pthread_cancel( si->si_statistics_updater_threadid );
	( void )pthread_cancel( si->si_twitpool_consumer_threadid );
	( void )pthread_cancel( si->si_sayers_listener_threadid );
	( void )pthread_cancel( si->si_hearers_listener_threadid );

	// Destroy mutexes and conditions
	( void )pthread_cond_destroy( &si->si_stats_sayers_cond );
	( void )pthread_cond_destroy( &si->si_stats_hearers_cond );
	( void )pthread_cond_destroy( &si->si_prepared_cond );
	( void )pthread_cond_destroy( &si->si_twitpool_cond );
	( void )pthread_mutex_destroy( &si->si_stats_lock );
	( void )pthread_mutex_destroy( &si->si_prepared_lock );
	( void )pthread_mutex_destroy( &si->si_twitpool_lock );
	( void )pthread_mutex_destroy( &si->si_twitpool_list_lock );

	// Destroy the twitpool
	( void )deltwitpool( &si->si_twitpool );

	// Destroy the twitpool list
	( void )deltwitpoollist( &si->si_twitpool_list );

	return ;
}
