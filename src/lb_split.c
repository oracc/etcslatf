#include <oraccsys.h>
#include "lb.h"

static int
find_and(Par *p, const char **s, int *w)
{
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      const char *o = p->segs[i]->o;
      const char *and = strstr(o, " and ");
      if (and && and < p->segs[i]->c)
	{
	  *s = and+1;
	  int nw = 0;
	  while (and < p->segs[i]->c)
	    {
	      if (isspace(*and))
		++nw;
	      else if (ELLIPSIS(and))
		nw += 2;
	      ++and;
	    }
	  *w = (p->segs[i]->w - nw);
	  return i;
	}
    }
  return -1;
}

int
find_longest(Par *p, const char **s, int *w)
{
  int iw = 0;
  int wmax = 0;
  int imax = 0;

 retry:
  for (wmax = 0; iw < p->nsegs; ++iw)
    {
      if (p->segs[iw]->w > wmax)
	{
	  wmax = p->segs[iw]->w;
	  imax = iw;
	}
    }

  /* find a split point at a space after the halfway word mark */
  int nw = wmax / 2, xw = 0;
  const char *sp = p->segs[imax]->o;
  while (sp <= p->segs[imax]->c)
    {
      if (isspace(*sp))
	{
	  if (++xw == nw)
	    break;
	  else
	    ++sp;
	}
      else if (ELLIPSIS(sp))
	{
	  if ((xw+=2) < nw)
	    {
	      sp += 3;
	      if (!ELLIPSIS(sp))
		{
		  if ('.' == *sp || ',' == *sp)
		    ++sp;
		  break;
		}
	    }
	  else
	    break; /* we overflowed the word goal so this one won't work */
	}
      else
	++sp;
    }
  /* Did we fail to find enough words? */
  if (xw < nw || xw >= p->segs[imax]->w)
    {
      if ((imax+1) < p->nsegs)
	{
	  iw = imax+1;
	  goto retry;
	}

      if (!xw) /* there is no split point :( */
	{
	  fprintf(stderr, "%s:%s: no split point found in para\n", p->t->Q, p->label);
	  *s = NULL;
	  imax = -1;
	}
      else
	{
	  /* just return the first split point */
	  sp = p->segs[imax]->o;
	  while (sp <= p->segs[imax]->c)
	    {
	      if (isspace(*sp))
		{
		  xw = 1;
		  break;
		}
	      else if (ELLIPSIS(sp))
		{
		  xw = 3;
		  sp += 3;
		  break;
		}
	      else
		++sp;
	    }
	}
    }
  *s = (0xE2==*(ucp)sp)?sp:sp+1;
  *w = xw;
  return imax;
}

void
lb_split_segs(Memo *segmem, Par *p)
{
  while (p->nsegs < p->lbgoal)
    {
      const char *split = NULL;
      int w;
      int splitme = find_and(p, &split, &w);
      if (splitme < 0)
	splitme = find_longest(p, &split, &w);
      if (splitme >= 0)
	{
	  /* Make room for the new seg at splitme+1 by moving rest of
	     segs one to the right */
	  int nmove = p->nsegs - splitme;
	  p->segs = realloc(p->segs, (p->nsegs+2) * sizeof(Seg*));
	  memmove(&p->segs[splitme+2], &p->segs[splitme+1], nmove * sizeof(Seg*));

	  /* Create the new seg */
	  p->segs[splitme+1] = memo_new(segmem);

	  /* Save members for new seg */
	  const char *c = p->segs[splitme]->c;
	  int b = p->segs[splitme]->b;
	  int nw = p->segs[splitme]->w - w;

	  /* Reset the LHS of the segment being split */
	  p->segs[splitme]->c = split-1;
	  p->segs[splitme]->w = w;
	  p->segs[splitme]->b = ' ';
	  
	  /* set the new seg's members */
	  p->segs[splitme+1]->o = split;
	  p->segs[splitme+1]->c = c;
	  p->segs[splitme+1]->w = nw;
	  p->segs[splitme+1]->b = b;
	  p->segs[splitme+1]->p = p;

	  ++p->nsegs;
	}
      else
	{
	  fprintf(stderr, "%s:%s: lb_split_segs failed\n", p->t->Q, p->label);
	  break;
	}
    }
}
