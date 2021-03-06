/*

The MIT License (MIT)

Copyright (c) 2015 Fabien Lahoudere

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdlib.h>

#include <buffer_pool.h>


int alloc_pool( buf_pool_t * pool, int pool_size, int buffer_size )
{
	int idx;

	if ( !pool )
		return EXIT_FAILURE;

	pool->sz = pool_size;

	pool->buf = malloc( pool_size * sizeof(buf_pool_entry_t) );
	if ( !pool->buf )
		goto error;
	for ( idx = 0; idx < pool_size ; idx++ )
	{
		pool->buf[idx].free = 1;
		pool->buf[idx].buf = malloc(buffer_size);
		if ( !pool->buf[idx].buf )
			goto error;
	}

	return EXIT_SUCCESS;
error:
	free_pool( pool );
	return EXIT_FAILURE;
}

void * get_buffer( buf_pool_t * pool )
{
	if ( pool && pool->buf )
	{
		int idx;
		for ( idx = 0; idx < pool->sz ; idx++ )
		{
			if ( pool->buf[idx].free != 0 )
			{
				pool->buf[idx].free = 0;
				return pool->buf[idx].buf;
			}
		}
	}
	return NULL;
}

int free_buffer( buf_pool_t * pool, void * buffer )
{
	if ( pool && pool->buf )
	{
		int idx;
		for ( idx = 0; idx < pool->sz ; idx++ )
		{
			if ( pool->buf[idx].buf == buffer )
			{
				pool->buf[idx].free = 1;
				return EXIT_SUCCESS;
			}
		}
	}
	return EXIT_FAILURE;
}

void free_pool( buf_pool_t * pool )
{
	if ( pool && pool->buf )
	{
		int idx;

		for ( idx = 0; idx < pool->sz ; idx++ )
		{
			if ( pool->buf[idx].buf )
				free( pool->buf[idx].buf );
		}
		free(pool->buf);
	}
}
