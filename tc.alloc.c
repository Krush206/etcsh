#include "sh.h"

static struct Memory *memsrch(void *);

void *
xcalloc(size_t n, size_t size)
{
    size_t total;
    void *new;

    if (n > 0 && size > SIZE_MAX / n)
	stderror(ERR_NOMEM);
    total = n * size;
    if (total == 0)
	return NULL;
    if (total >= BUF_MAX)
	stderror(ERR_NOMEM);
    new = xmalloc(total);
    if (new == NULL)
	stderror(ERR_NOMEM);
    return memset(new, 0, total);
}

void *
xrealloc(void *ptr, size_t size)
{
    void *new;
    struct Memory *pool;

    if (size == 0) {
	xfree(ptr);
	return NULL;
    }
    if (ptr == NULL)
	return xmalloc(size);
    new = xmalloc(size);
    if (new == NULL)
	stderror(ERR_SILENT);
    pool = memsrch(ptr);
    if (pool == NULL) {
	xfree(new);
	stderror(ERR_SILENT);
    }
    (void) memcpy(new, ptr, pool->size);
    xfree(ptr);
    return new;
}

void *
xmalloc(size_t size)
{
    struct Memory *pool;

    if (size == 0)
	return NULL;
    if (size >= BUF_MAX)
	stderror(ERR_NOMEM);
    for (pool = (*mem)->next; pool != *mem; pool = pool->next)
	if (!pool->use) {
	    pool->use = 1;
	    pool->size = size;
	    return pool->alloc = &pool->buf[BUF_MAX - size];
	}
    stderror(ERR_NOMEM);
    return NULL;
}

static struct Memory *
memsrch(void *ptr)
{
    struct Memory *pool;

    for (pool = (*mem)->next; pool != *mem; pool = pool->next)
	if (pool->alloc == ptr)
	    return pool;
    return NULL;
}

void
xfree(void *ptr)
{
    struct Memory *pool;

    if (ptr == NULL)
	return;
    pool = memsrch(ptr);
    if (pool == NULL)
	stderror(ERR_SILENT);
    pool->use = 0;
}
