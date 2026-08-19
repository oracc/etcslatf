#include <oraccsys.h>
#include "lb.h"

static int count_sentences(Seg**segs);
static void lbc_identity(Seg **segs, const char **labels);
static void lbc_multiple(Seg **segs, const char **labels);
static void lbc_identity_sent(Seg **segs, const char **labels);
static void lbc_multiple_sent(Seg **segs, const char **labels);

void
lb_choose(const char *Q, const char *L, const char *G, const char *W, int new_xW,
	  Seg**segs, int nsegs, const char **labels)
{
  int nG = atoi(G);
  if (nsegs == nG)
    lbc_identity(segs, labels);
  else if ((nsegs % nG) == 0)
    lbc_multiple(segs, labels);
  else
    {
      int nsent = count_sentences(segs);
      if (nsent == nG)
	lbc_identity_sent(segs, labels);
      else if ((nsent % nG) == 0)
	lbc_multiple_sent(segs, labels);
    }
  lb_log_segs2(Q,L,G,W,new_xW,segs,nsegs);
}

static void
lbc_identity(Seg **segs, const char **ll)
{
  int i;
  for (i = 0; segs[i]; ++i)
    {
      segs[i]->label = ll[i];
      segs[i]->lb = 1;
    }
}

static void
lbc_multiple(Seg **segs, const char **labels)
{
}
static void
lbc_identity_sent(Seg **segs, const char **labels)
{
}
static void
lbc_multiple_sent(Seg **segs, const char **labels)
{
}

static int
count_sentences(Seg **segs)
{
  int i, s;
  for (i = s = 0; segs[i]; ++i)
    if ('.' == segs[i]->b)
      ++s;
  return s;
}
