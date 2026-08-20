#include <oraccsys.h>
#include "lb.h"

void
lb_log_segs(Par *p)
{
  const char *bang = (p->xwords != p->re_xwords) ? "!" : "";
  fprintf(logfp, "&%s\t%s\t%d/%d\t%d\t%s%d\n", p->t->Q, p->label, p->lbgoal, p->nsegs, p->xwords, bang, p->re_xwords);
  int i;
  for (i = 0; p->segs[i]; ++i)
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
  const char *bang = (p->xwords != p->re_xwords) ? "!" : "";
#if 0
  fprintf(logfp, "&%s\t%s\t%d/%d\t%d\t%s%d\n", p->t->Q, p->label, p->lbgoal, p->nsegs, p->xwords, bang, p->re_xwords);
#endif
  int i;
  for (i = 0; p->segs[i]; ++i)
    {
      fprintf(logfp, "}%c\t", p->segs[i]->lb ? '+' : '-');
      fprintf(logfp, "%s\t", p->segs[i]->label ? p->segs[i]->label : "");
      fwrite(p->segs[i]->o, sizeof(char), p->segs[i]->c - p->segs[i]->o, logfp);
      fputc('\n', logfp);
    }
  fputs("======================================================\n", logfp);
}

