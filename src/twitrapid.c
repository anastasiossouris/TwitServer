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
 * \file twitrapid.c
 *
 * File twitrapid.c contains the implementation of the twitrapid program that is part of the project in Operating Systems at TUC.
 *
 * Usage:
 *	twitrapid ipaddr port filename timeunit
 * , where ipaddr and port define to which addr and port the twitrapid program will connect to and filename is the name of the file
 * that will be used to read the twits from and timeunit specifies the time interval by which the twitrapid sends twits to the server.
 * If filename is "stdin" then stdin will be used for input.
 *
 * The twitrapid program connects to the twitserver and sends a continuous stream of twits to the twitserver.
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
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "connect.h"
#include "read.h"
#include "config.h"
#include "error.h"
#include "util.h"



/**
 * The usage() function shall display to stderr information about the usage of the program and exit with exit status EXIT_FAILURE. 
 * The name of the program is pointed to by parameter programname which shall not be a NULL pointer.
 *
 * @return Nothing.
 * @param programname Pointer to the name of the program.
 */
static void usage( const char * restrict programname );



int main( int argc, char *argv[] ){
	const char *addr = NULL;
	const char *port = NULL;
	const char *filename = NULL;
	FILE *fp = NULL; // Must initialize to NULL cause it is used as an error flag later
	int sockfd = -1; // Must initialize to -1 cause it is used as an error flag later
 	int status = EXIT_SUCCESS;
	int timeunit;
	ssize_t nbytes;
	char twit[ TWIT_MAXLEN + 1 ];
	size_t twitlen = sizeof( twit ) / sizeof( twit[ 0 ] );
	struct linger lingerbuf;

	// Verify that user gave the appropriate arguments
	if ( argc != 5 ){
		usage( argv[ 0 ] );
	}

	// Retrieve the command line arguments
	addr = argv[ 1 ];
	port = argv[ 2 ];
	filename = argv[ 3 ];
	timeunit = atoi( argv[ 4 ] );
	assert( timeunit >= 0 );

	do{
		// Connect to the twitserver
		if ( ( sockfd = connect_to_twitserver( addr, port ) ) == -1 ){ 
			status = EXIT_FAILURE;
			break;
		}
		
		// Open the file to get twits from only if not stdin
		if ( !strcmp( filename, "stdin" ) ){
			fp = stdin;
		}
		else{
			errno = 0;
			if ( ( fp = fopen( filename, "r" ) ) == NULL ){
				error( "could not open file %s: (%s)\n", filename, strerror( errno ) );
				status = EXIT_FAILURE;
				break;
			}
		}

		// For each twit in the file
		while ( ( nbytes = gettwitfromfile( fp, twit, twitlen ) ) != ( ssize_t )-1 ){
			printf( "Will send %s\n", twit );
			fflush( stdout );
			sleep( timeunit );
			// Send the twit to the server
			if ( send_to_twitserver( sockfd, twit, ( size_t )nbytes ) == -1 ){
				error( "failed to send one twit. The program will not attempt to send any more twits\n" );
				status = EXIT_FAILURE;
				break;
			}
		}
		// EndFor
	}while ( 0 );

	// Cleanup code

	// Disconnect from the twitserver
	// must set SO_LINGER so as to block close() and send all the data to the server
	lingerbuf.l_onoff = 1;
	lingerbuf.l_linger = 20;
	while ( setsockopt( sockfd, SOL_SOCKET, SO_LINGER, &lingerbuf, sizeof( lingerbuf ) ) == -1 ){ continue; }
	if ( sockfd != -1 && disconnect_from_twitserver( TwitClientType_SAYER, sockfd ) == -1 ){ status = EXIT_FAILURE; }
	if ( fp != NULL && strcmp( filename, "stdin" ) ){ 
		errno = 0;
		if ( fclose( fp ) == EOF ){
			error( "failed to close file %s: (%s)\n", filename, strerror( errno ) );
			status = EXIT_FAILURE;
		}
	}

	// Return
	exit( status );
}



// Display usage info and exit
static void usage( const char * restrict programname ){
	assert( programname != NULL );
	error( "usage: %s addr port filename timeunit\n", programname );
	exit( EXIT_FAILURE );
}
