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
 * \file statistics.h
 *
 * File statistics.h defines the statistics structure.
 *
 * @author Tassos Souris
 */
#if !defined( STATISTICS_H_IS_INCLUDED )
#define STATISTICS_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C"{
#endif

#include <assert.h>
#include <limits.h>

#define THREADS_MAX INT_MAX
#define HEARERS_MAX INT_MAX
#define SAYERS_MAX INT_MAX
#define ARRIVED_TWITS_MAX INT_MAX
#define DELIVERED_TWITS_MAX INT_MAX

#define increaseTwitsStored( stats )
#define increaseThreadsNum( st ) do{ \
	assert( (st) != NULL );	\
	if ( THREADS_MAX - (st)->stats_threadsNum >= 1 ){	\
		++(st)->stats_threadsNum;	\
	}	\
}while ( 0 )
#define decreaseThreadsNum( st ) do{ \
	assert( (st) != NULL ); \
	assert( (st)->stats_threadsNum > 0 ); \
	--(st)->stats_threadsNum; \
}while ( 0 )
#define increaseHearersNum( st ) do{ \
	assert( (st) != NULL );	\
	if ( HEARERS_MAX - (st)->stats_hearersNum >= 1 ){	\
		++(st)->stats_hearersNum;	\
	}	\
}while ( 0 )
#define decreaseHearersNum( st ) do{ \
	assert( (st) != NULL ); \
	assert( (st)->stats_hearersNum > 0 ); \
	--(st)->stats_hearersNum; \
}while ( 0 )
#define increaseSayersNum( st ) do{ \
	assert( (st) != NULL );	\
	if ( SAYERS_MAX - (st)->stats_sayersNum >= 1 ){	\
		++(st)->stats_sayersNum;	\
	}	\
}while ( 0 )
#define decreaseSayersNum( st ) do{ \
	assert( (st) != NULL ); \
	assert( (st)->stats_sayersNum > 0 ); \
	--(st)->stats_sayersNum; \
}while ( 0 )
#define increaseArrivedTwitsNum( st ) do{ \
	assert( (st) != NULL );	\
	if ( ARRIVED_TWITS_MAX - (st)->stats_arrivedTwitsNum >= 1 ){	\
		++(st)->stats_arrivedTwitsNum;	\
	}	\
}while ( 0 )
#define increaseDeliveredTwitsNum( st ) do{ \
	assert( (st) != NULL );	\
	if ( DELIVERED_TWITS_MAX - (st)->stats_deliveredTwitsNum >= 1 ){	\
		++(st)->stats_deliveredTwitsNum;	\
	}	\
}while ( 0 )

/**
 * \struct statistics
 *
 * The statistics structure keeps track of various statistics related to the lifetime of the server.
 */
struct statistics{
	int stats_storedTwitsNum; /**< Number of twits currently stored */
	int stats_threadsNum; /**< Number of threads */
	int stats_hearersNum; /**< Number of hearers connected to the server */
	int stats_sayersNum;  /**< Number of sayers connected to the server */
	int stats_arrivedTwitsNum; /**< Total number of twits arrived in the server */ 
	int stats_deliveredTwitsNum; /**< Total number of twits delivered to the hearers */
	float stats_averageTwitsIncomingRate; /**< Average incoming rate of twits per sec */
	float stats_averageTwitsOutcomingRate; /**< Average outcoming rate of twits per sec */
};

/**
 * The statisticsUpdater() function shall update every STATS_UPDATE_NSEC the statistics structure of the serverinfo structure
 * passed as parameter. The statisticsUpdater() function shall run in its own thread of execution.
 *
 * @return The statisticsUpdater() function shall always return NULL.
 */
void *statisticsUpdater( void *arg );

#if defined( __cplusplus )
}
#endif

#endif
