#include <oraccsys.h>
#include "lb.h"

#if 0
static int
find_and(Par *p, const char **s, int *w)
{
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      const char *o = p->segs[i]->o;
      /* prevent matching at start of segment */
      while (isspace(*o))
	++o;
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
#endif

int
find_longest(Par *p, const char **s, int *w)
{
  int iw = 0;
  int wmax = 0;
  int imax = 0;
  int ellipsis_break = 0;

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
  if (wmax % 2)
    ++wmax;
  int nw = wmax / 2, xw = 0;
  const char *sp = p->segs[imax]->o;
  while (sp <= p->segs[imax]->c)
    {
      if (isspace(*sp))
	{
	  if (++xw >= nw)
	    break;
	  else
	    ++sp;
	}
      else if (ELLIPSIS(sp))
	{
	  /* there are normally two ellipses together in ETCSL corpus */
	  /* If both are less than the goal, process them and continue */
	  if (!xw || xw+4 < nw)
	    {
	      xw += 2;
	      sp += 3;
	      if (ELLIPSIS(sp))
		{
		  xw += 2;
		  sp += 3;
		}
	    }
	  else
	    {
	      /* take the break point before the ellipsis */
	      if ('.' == *sp || ',' == *sp)
		++sp;
	      ellipsis_break++;
	      break;
	    }
	}
      else if ('@' == *sp)
	sp = lb_skip_tag_and_arg(++sp);
      else if (CTRL_X == *sp)
	sp = lb_skip_XtoY(sp);
      else
	++sp;
    }
  /* Did we fail to find enough words? */
  if (!ellipsis_break && (xw < nw || xw >= p->segs[imax]->w))
    {
      if ((imax+1) < p->nsegs)
	{
	  iw = ++imax;
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
		  xw = 2;
		  sp += 3;
		  break;
		}
	      else if ('@' == *sp)
		sp = lb_skip_tag_and_arg(++sp);
	      else if (CTRL_X == *sp)
		sp = lb_skip_XtoY(sp);
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
  while (p->nlabs < p->re_goal)
    {
      const char *split = NULL;
      int w;

      int splitme = 0;
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
	  p->segs[splitme]->c = split;
	  p->segs[splitme]->w = w;
	  p->segs[splitme]->b = ' ';
	  
	  /* set the new seg's members */
	  p->segs[splitme+1]->o = split;
	  p->segs[splitme+1]->c = c;
	  p->segs[splitme+1]->w = nw;
	  p->segs[splitme+1]->b = b;
	  p->segs[splitme+1]->p = p;

	  ++p->nsegs;
	  ++p->nlabs;
	}
      else
	{
	  fprintf(stderr, "%s:%s: lb_split_segs failed: nlabs=%d; re_goal=%d\n",
		  p->t->Q, p->label, p->nlabs, p->re_goal);
	  break;
	}
    }
}
