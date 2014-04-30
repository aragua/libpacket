#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

typedef struct buf_pool_entry_s buf_pool_entry_t;
struct buf_pool_entry_s
{
	int free;
	size_t sz;
	void * buf;
};

typedef struct buf_pool_s buf_pool_t;
struct buf_pool_s
{
	size_t sz;
	buf_pool_entry_t * buf;
};

int alloc_pool( buf_pool_t * pool, int pool_size, int buffer_size );

void * get_buffer( buf_pool_t * pool);
int free_buffer( buf_pool_t * pool, void * buffer );

void free_pool( buf_pool_t * pool );

#endif /* BUFFER_POOL_H */
