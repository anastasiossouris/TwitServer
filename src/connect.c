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
 * \file connect.c
 *
 * File connect.c contains the implementation of the connect.h interface.
 *
 * @author Tassos Souris
 */
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "connect.h"
#include "config.h"
#include "error.h"
#include "util.h"

// Establish a connection with the twitserver. Return the twitserver file descriptor if ok and -1 otherwise
int connect_to_twitserver( const char * restrict addr, const char * restrict port ){
	struct addrinfo *infop = NULL;
	struct addrinfo hint;
	int status = 0;
	int socketfd = -1;

	assert( addr != NULL );
	assert( port != NULL );
	
	do{
		// Get the addr of the twitserver
		( void )memset( &hint, 0, sizeof( hint ) );
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = 0;
		if ( ( status = getaddrinfo( addr, port, &hint, &infop ) ) != 0 ){
			error( "Could not find the address of twitserver with %s and %s: (%s)\n",
				addr,
				port,
				gai_strerror( status )
				);
			status = -1;
			break;
		}

		// Create a socket
		errno = 0;
		if ( ( socketfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ){
			error( "Failed to create a socket: (%s)\n",
				strerror( errno )
				);
			status = -1;
			break;
		}

		// Connect that socket with the twitserver
		while ( connect( socketfd, infop->ai_addr, infop->ai_addrlen ) == -1 ){
			if ( errno == EINTR ){
				continue;	
			}
			error( "failed to connect to the twitserver: (%s)\n", strerror( errno ) );
			status = -1;
			break;
		}

		// If ok till now the function will return the socket fd
		if ( status == 0 ){
			status = socketfd;
		}
	}while ( 0 );

	// Clean-up code
	if( status == -1 ){
		if ( infop != NULL ){
			freeaddrinfo( infop );
		}
		if ( socketfd != -1 ){
			errno = 0;
			if ( safe_close( socketfd ) ){
				error( "failed to close socket: (%s)\n", strerror( errno ) );
			}
		}
	}
	
	return ( status );
}

// Start receiving twits from the twitserver. Return 0 if ok and -1 otherwise.
int recv_from_twitserver( int sockfd, int timeunit ){
	int status = 0;
	ssize_t nbytes;	
	char ch;

	assert( timeunit >= 0 );

	while( 1 ){
		errno = 0;
		sleep( timeunit );
		nbytes = read( sockfd, &ch, sizeof( ch ) );
		if ( nbytes == -1 ){
			if ( errno == EINTR ){
				continue;
			}
			error( "could not receive byte from twit server: (%s)\n", strerror( errno ) );
			status = -1;
			break;
		}
		errno = 0;
		if ( fprintf( stdout, "%c", ch ) == -1 ){
			error( "could not send byte to stdout: (%s)\n", strerror( errno ) );
			status = -1;
			break;
		}
		( void )fflush( stdout );
	}

	return ( status );
}

// Send a twit to the twitserver. Return 0 if ok and -1 otherwise.
int send_to_twitserver( int sockfd, const char * restrict buf, size_t nbytes ){	
	size_t bytesToSend = nbytes <= TWIT_MAXLEN ? nbytes : TWIT_MAXLEN;
	int status = 0;

	assert( buf != NULL );	
	
	errno = 0;
	if ( writeall( sockfd, buf, bytesToSend ) != ( ssize_t )bytesToSend ){
		error( "failed to send bytes to the twitserver: (%s)\n", strerror( errno ) );
		status = -1;
	}

	return ( status );
}

// Close connection with the twitserver. Return 0 if ok and -1 otherwise.
int disconnect_from_twitserver( enum TwitClientType twitClientType, int sockfd ){
	int status = 0;
	
	assert( isTwitClientTypeValid( twitClientType ) );

	errno = 0;
	if ( safe_close( sockfd ) == -1 ){
		error( "failed to close socket: (%s)\n", strerror( errno ) );
		status = -1;
	}

	return ( status );
}
