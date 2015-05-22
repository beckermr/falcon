#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>
//#include "check_syscalls.h"
#include <inttypes.h>

///////////////////////////////////////////////////////////////////////////////////////////////
/*
  hashing code from Peter Behroozi
  lifted under GPL 
  some of the system calls have been switched back
  to standard versions
 */

#define IH_INVALID INT64_MAX
#define IH_DELETED (INT64_MAX-1)
#define IH_SKIP (INT64_MAX-2)

struct intbucket {
  int64_t key;
  void *data;
};

struct inthash {
  uint64_t hashwidth, elems, num_buckets, hashnum;
  struct intbucket *buckets;
};

static struct inthash *new_inthash(void);
static int64_t *ih_keylist(struct inthash *ih);
static void *ih_getval(struct inthash *ih, int64_t key);
static void ih_setval(struct inthash *ih, int64_t key, void *data);
static void ih_delval(struct inthash *ih, int64_t key);
static void free_inthash(struct inthash *ih);
static void ih_prealloc(struct inthash *ih, int64_t size);

static void free_inthash2(struct inthash *ih);
static void ih_setval2(struct inthash *ih, int64_t key1, int64_t key2, void *data);
static void *ih_getval2(struct inthash *ih, int64_t key1, int64_t key2);
static void ih_setint64(struct inthash *ih, int64_t key, int64_t value);
static int64_t ih_getint64(struct inthash *ih, int64_t key);
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// actual memory prof.
static struct inthash *MemHash = NULL;
static int64_t MaxMem = 0;
static int64_t CurrMem = 0;
static int PrintHighWater = 1;

void falcon_cleanup(void) {
  MaxMem = 0;
  CurrMem = 0;
  free_inthash(MemHash);
}

void falcon_print_on(void) {
  PrintHighWater = 1;
}

void falcon_print_off(void) {
  PrintHighWater = 0;
}

void falcon_setmem(void *p, int64_t num) {
  if(p == NULL) return;

  if(MemHash == NULL) MemHash = new_inthash();

  ih_setint64(MemHash,(int64_t) p,num);
  CurrMem += num;
  if(CurrMem > MaxMem) {
    if(PrintHighWater) {
      fprintf(stderr,"mem high water: %lld bytes\n",CurrMem);
      fflush(stderr);
    }
    
    MaxMem = CurrMem;
  }
}

void falcon_print_memuse(void) {
  fprintf(stderr,"mem usage: %lld bytes\n",CurrMem);
  fflush(stderr);
}

void falcon_print_highwater(void) {
  fprintf(stderr,"mem usage: %lld bytes\n",MaxMem);
  fflush(stderr);
}

long falcon_memuse(void) {
  return CurrMem;
}

long falcon_highwater(void) {
  return MaxMem;
}

void falcon_unsetmem(void *p) {
  if(p == NULL) return;

  if(MemHash == NULL) MemHash = new_inthash();
  
  int64_t num = ih_getint64(MemHash,(int64_t) p);
  CurrMem -= num;
  ih_delval(MemHash,(int64_t) p);
}

void *falcon_realloc(void *p, size_t num) {
  falcon_unsetmem(p);
  void *pnew = realloc(p,num);
  falcon_setmem(pnew,num);
  return pnew;
}

void *falcon_calloc(size_t num, size_t size) {
  void *p = calloc(num,size);
  falcon_setmem(p,num*size);
  return p;
}

void *falcon_malloc(size_t num) {
  void *p = malloc(num);
  falcon_setmem(p,num);
  return p;
}

void falcon_free(void *p) {
  falcon_unsetmem(p);
  free(p);  
  return;
}
///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////
/*
  hashing code from Peter Behroozi
  lifted under GPL 
  some of the system calls have been switched back
  to standard versions
 */

#define MAX_LOAD_FACTOR 0.7

struct inthash *new_inthash(void) {
  /*struct inthash *ih = check_realloc(NULL, sizeof(struct inthash),
    "Allocating inthash.");*/
  struct inthash *ih = (struct inthash*)realloc(NULL, sizeof(struct inthash));
  assert(ih != NULL);
  
  int64_t i;
  memset(ih, 0, sizeof(struct inthash));
  ih->hashnum = rand() + 
    (((uint64_t)rand())<<(uint64_t)32) + (uint64_t)(rand());
  ih->hashwidth = 8;
  ih->num_buckets = (uint64_t)1 << ih->hashwidth;
  
  /*ih->buckets = check_realloc(NULL, sizeof(struct intbucket)*ih->num_buckets,
    "Allocating hash buckets.");*/
  ih->buckets = (struct intbucket*)realloc(NULL, sizeof(struct intbucket)*ih->num_buckets);
  assert(ih->buckets != NULL);
  
  memset(ih->buckets, 0, sizeof(struct intbucket)*ih->num_buckets);
  for (i=0; i<ih->num_buckets; i++) ih->buckets[i].key = IH_INVALID;
  if (!(ih->hashnum & 1)) ih->hashnum++;
  return ih;
}

int64_t *ih_keylist(struct inthash *ih) {
  int64_t i, j=0;
  //int64_t *kl = check_realloc(NULL, sizeof(int64_t)*ih->elems, "Allocating key list.");
  int64_t *kl = (int64_t*)realloc(NULL, sizeof(int64_t)*ih->elems);
  assert(kl != NULL);
  
  struct intbucket *ib = NULL;
  for (i=0; i<ih->num_buckets; i++) {
    ib = ih->buckets + i;
    if (ib->key>IH_SKIP) continue;
    kl[j]=ib->key;
    j++;
  }
  return kl;
}


static inline uint64_t _ih_hash_function(struct inthash *ih, uint64_t key) {
  return ((key*ih->hashnum)>>(64 - ih->hashwidth));
}

static void _ih_add_more_buckets(struct inthash *ih, int64_t add_factor) {
  int64_t i;
  int64_t old_num_buckets = ih->num_buckets;
  struct intbucket *new_buckets, *ib, *newb;

  ih->num_buckets <<= add_factor;
  int64_t new_alloc_size = sizeof(struct intbucket)*(ih->num_buckets);

  //new_buckets = check_realloc(NULL, new_alloc_size, "Allocating new buckets.");
  new_buckets = (struct intbucket*)realloc(NULL, new_alloc_size);
  assert(new_buckets != NULL);
  
  memset(new_buckets, 0, new_alloc_size);
  for (i=0; i<ih->num_buckets; i++) new_buckets[i].key = IH_INVALID;

  ih->hashwidth+=add_factor;
  for (i=0; i<old_num_buckets; i++) {
    ib = ih->buckets + i;
    if (ib->key>IH_SKIP) continue;
    int64_t key = ib->key;
    do {
      newb = new_buckets + _ih_hash_function(ih, key);
      key = key*key + key + 3;
    } while (newb->key!=IH_INVALID);
    newb->key = ib->key;
    newb->data = ib->data;
  }

  free(ih->buckets);
  ih->buckets = new_buckets;
}


static void ih_prealloc(struct inthash *ih, int64_t size) {
  if (size <= 0) return;
  int64_t numbits = ceil(log(size/MAX_LOAD_FACTOR)/log(2));
  if (numbits <= ih->hashwidth) return;
  _ih_add_more_buckets(ih, numbits-ih->hashwidth);
}

static inline struct intbucket *_ih_getval(struct inthash *ih, int64_t key) {
  struct intbucket *ib = ih->buckets + _ih_hash_function(ih, key);
  int64_t key2 = key;
  while (ib->key!=IH_INVALID) {
    if (ib->key == key) return ib;
    key2 = key2*key2 + key2 + 3;
    ib = ih->buckets + _ih_hash_function(ih, key2);
  }
  return ib;
}

static inline struct intbucket *_ih_getval_deleted(struct inthash *ih, int64_t key) {
  struct intbucket *ib = ih->buckets + _ih_hash_function(ih, key);
  struct intbucket *ib_del = NULL;
  int64_t key2 = key;
  while (ib->key!=IH_INVALID) {
    if (ib->key == key) return ib;
    if (ib->key == IH_DELETED) ib_del = ib;
    key2 = key2*key2 + key2 + 3;
    ib = ih->buckets + _ih_hash_function(ih, key2);
  }
  if (ib_del) return ib_del;
  return ib;
}

void *ih_getval(struct inthash *ih, int64_t key) {
  struct intbucket *ib = _ih_getval(ih, key);
  if (ib->key==key) return ib->data;
  return NULL;
}

int64_t ih_getint64(struct inthash *ih, int64_t key) {
  struct intbucket *ib = _ih_getval(ih, key);
  if (ib->key==key) return (int64_t)ib->data;
  return IH_INVALID;
}

void ih_setint64(struct inthash *ih, int64_t key, int64_t value) {
  ih_setval(ih,key,(void *)value);
}

void ih_setval(struct inthash *ih, int64_t key, void *data) {
  struct intbucket *ib;
  ib = _ih_getval_deleted(ih, key);
  if (ib->key>IH_SKIP) {
    if (ih->elems>=ih->num_buckets*MAX_LOAD_FACTOR) {
      _ih_add_more_buckets(ih,1);
      ib = _ih_getval(ih, key);
    }
    ih->elems++;
    ib->key = key;
  }
  ib->data = data;
}

void ih_delval(struct inthash *ih, int64_t key) {
  struct intbucket *ib = _ih_getval(ih, key);
  if (key > IH_SKIP || ib->key != key) return;
  ib->key = IH_DELETED;
  ih->elems--;
}

void free_inthash(struct inthash *ih) {
  if (!ih) return;
  if (ih->buckets) {
    memset(ih->buckets, 0, sizeof(struct intbucket)*(ih->num_buckets));
    free(ih->buckets);
  }
  memset(ih, 0, sizeof(struct inthash));
  free(ih);
}

void free_inthash2(struct inthash *ih) {
  int64_t i;
  if (!ih) return;
  for (i=0; i<ih->num_buckets; i++)
    if (ih->buckets[i].key) free_inthash(ih->buckets[i].data);
  free_inthash(ih);
}


void ih_setval2(struct inthash *ih, int64_t key1, int64_t key2, void *data) {
  struct inthash *ih2 = ih_getval(ih, key1);
  if (!ih2) {
    ih2 = new_inthash();
    ih_setval(ih, key1, ih2);
  }
  ih_setval(ih2, key2, data);
}

void *ih_getval2(struct inthash *ih, int64_t key1, int64_t key2) {

  struct inthash *ih2 = ih_getval(ih, key1);
  if (!ih2) return NULL;
  return ih_getval(ih2, key2);
}

#undef IH_INVALID
#undef IH_DELETED
#undef IH_SKIP
#undef MAX_LOAD_FACTOR
