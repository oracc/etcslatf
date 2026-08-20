#include <oraccsys.h>
#include "lb.h"

static void lb_label(Par *p);
static int count_sentences(Seg**segs);
static void lbc_identity(Par *p);
static void lbc_multiple(Par *pp, int n);
static void lbc_identity_sent(Par *p);
static void lbc_multiple_sent(Par *p, int n);

void
lb_choose(Par *p)
{
  if (p->nsegs == p->lbgoal)
    lbc_identity(p);
  else if ((p->nsegs % p->lbgoal) == 0)
    lbc_multiple(p, p->nsegs / p->lbgoal);
  else
    {
      int nsent = count_sentences(p->segs);
      if (nsent == p->lbgoal)
	lbc_identity_sent(p);
      else if ((nsent % p->lbgoal) == 0)
	lbc_multiple_sent(p, nsent / p->lbgoal);
    }
  if (p->choice != C_NONE)
    lb_label(p);
  lb_log_segs2(p);
}

void
lb_label(Par *p)
{
  int i;
  int n = 0;
  int j = -1;
  /*p->segs[0]->label = p->labels[0];*/
  for (i = 0; p->segs[i]; ++i)
    {
      if (-1 == j)
	j = i;

      if (p->segs[i]->lb)
	{
	  p->segs[j]->label = p->labels[n++];
	  j = -1;
	}
    }
}

static void
lbc_identity(Par *p)
{
  int i;
  p->choice = C_IDENT;
  for (i = 0; p->segs[i]; ++i)
    p->segs[i]->lb = 1;
}

static void
lbc_multiple(Par *p, int n)
{
  p->choice = C_MULTI;
  int i;
  int j = 0;
  for (i = 0; p->segs[i]; ++i)
    {
      if (++j == n)
	{
	  p->segs[i]->lb = 1;
	  j = 0;
	}
    }
}

static void
lbc_identity_sent(Par *p)
{
  p->choice = C_IDENT_SENT;
  int i;
  for (i = 0; p->segs[i]; ++i)
    {
      if ('.' == p->segs[i]->b)
	p->segs[i]->lb = 1;
    }
}

static void
lbc_multiple_sent(Par *p, int n)
{
  p->choice = C_MULTI_SENT;
  int i;
  int j = 0;
  for (i = 0; p->segs[i]; ++i)
    {
      if ('.' == p->segs[i]->b)
	{
	  if (++j == n)
	    {
	      p->segs[i]->lb = 1;
	      j = 0;
	    }
	}
    }
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
