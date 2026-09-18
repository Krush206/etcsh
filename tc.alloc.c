#include "sh.h"

int dont_free = 0;

struct Memory (*mem)[MEM_MAX];
struct Memory *memfree;

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
    if (pool->size < size)
	(void) memcpy(new, ptr, pool->size);
    else
	(void) memcpy(new, ptr, size);
    xfree(ptr);
    return new;
}

static struct Memory *
memsrch(void *ptr)
{
    int high;
    int low;
    uintptr_t memstart;
    uintptr_t memend;
    uintptr_t target;

    memstart = (uintptr_t) *mem;
    memend = (uintptr_t) &(*mem)[MEM_MAX];
    target = (uintptr_t) ptr;
    if (target < memstart || target >= memend)
	return NULL;
    low = 0;
    high = MEM_MAX - 1;
    while (low <= high) {
	int mid;
	struct Memory *pool;
	uintptr_t bufstart;
	uintptr_t bufend;

	mid = low + (high - low) / 2;
	pool = &(*mem)[mid];
	bufstart = (uintptr_t) pool->buf;
	bufend = bufstart + BUF_MAX;
	if (target >= bufstart && target < bufend)
	    return pool;
	if (target < bufstart)
	    high = mid - 1;
	else
	    low = mid + 1;
    }
    return NULL;
}

void *
xmalloc(size_t size)
{
    struct Memory *pool;

    if (size == 0)
	return NULL;
    if (size >= BUF_MAX)
	stderror(ERR_NOMEM);
    if (memfree == NULL)
	stderror(ERR_NOMEM);
    pool = memfree;
    memfree = pool->next;
    pool->use = 1;
    pool->size = size;
    return &pool->buf[BUF_MAX - size];
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
    if (!pool->use)
	return;
    pool->use = 0;
    pool->next = memfree;
    memfree = pool;
}

void
showall(Char **v, struct command *c)
{
    struct Memory *pool;
    unsigned int i;

    USE(v);
    USE(c);
    i = 0;
    for (pool = *mem; pool != &(*mem)[MEM_MAX]; pool++)
	if (pool->use)
	    i++;
    xprintf("%u pools in use.\n", i);
}
