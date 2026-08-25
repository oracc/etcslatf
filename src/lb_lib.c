#include <oraccsys.h>
#include "lb.h"

void
lb_merge_segs(Par *p, int m)
{
  /* we are going to overwrite m with m+1 so adjust m+1's data
   * accordingly */
  p->segs[m+1]->o = p->segs[m]->o;
  p->segs[m+1]->w += p->segs[m]->w;
  memmove(&p->segs[m], &p->segs[m+1], (p->nsegs-m-1)*sizeof(Seg*));
  --p->nsegs;
  --p->nlabs;
}
