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
 * \file twithear.c
 *
 * File twithear.c contains the implementation of the twithear program that is part of the project in Operating Systems at TUC.
 *
 * Usage:
 *	twithear ipaddr port timeunit
 * , where ipaddr and port define to which addr and port the twithear program will connect to and timeunit specifies the interval
 * by which the twithear program will receive a byte from the server.
 *
 * The twithear program does the following simple job:
 *	1) Connects to the twitserver (to the addr and port given as command line arguments)
 *	2) Starts receiving twits from the twitserver and prints them to stdout.
 * and it terminates on the arrival of a SIGINT signal (Control-C).
 *
 *
 * @author Tassos Souris
 */
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <sys/types.h>
#include "connect.h"
#include "error.h"



/**
 * The usage() function shall display to stderr information about the usage of the program and exit with exit status EXIT_FAILURE. 
 * The name of the program is pointed to by parameter programname which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param programname Pointer to the name of the program.
 */
static void usage( const char * restrict programname );

/**
 * The signal_handler() function is used to terminate the twithear program safely on the arrival of various signals.
 *
 * @return Nothing.
 * @param signum The signal arrived.
 */
static void signal_handler( int signum );

/**
 * The setup_signal_handling() function shall initialize the handling needed for various signals.
 *
 * @return Nothing.
 */
static void setup_signal_handling( void );



// This is "signalled" by the signal_handler() and tells main to terminate
static jmp_buf jmpbuf_info;



int main( int argc, char *argv[] ){
	const char *addr = NULL;
	const char *port = NULL;
	int timeunit;
	int sockfd = -1; // Must initialize to -1 cause it is used as an error flag later
	int status = EXIT_SUCCESS; // Must initialize to EXIT_SUCCESS (at the beginning to error has occured)
	
	// Verify that user gave the appropriate arguments
	if ( argc != 4 ){
		usage( argv[ 0 ] );
	}

	// Retrieve the command line arguments
	addr = argv[ 1 ];
	port = argv[ 2 ];	
	timeunit = atoi( argv[ 3 ] );
	assert( timeunit >= 0 );

	// Set up the signal handling now
	setup_signal_handling();
	
	if ( setjmp( jmpbuf_info ) == 0 ){
		// Connect to the twitserver
		if ( ( sockfd = connect_to_twitserver( addr, port ) ) == -1 ){ 
			status = EXIT_FAILURE; 
		}
		// Start receiving twits from the twitserver
		else if ( recv_from_twitserver( sockfd, timeunit ) == -1 ){ 
			status = EXIT_FAILURE;
		}
	}

	// Clean-up code

	// Disconnect from the twitserver
	if ( sockfd != -1 && disconnect_from_twitserver( TwitClientType_HEARER, sockfd ) == -1 ){ status = EXIT_FAILURE; }

	// Return
	exit( status );
}



// Display usage info and exit
static void usage( const char * restrict programname ){
	assert( programname != NULL );
	error( "usage: %s addr port timeunit\n", programname );
	exit( EXIT_FAILURE );
}

// This is called when various signals arrive. It "signals" main that it must terminate safelly...
static void signal_handler( int signum ){
	longjmp( jmpbuf_info, 1 );
}

// Do what is necessary to handle the signals appropriately
static void setup_signal_handling( void ){
#define REGISTER_SIGNAL( signum ) do{	\
	while( sigaction( (signum), &sig_action, NULL ) == -1 ){ continue; }	\
}while( 0 )
	sigset_t sigset;
	struct sigaction sig_action;

	while ( sigfillset( &sigset ) == -1 ){ continue; }

	( void )memset( &sig_action, 0, sizeof( sig_action ) );
	sig_action.sa_handler = &signal_handler;
	sig_action.sa_mask = sigset;

	REGISTER_SIGNAL( SIGHUP );
	REGISTER_SIGNAL( SIGINT );
	REGISTER_SIGNAL( SIGTERM );
	REGISTER_SIGNAL( SIGQUIT );

	return ;
#undef REGISTER_SIGNAL
}
