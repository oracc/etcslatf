#include <oraccsys.h>
#include "lb.h"

static int count_sentences(Seg**segs);
static void lbc_identity(Par *p);
static void lbc_multiple(Par *p);
static void lbc_identity_sent(Par *p);
static void lbc_multiple_sent(Par *p);

void
lb_choose(Par *p)
{
  if (p->nsegs == p->lbgoal)
    lbc_identity(p);
  else if ((p->nsegs % p->lbgoal) == 0)
    lbc_multiple(p);
  else
    {
      int nsent = count_sentences(p->segs);
      if (nsent == p->lbgoal)
	lbc_identity_sent(p);
      else if ((nsent % p->lbgoal) == 0)
	lbc_multiple_sent(p);
    }
  lb_log_segs2(p);
}

static void
lbc_identity(Par *p)
{
  int i;
  for (i = 0; p->segs[i]; ++i)
    {
      p->segs[i]->label = p->labels[i];
      p->segs[i]->lb = 1;
    }
}

static void
lbc_multiple(Par *p)
{
}
static void
lbc_identity_sent(Par *p)
{
}
static void
lbc_multiple_sent(Par *p)
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
