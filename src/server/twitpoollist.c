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
 * \file twitpoollist.c
 *
 * File twitpoollist.c contains the implementation of the twitpoollist.h interface.
 *
 * @author Tassos Souris. 
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "twitpool.h"
#include "twitpoollist.h"



// Prepare the struct twitpoollist
int inittwitpoollist( struct twitpoollist * restrict tpl ){
	// Validate the parameter
	if ( tpl == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// At first the list is empty so head points to nothing
	tpl->tpl_head = NULL;

	return ( 0 );
}

// Create a node and insert it at the begining of the list. Store the address of the new node at *tplnode. 
int newtwitpool( struct twitpoollist * restrict tpl, 
			struct twitpoollist_node ** restrict tplnode ){
	struct twitpoollist_node *newNode = NULL; // Must be initialized to NULL cause it is used as an error flag later
	int status = 0; // Must initialize to 0 cause status will only be changed if something wrong happens

	// Validate the parameters
	if ( tpl == NULL || tplnode == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	do{
		// Create the new node
		if ( ( newNode = malloc( sizeof( struct twitpoollist_node ) ) ) == NULL ){ status = -1; break; }

		// Initialize the new node as needed
		( void )inittwitpool( &newNode->tpln_twitpool );

		// Initialize the mutexes and conditions
		while ( pthread_mutex_init( &newNode->tpln_lock, NULL ) ){ continue; }
		while ( pthread_cond_init( &newNode->tpln_cond, NULL ) ){ continue; }

		// Link the new node at the beginning of the list
		newNode->tpln_next = tpl->tpl_head;
		if ( tpl->tpl_head != NULL ){
			tpl->tpl_head->tpln_previous = newNode;
		}
		tpl->tpl_head = newNode;

		// Inform client of the new node
		*tplnode = newNode;
	}while ( 0 );

	// Return the status
	return ( status );
}

// Remove the node pointed to by tplnode from the list
int removefromtwitpoollist( struct twitpoollist * restrict tpl, 
				struct twitpoollist_node * restrict tplnode ){	
	// Validate the parameters
	if ( tpl == NULL || tplnode == NULL ){
		errno = EINVAL;
		return ( -1 );
	}

	// Unlink the node from the list
	// If it is the first node in the list then the head must be made to point to the next node from
 	// the one to be deleted

	if ( tplnode->tpln_previous == NULL ){
		tpl->tpl_head = tplnode->tpln_next;
	}
	// Otherwise that must happen for the previous node of the one to be deleted.
	else{
		tplnode->tpln_previous->tpln_next = tplnode->tpln_next;
	}

	// Cleanup that node
	deltwitpool( &tplnode->tpln_twitpool );
	while ( pthread_mutex_destroy( &tplnode->tpln_lock ) ){ continue; }
	while ( pthread_cond_destroy( &tplnode->tpln_cond ) ){ continue; }
	free( tplnode );

	return ( 0 );
}

// Cleanup all twitpoollist
void deltwitpoollist( struct twitpoollist * restrict tpl ){
	if ( tpl != NULL ){
		struct twitpoollist_node *current = NULL;
		struct twitpoollist_node *next = NULL;
		for ( current = tpl->tpl_head; current != NULL; current = next ){
			next = current->tpln_next;
			deltwitpool( &current->tpln_twitpool );
			while ( pthread_mutex_destroy( &current->tpln_lock ) ){ continue; }
			while ( pthread_cond_destroy( &current->tpln_cond ) ){ continue; }
			free( current );
		}
	}

	return ;
}


void acquire_twitpool_in_twitpoollist_node( struct twitpoollist_node * restrict tpln ){
	assert( tpln != NULL );

	while ( pthread_mutex_lock( &tpln->tpln_lock ) ){ continue; }

	return ;
}

void release_twitpool_in_twitpoollist_node( struct twitpoollist_node * restrict tpln ){
	assert( tpln != NULL );

	while ( pthread_mutex_unlock( &tpln->tpln_lock ) ){ continue; }

	return ;
}
