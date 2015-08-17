#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "heron.h"

int main() {

  int *p1;
  int *p2;
  int *p3;
  int num[3] = {1000,10000000,3223};
  long use;

  heron_print_off();
  
  p1 = malloc(num[0]*sizeof(double));
  assert(p1 != NULL);

  p2 = calloc(num[1],sizeof(float));
  assert(p2 != NULL);

  heron_print_on();

  p3 = malloc(num[2]*sizeof(float));
  assert(p3 != NULL);

  use = heron_memuse();  
  if(use != num[0]*sizeof(double) + num[1]*sizeof(float) + num[2]*sizeof(float))
    fprintf(stderr,"ERROR %ld, should be %ld\n\n",use,num[0]*sizeof(double) + num[1]*sizeof(float) + num[2]*sizeof(float));

  free(p2);
  use = heron_memuse();
  if(use != num[0]*sizeof(double) + num[2]*sizeof(float))
    fprintf(stderr,"ERROR %ld, should be %ld\n\n",use,num[0]*sizeof(double) + num[2]*sizeof(float));

  p3 = realloc(p3,num[2]*sizeof(double));

  use = heron_memuse();
  if(use != num[0]*sizeof(double) + num[2]*sizeof(double))
    fprintf(stderr,"ERROR: %ld, should be %ld\n\n",use,num[0]*sizeof(double) + num[2]*sizeof(double));

  free(p1);
  free(p3);
  use = heron_memuse();
  if(use != 0)
    fprintf(stderr,"ERROR %ld, should be 0\n\n",use);

  use = heron_highwater();
  if(use != num[0]*sizeof(double) + num[1]*sizeof(float) + num[2]*sizeof(float))
    fprintf(stderr,"ERROR %ld, should be %ld\n\n",use,num[0]*sizeof(double) + num[1]*sizeof(float) + num[2]*sizeof(float));

  heron_cleanup();

  return 0;
}
