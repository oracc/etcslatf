#include <oraccsys.h>
#include "lb.h"

void
lbv_seg(Seg *s, int i)
{
  const char *v = s->o;
  while (v < s->c)
    {
      const char *x = strchr(v, CTRL_X);
      if (x)
	{
	  if (x < s->c)
	    {
	      ++x;
	      if (!strncmp(x, "@vart", 5))
		{
		  x = strchr(x, '[');
		  if (x)
		    {
		      ++x;
		      const char *id1 = x;
		      x = strchr(x, ',');
		      if (x)
			{
			  const char *id2 = x+1;
			  int j;
			  for (j = 0; id1[j] != ',' && id2[j] != ']'; ++j)
			    if (id1[j] != id2[j])
			      break;
			  if (id1[j] == ',')
			    {
			      fprintf(stderr, "lbv_seg:%s:%s: found @vari\n", s->p->t->Q, s->p->label);
			    }
			}
		    }
		}
	    }
	  v = x;
	}
      else
	break;
    }
}

void
lb_vari(Par *p)
{
  int i;
  for (i = 0; i < p->nsegs; ++i)
    lbv_seg(p->segs[i], i);
}
