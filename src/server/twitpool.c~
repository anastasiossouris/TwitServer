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
 * \file twitpool.c 
 *
 * File twitpool.c contains the implementation of the twitpool.h interface.
 *
 * The twitpool is implemented as a doubly-linked queue.
 *
 * @author Tassos Souris
 */
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "twit.h"
#include "twitpool.h"



/**
 * The linknode() function shall link the twitpool_node in the twitpool. Parameters shall not be NULL pointers.
 *
 * @return Nothing.
 */
static inline void linknode( struct twitpool * restrict tp, struct twitpool_node * restrict node );



// Initialize the struct twitpool object
int inittwitpool( struct twitpool * restrict tp ){
	// Validate the parameter
	if ( tp == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// At first no twit has been inserted so head and tail must be NULL
	tp->tp_head = NULL;
	tp->tp_tail = NULL;
	tp->tp_count = 0;

	return ( 0 );
}

// Put the twit in the pool
int putintwitpool( struct twitpool * restrict tp, 
			const char * restrict string, 
			size_t string_len ){
	struct twitpool_node *newNode = NULL;

	// Validate the parameters
	if ( tp == NULL || string == NULL || string_len == 0 ){
		errno = EINVAL;
		return ( -1 );
	}

	// Allocate the node
	if ( ( newNode = malloc( sizeof( struct twitpool_node ) ) ) == NULL ){
		return ( -1 );
	}

	// Enter the string inside the twit
	errno = 0;
	if ( filltwit( &newNode->tpn_twit, string, string_len ) == -1 ){
		assert( errno == ENOMEM );
		// Must free the node allocated or else i'll have a memory leak
		free( newNode );
		return ( -1 );
	}

	// Link the new node inside the pool
	linknode( tp, newNode );

	// Increase the count
	++tp->tp_count;

	return ( 0 );
}

// Retrieve twit from the head
int getfromtwitpool( struct twitpool * restrict tp, 
			struct twit * restrict t ){
	struct twitpool_node *node = NULL;
	
	// Validate the parameters
	if ( tp == NULL || t == NULL ){
		errno = EINVAL;
		return ( -1 );	
	}
	else if ( twitpoolisempty( tp ) ){
		errno = EPERM;
		return ( -1 );
	}

	node = tp->tp_head;
	// If it is the last twit
	if ( node == tp->tp_tail ){
		tp->tp_tail = NULL;
		tp->tp_head = NULL;
	}
	else{
		tp->tp_head = node->tpn_previous;
		tp->tp_head->tpn_next = NULL;
	}

	// need not copy the twit to the struct twit. I just pass the pointer. 
	t->t_twit = node->tpn_twit.t_twit;
	t->t_twitlen = node->tpn_twit.t_twitlen;
	
	// free the pool node
	free( node );

	// Decrease the count
	--tp->tp_count;

	return ( 0 );
}

// Test whether the pool is empty or not
int twitpoolisempty( const struct twitpool * restrict tp ){
	// Validate the parameter
	if ( tp == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	return ( tp->tp_count == 0 );
}

// Return number of twits
ssize_t twitpoolcount( const struct twitpool * restrict tp ){
	// Validate the parameter
	if ( tp == NULL ){
		return ( -1 );
	}
	
	return ( tp->tp_count );
}

// Delete all nodes from the pool
void deltwitpool( struct twitpool * restrict tp ){
	if ( tp != NULL ){
		struct twitpool_node *current = NULL;
		struct twitpool_node *next = NULL;
		for ( current = tp->tp_tail; current != NULL; current = next ){
			next = current->tpn_next;
			deltwit( &current->tpn_twit );
			free( current );
		}
	}
}

// Implementation of local functions...



// Used to link a new twitpool_node inside a twitpool
static inline void linknode( struct twitpool * restrict tp, 
				struct twitpool_node * restrict node ){
	assert( tp != NULL );
	assert( node != NULL );

	// If it is the first node to be entered in the pool both head and tail must point to it
	if ( twitpoolisempty( tp ) ){
		tp->tp_head = node;
	}

	// The previous pointer of the new node must be made to point to where the current head points 
	node->tpn_next = tp->tp_tail;

	// Since this new node is placed in the tail there is no previous node
	node->tpn_previous = NULL;

	// the current tail node's previous must be made to point to the new node
	if ( tp->tp_tail != NULL ){
		tp->tp_tail->tpn_previous = node;
	}

	// Now the tail must point to the new node
	tp->tp_tail = node;	
}
