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
 * \file connect.h
 *
 * File connect.h declares all the necessary functions for the clients to connect to a twitserver, at a specified address and port,
 * and according to their type, meaning whether they are hearers or sayers, to receive or send bytes respectively.
 *
 * @author Tassos Souris
 */
#if !defined( CONNECT_H_IS_INCLUDED )
#define CONNECT_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#define isTwitClientTypeValid( twitClientType ) ( (twitClientType) > TwitClientType_InvalidFirst && (twitClientType) < TwitClientType_InvalidLast )

/**
 * \enum TwitClientType
 *
 * The TwitClientType enumeration distinguishes between the two different types of clients that can connect to 
 * a twitserver.
 */
enum TwitClientType{
	TwitClientType_InvalidFirst = -1,
	TwitClientType_HEARER, /**< This type of client connects to a twitserver and receives twits */
	TwitClientType_SAYER, /**< This type of client connects to a twitserver and sends twits */
	TwitClientType_InvalidLast
};

/**
 * The connect_to_twitserver() function shall attempt to establish a connection with the twitserver at the specified address and port.
 * The connect_to_twitserver() function shall write to stderr any message in case of failure. Parameters addr and port shall not be 
 * NULL pointers.
 *
 * @return Upon successful completion the socket file descriptor shall be returned; otherwise, -1 shall be returned.
 * @param addr Pointer to the address.
 * @param port Pointer to the port.
 */
int connect_to_twitserver( const char * restrict addr, const char * restrict port );

/**
 * The recv_from_twitserver() function shall receive bytes from the twitserver associated with the socket file descriptor given as parameter
 * and print them to stdout. The recv_from_twitserver() function shall write to stderr any message in case of failure. The recv_from_twitserver()
 * function shall receive each byte from the server every timeunit time.
 *
 * @return The recv_from_twitserver() function shall return zero if successful; otherwise, -1 shall be returned.
 * @param sockfd The twitserver file descriptor.
 */
int recv_from_twitserver( int sockfd, int timeunit );

/**
 * The send_to_twitserver() function shall send nbytes bytes starting from the byte pointed to by parameter buf, which shall not be a NULL pointer,
 * to the twitserver associated with the socket file descriptor given as parameter. The send_to_twitserver() function shall write to stderr any message
 * in case of failure. No more than TWIT_MAXLEN, as defined in the config.h header file, bytes shall be send by the send_to_twitserver() function.
 *
 * @return Upon successful completion zero shall be returned; otherwise, -1 shall be returned.
 * @param sockfd The twitserver file descriptor.
 * @param buf Pointer to the message to be send
 * @param nbytes How many bytes to be send.
 */
int send_to_twitserver( int sockfd, const char * restrict buf, size_t nbytes );

/**
 * The disconnect_from_twitserver() function shall close the connection with twitserver associated with the socket file descriptor given as
 * parameter. The disconnect_from_twitserver() function shall write to stderr any message in case of failure. Parameter twitClientType, which
 * shall be valid, specified the type of client that shall disconnect from the twitserver.
 *
 * @return The disconnect_from_twitserver() function shall return zero if successful; otherwise, -1 shall be returned.
 * @param sockfd The twitserver file descriptor.
 */
int disconnect_from_twitserver( enum TwitClientType twitClientType, int sockfd );

#if defined( __cplusplus )
}
#endif

#endif
