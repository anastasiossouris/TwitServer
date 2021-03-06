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
 * \file util.h
 *
 * File util.h defines some utility functions that i used mostly as safe replacements of system calls.
 *
 * @author Tassos Souris.
 */
#if !defined( UTIL_H_IS_INCLUDED )
#define UTIL_H_IS_INCLUDED 1

#if defined( __cplusplus )
extern "C" {
#endif

#include <sys/types.h>

/**
 * The safe_close() function is equivalent to the close() function except that in case of an EINTR error it retries.
 *
 * @return Refer to the close() function
 * @param fd Refer to the close() function
 */
int safe_close( int fd );

/**
 * The writeall() function is equivalent to the write() function except that it retries in case of a EINTR error and
 * attempts to write all bytes.
 *
 * @return Refer to the write() function.
 * @param fd Refer to the write() function.
 * @param buf Refer to the write() function.
 * @param nbytes Refer to the write() function.
 */
ssize_t writeall( int fd, const void *buf, size_t nbytes );

/**
 * The readall() function is equivalent to the read() function except that it retries in case of a EINTR error and
 * attempts to write all bytes.
 *
 * @return Refer to the read() function.
 * @param fd Refer to the read() function.
 * @param buf Refer to the read() function.
 * @param nbytes Refer to the read() function.
 */
ssize_t readall( int fd, void *buf, size_t nbytes );

#if defined( __cplusplus )
}
#endif

#endif
