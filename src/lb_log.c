#include <oraccsys.h>
#include "lb.h"

void
lb_log_segs(Par *p)
{
  const char *bang = (p->xwords != p->re_xwords) ? "!" : "";
  fprintf(logfp, "&%s\t%s\t%d/%d\t%d\t%s%d\n", p->t->Q, p->label, p->lbgoal, p->nsegs, p->xwords, bang, p->re_xwords);
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      fprintf(logfp, ">%c\t%c\t%d\t", p->segs[i]->lb ? '+' : '-', p->segs[i]->b, p->segs[i]->w);
      fwrite(p->segs[i]->o, sizeof(char), p->segs[i]->c - p->segs[i]->o, logfp);
      fputc('\n', logfp);
    }
  fputs("------------------------------------------------------\n", logfp);
}

void
lb_log_segs2(Par *p)
{
  fprintf(logfp, "{%s}", cnames[p->choice]);
  if (p->ss_str)
    fprintf(logfp, " ss=%s", p->ss_str);
  fputc('\n', logfp);
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (!p->segs[i]->with)
	{
	  const char *label = p->segs[i]->label ? p->segs[i]->label : "";
	  if (p->choice != C_NONE)
	    {
	      if (!*label)
		fprintf(stderr, "%s:%s: no label on segment %d\n", p->t->Q, p->label, i);
	      else if (p->segs[i]->c - p->segs[i]->o == 1 && isspace(*p->segs[i]->o))
		fprintf(stderr, "%s:%s: segment is a single space\n", p->t->Q, p->label);
	    }
	  fprintf(logfp, "}%c\t", p->segs[i]->lb ? '+' : '-');
	  fprintf(logfp, "%s\t", label);
	  size_t len = p->segs[i]->c - p->segs[i]->o;
	  if (len > 1000000)
	    fprintf(stderr, "%s:%s: oversize segment (length %ld) will be dropped\n", p->t->Q, p->label, len);
	  else
	    fwrite(p->segs[i]->o, sizeof(char), len, logfp);
	  if (p->segs[i]->next)
	    {
	      int j = i;
	      int need_0 = 0;
	      do
		{
		  ++j;
		  fputc(' ', logfp);
		  len = p->segs[i]->c - p->segs[i]->o;
		  if (len > 100000)
		    fprintf(stderr, "%s:%s: oversize segment (length %ld) will be dropped\n", p->t->Q, p->label, len);
		  else
		    fwrite(p->segs[j]->o, sizeof(char), p->segs[j]->c - p->segs[j]->o, logfp);
		  ++need_0;
		}
	      while (p->segs[j]->next);
	      i = j-1;
	      while (need_0-- > 0)
		fputs("\n}0\t", logfp);
	    }
	  fputc('\n', logfp);
	}
    }
  fputs("======================================================\n", logfp);
}

