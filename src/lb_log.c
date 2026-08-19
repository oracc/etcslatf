#include <oraccsys.h>
#include "lb.h"

void
lb_log_segs(const char *Q, const char *L, const char *G, const char *W, int new_xW, Seg **segs, int nsegs)
{
  const char *bang = (atoi(W) != new_xW) ? "!" : "";
  fprintf(logfp, "&%s\t%s\t%s/%d\t%s\t%s%d\n", Q, L, G, nsegs, W, bang, new_xW);
  int i;
  for (i = 0; segs[i]; ++i)
    {
      fprintf(logfp, ">%c\t%c\t%d\t", segs[i]->lb ? '+' : '-', segs[i]->b, segs[i]->w);
      fwrite(segs[i]->o, sizeof(char), segs[i]->c - segs[i]->o, logfp);
      fputc('\n', logfp);
    }
  fputs("======================================================\n", logfp);
}

void
lb_log_segs2(const char *Q, const char *L, const char *G, const char *W, int new_xW, Seg **segs, int nsegs)
{
  const char *bang = (atoi(W) != new_xW) ? "!" : "";
  fprintf(logfp, "&%s\t%s\t%s/%d\t%s\t%s%d\n", Q, L, G, nsegs, W, bang, new_xW);
  int i;
  for (i = 0; segs[i]; ++i)
    {
      fprintf(logfp, ">%c\t%c\t%d\t", segs[i]->lb ? '+' : '-', segs[i]->b, segs[i]->w);
      fwrite(segs[i]->o, sizeof(char), segs[i]->c - segs[i]->o, logfp);
      fputc('\n', logfp);
    }
  fputs("======================================================\n", logfp);
}
