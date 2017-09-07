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
 * \file twitspeak.c
 *
 * File twitspeak.c contains the implementation of the twitspeak program that is part of the project in Operating Systems at TUC.
 *
 * Usage:
 *	twitspeak ipaddr port timeunit
 * , where ipaddr and port define to which addr and port the twithear program will connect to and timeunit how much the twitspeak program 
 * will wait before sending the next message.
 *
 * The twitspeak program performs the following simple steps:
 *	1) Read lines from stdin
 *	For each line read do:
 *		Call program twitsay with that line
 *
 * @author Tassos Souris
 */
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "read.h"
#include "error.h"
#include "config.h"
#include "util.h"



/**
 * The usage() function shall display to stderr information about the usage of the program and exit with exit status EXIT_FAILURE. 
 * The name of the program is pointed to by parameter programname which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param programname Pointer to the name of the program.
 */
static void usage( const char * restrict programname );



/**
 * The setup_signal_handling() function shall initialize the handling needed for various signals.
 *
 * @return Nothing.
 */
static void setup_signal_handling( void );



/**
 * The call_twitsay() function shall call the twitsay program with the given addr and port to send the nbytes bytes from the buffer
 * pointed to by parameter buf. No parameter shall be a NULL pointer. The call_twitsay() function shall write to stderr any error message.
 *
 * @return The call_twitsay() function shall return zero if successful; otherwise, -1 shall be returned.
 */
static int call_twitsay( const char * restrict addr, const char * restrict port, const char * restrict buf, size_t nbytes );



int main( int argc, char *argv[] ){
	const char *addr = NULL;
	const char *port = NULL;
	int status = EXIT_SUCCESS;
	ssize_t nbytes;
	unsigned int timeunit = 0;
	char twit[ TWIT_MAXLEN + 1 ];
	size_t twitlen = sizeof( twit ) / sizeof( twit[ 0 ] );

	// Verify that user gave the appropriate arguments
	if ( argc != 4 ){
		usage( argv[ 0 ] );
	}

	// Retrieve the command line arguments
	addr = argv[ 1 ];
	port = argv[ 2 ];
	timeunit = atoi( argv[ 3 ] );
	assert( timeunit >= 0 );

	// Must catch SIGPIPE cause the twitsay child process might terminate before it gets EOF and thus the one end of the pipe will be closed
 	// and we will get a SIGPIPE
	setup_signal_handling();

	// Get a line from stdin
	while ( ( nbytes = gettwitfromfile( stdin, twit, twitlen ) ) != ( ssize_t )-1 ){
		sleep( timeunit );
		printf( "%s", twit) ;
		// Call twitsay with that line
		if ( call_twitsay( addr, port, twit, ( size_t )nbytes ) == -1 ){
			error( "call_twitsay() failed: (%s)\n", strerror( errno ) );
			break;
		} 
	}

	exit( status );
}



// Display usage info and exit
static void usage( const char * restrict programname ){
	assert( programname != NULL );
	error( "usage: %s addr port timeunit\n", programname );
	exit( EXIT_FAILURE );
}

// Do what is necessary to handle the signals appropriately
static void setup_signal_handling( void ){
	sigset_t sigset;
	struct sigaction sig_action;

	while ( sigfillset( &sigset ) == -1 ){ continue; }

	( void )memset( &sig_action, 0, sizeof( sig_action ) );
	sig_action.sa_handler = SIG_IGN;
	sig_action.sa_mask = sigset;

	while( sigaction( SIGPIPE, &sig_action, NULL ) == -1 ){ continue; }

	return ;
}

// Call twitsay to send to twitserver the twit
static int call_twitsay( const char * restrict addr, const char * restrict port, const char * restrict buf, size_t nbytes ){
	pid_t pid;
	int status = 0;
	int pipefd[ 2 ];
	siginfo_t siginfo;

	assert( addr != NULL );
	assert( port != NULL );
	assert( buf != NULL );

	while ( pipe( pipefd ) == -1 ){ continue; }

	errno = 0;
	switch( pid = fork() ){
	case 0: // in child process
		while ( dup2( pipefd[ 0 ], STDIN_FILENO ) == -1 ){ continue; }
		while ( safe_close( pipefd[ 0 ] ) == -1 ){ continue; }
		while ( safe_close( pipefd[ 1 ] ) == -1 ){ continue; }
		errno = 0;
		execlp( TWITSAY_PROGNAME, TWITSAY_PROGNAME, addr, port, ( char *)0 );
		error( "exec() failed: (%s)\n", strerror( errno ) );
		_exit( EXIT_FAILURE );
		break;
	case -1: // fork() failed
		error( "could not call " TWITSAY_PROGNAME ". fork() failed: (%s)\n", strerror( errno ) );
		status = -1;
		break;
	default: // in parent process
		// write the message to the pipe so that the child process will read it
		while ( safe_close( pipefd[ 0 ] ) == -1 ){ continue; }
		errno = 0;
		if ( writeall( pipefd[ 1 ], buf, nbytes ) != ( ssize_t )nbytes ){
			error( "could not pass (all?) the bytes to " TWITSAY_PROGNAME "\n" );
		}
		while ( safe_close( pipefd[ 1 ] ) == -1 ){ continue; }
		// wait for the child process		
		while ( waitid( P_PID, pid, &siginfo, WEXITED ) == -1 ){ continue; }
		error( TWITSAY_PROGNAME " exited with exit status %d\n", siginfo.si_status );
		break;
	}

	return ( status );
}
