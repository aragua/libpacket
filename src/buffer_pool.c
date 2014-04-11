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
