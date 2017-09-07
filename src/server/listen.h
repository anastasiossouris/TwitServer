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
 * \file listen.h
 *
 * The listen.h header file contains the declaration of the prepareListenerSocket() function.
 *
 * @author Tassos Souris
 */
#if !defined( LISTEN_H_IS_INCLUDED )
#define LISTEN_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

/**
 * The prepareListenerSocket() function shall create a socket to listen for connections in the local machine at the specified port.
 *
 * @return Upon successful completion the socket created shall be returned; otherwise, -1 shall be returned and errno shall be set
 * 	to indicate the error.
 */
int prepareListenerSocket( int port );

/**
 * The prepareSayersListenerSocket() function shall create a socket to listen for sayers.
 *
 * @return Upon successful completion the socket created shall be returned; otherwise, -1 shall be returned and errno shall be set
 * 	to indicate the error.
 */
int prepareSayersListenerSocket( void );

/**
 * The prepareHearersListenerSocket() function shall create a socket to listen for hearers.
 *
 * @return Upon successful completion the socket created shall be returned; otherwise, -1 shall be returned and errno shall be set
 * 	to indicate the error.
 */
int prepareHearersListenerSocket( void );

/**
 * The sayersListener() function shall be responsible for accepting connections from sayers. The sayersListener() function
 * shall run in its own thread and shall be passed a pointer to a serverinfo structure as parameter.
 *
 * @retun The sayersListener() function shall always return NULL.
 */
void *sayersListener( void *arg );

/**
 * The hearersListener() function shall be responsible for accepting connections from hearers. The hearersListener() function
 * shall run in its own thread and shall be passed a pointer to a serverinfo structure as parameter.
 *
 * @retun The hearersListener() function shall always return NULL.
 */
void *hearersListener( void *arg );

#if defined( __cplusplus )
}
#endif

#endif
