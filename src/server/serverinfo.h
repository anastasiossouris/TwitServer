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
 * \file serverinfo.h
 *
 * File serverinfo.h contains the definition of the serverinfo and connserverinfo structures.
 *
 * @author Tassos Souris
 */
#if !defined( SERVERINFO_H_IS_INCLUDED )
#define SERVERINFO_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#include <pthread.h>
#include "statistics.h"
#include "twitpool.h"
#include "twitpoollist.h"



/**
 * \struct serverinfo
 *
 * struct serverinfo is the structure shared by the threads.
 *
 * It contains members for:
 *	1) Managing the statistics
 *		+ A statistics structure
 *		+ A lock for handling access to the statistics structure
 *		+ A condition for signaling whether a sayer was disconnected
 *		+ A condition for signaling whether a hearer was disconnected
 *	2) Managing preparation status 
 *		+ A flag that gets one of three values; first it is -1 meaning that the
 *		preparation status has not yet been determined. It later gets 1 or 0
 *		meaning that the thread prepared successfully or not, respectively.
 *		+ A lock for handling access to that flag.
 *		+ A condition variable for signaling whether the preparation status
 *		is determined or not.
 *	3) Managing the message data structure
 *	4) Keeping track of the threads
 *
 * The members in the serverinfo structure are ordered logically as parts that can be grouped together.
 * Reordering the members i could save around 8 bytes (as shown in my machine) but since only one object
 * of the serverinfo structure will be used and for only one object memory is not an issue i opted to
 * group the members logically and by frequency of usage.
 */
struct serverinfo{
	// Managing the statistics
	struct statistics si_stats;
	pthread_mutex_t si_stats_lock;
	pthread_cond_t si_stats_sayers_cond;
	pthread_cond_t si_stats_hearers_cond;
	// Managing preparation status
	int si_prepared;
	pthread_mutex_t si_prepared_lock;
	pthread_cond_t si_prepared_cond;
	// Structure holding the twits
	struct twitpool si_twitpool;
	pthread_mutex_t si_twitpool_lock;
	pthread_cond_t si_twitpool_cond;
	// One twitpool for each hearer
	struct twitpoollist si_twitpool_list;
	pthread_mutex_t si_twitpool_list_lock;
	// This is the thread listening for sayers
	pthread_t si_sayers_listener_threadid;
	// This is the thread listening for hearers
	pthread_t si_hearers_listener_threadid;
	// This is the thread updating the statistics
	pthread_t si_statistics_updater_threadid;	
	// This is the thread consuming the twitpool
	pthread_t si_twitpool_consumer_threadid;
};

/**
 * \struct connserverinfo
 *
 * The connserverinfo structure is used for the threads handling the connection with sayers and hearers.
 * It uses the same serverinfo structure shared by all threads (thus the pointer to struct serverinfo)
 * and adds the socket file descriptor to which the thread will read from or write to as well as the 
 * struct twitpool from which to retrieve twits (inside twitpoollist_node).
 */
struct connserverinfo{
	struct serverinfo *csi_serverinfo;
	struct twitpoollist_node *csi_tpln;
	int csi_sockfd;
};



/**
 * The acquire_statistics() function shall acquire ownership of the statistics structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void acquire_statistics( struct serverinfo * restrict si );

/**
 * The release_statistics() function shall release ownership of the statistics structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void release_statistics( struct serverinfo * restrict si );

/**
 * The acquire_twitpool() function shall acquire ownership of the twitpool structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void acquire_twitpool( struct serverinfo * restrict si );

/**
 * The release_twitpool() function shall release ownership of the twitpool structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void release_twitpool( struct serverinfo * restrict si );

/**
 * The acquire_twitpool_list() function shall acquire ownership of the twitpoollist structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void acquire_twitpool_list( struct serverinfo * restrict si );

/**
 * The release_twitpool_list() function shall release ownership of the twitpoollist structure in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void release_twitpool_list( struct serverinfo * restrict si );

/**
 * The acquire_preparation_status() function shall acquire ownership of the preparation status member in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void acquire_preparation_status( struct serverinfo * restrict si );

/**
 * The acquire_preparation_status() function shall release ownership of the preparation status member in the serverinfo structure
 * pointed to by parameter si, which shall not be a NULL pointer.
 *
 * @return Nothing.
 */
void release_preparation_status( struct serverinfo * restrict si );

/**
 * The signal_prepared_status() function shall signal the prepared status of the struct serverinfo object pointed to by parameter
 * si. The status given as parameter shall be either 0 or 1.
 *
 * @return Nothing.
 */
void signal_prepared_status( struct serverinfo * restrict si, int status );

#if defined( __cplusplus )
}
#endif

#endif
