#ifndef _FALCON_
#define _FALCON_

#ifdef FALCON
void falcon_print_on(void);
void falcon_print_off(void);
void falcon_print_memuse(void);
void falcon_print_highwater(void);
long falcon_memuse(void);
long falcon_highwater(void);
void falcon_cleanup(void);

void falcon_setmem(void *p, int64_t num);
void falcon_unsetmem(void *p);

void *falcon_realloc(void *p, size_t num);
void *falcon_calloc(size_t num, size_t size);
void *falcon_malloc(size_t num);
void falcon_free(void *p);

#define realloc falcon_realloc
#define calloc falcon_calloc
#define malloc falcon_malloc
#define free falcon_free
#endif

#endif /* _FALCON_ */

