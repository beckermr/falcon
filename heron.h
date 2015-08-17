#ifndef _HERON_
#define _HERON_

#ifdef HERON
void heron_print_on(void);
void heron_print_off(void);
void heron_print_memuse(void);
void heron_print_highwater(void);
long heron_memuse(void);
long heron_highwater(void);
void heron_cleanup(void);

void heron_setmem(void *p, int64_t num);
void heron_unsetmem(void *p);

void *heron_realloc(void *p, size_t num);
void *heron_calloc(size_t num, size_t size);
void *heron_malloc(size_t num);
void heron_free(void *p);

#define realloc heron_realloc
#define calloc heron_calloc
#define malloc heron_malloc
#define free heron_free
#endif

#endif /* _HERON_ */

