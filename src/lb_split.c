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
	  *s = and;
	  int nw = 0;
	  while (and < p->segs[i]->c)
	    {
	      if (isspace(*and))
		++nw;
	      else if (ELLIPSIS(and))
		nw += 3;
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
  int i;
  int wmax = 0;
  int imax = 0;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (p->segs[i]->w > wmax)
	{
	  wmax = p->segs[i]->w;
	  imax = i;
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
	  if ((xw+=2) >= nw)
	    {
	      sp += 3;
	      break;
	    }
	  else
	    sp += 3;
	}
      else
	++sp;
    }
  /* Did we fail to find enough words? */
  if (xw < nw)
    {
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
  *s = sp;
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
	  /* Reset the rhs of the segment that is being split */
	  const char *o = p->segs[splitme]->o;
	  p->segs[splitme]->o = split; /* ->c stays the same */
	  p->segs[splitme]->w = p->segs[splitme]->w - w;
	  /* Move overlapping remainder of segs one to the right */
	  int nmove = p->nsegs - splitme;
	  memmove(&p->segs[splitme+1], &p->segs[splitme], nmove * sizeof(Seg*));
	  /* set the inserted seg's members */
	  p->segs[splitme] = memo_new(segmem);
	  p->segs[splitme]->o = o;
	  p->segs[splitme]->c = split;
	  p->segs[splitme]->w = w;
	  ++p->nsegs;
	}
      else
	break;
    }
}
