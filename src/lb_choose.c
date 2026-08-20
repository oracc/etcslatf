#include <oraccsys.h>
#include "lb.h"

static void lb_label(Par *p);
static int count_sentences(Seg**segs);
static void lbc_identity(Par *p);
static void lbc_multiple(Par *pp, int n);
static void lbc_identity_sent(Par *p);
static void lbc_multiple_sent(Par *p, int n);
static void lbc_fallbacks(Par *p, int sent);

static int
count_sentences(Seg **segs)
{
  int i, s;
  for (i = s = 0; segs[i]; ++i)
    if ('.' == segs[i]->b)
      ++s;
  return s;
}

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
      else
	lbc_fallbacks(p, nsent);
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

/**********************************************************************
 *
 * If the easy choices don't work, we try a variety of fallbacks to
 * find the best set of choices, defined as the set of choices that
 * has the least total divergence from the expected words for this
 * para.
 *
 */

static int lbf_short_pairs(Par *p, int nset, int *ss);
static SSC *lb_best_fallback(List *ssl);
static void lbc_maybe_fb(Par *p, List *ssl, int *ss, Choice c);
static void lb_merge(Par *p, SSC*s);
static int lb_new_ss(List *ssl, int *ss, int nss);

static void
lbc_fallbacks(Par *p, int nsent)
{
  List *ssl = list_create(LIST_SINGLE);
  int fb[5] = { 0 , 0 , 0 , 0 , 0 };
  int ntries = 0;
  int *ss = calloc(p->nsegs, sizeof(int));

  if (lbf_short_pairs(p, nsent, ss) > 0)
    lbc_maybe_fb(p, ssl, ss, C_FB_SHORT_PAIRS);
  
  /* More fallbacks go here */
  /* set c to next C_FB_N */
  /* try fallback */
  /* add if new */
  /* set fb[tries] to c */

  SSC *ssc = NULL;

  if (list_len(ssl) > 1)
    ssc = lb_best_fallback(ssl);
  else if (list_len(ssl) == 1)
    ssc = list_first(ssl);

  if (ssc)
    lb_merge(p, ssc);
}

static SSC *
ss_c(int *ss, Choice c)
{
  SSC *s = memo_new(m_ssc);
  s->ss = ss;
  s->c = c;
  return s;
}

static void
lbc_maybe_fb(Par *p, List *ssl, int *ss, Choice c)
{
  if (list_len(ssl) == 0)
    list_add(ssl, ss_c(ss, c));
  else if (lb_new_ss(ssl, ss, p->nsegs))
    {
      list_add(ssl, ss_c(ss, c));
      ss = calloc(p->nsegs, sizeof(int));
    }
  else
    memset(ss, '\0', p->nsegs * sizeof(int));	
}

static int
lb_new_ss(List *ssl, int *ss, int nss)
{
  int *ssl_ss;
  for (ssl_ss = list_first(ssl); ssl_ss; ssl_ss = list_next(ssl))
    if (!memcmp(ssl_ss, ss, nss*sizeof(int)))
      return 0;
  return 1;
}

/* Compute the divergence of each candidate fallback from the goal and
 * select that as the best; if there are multiple solutions with the
 * same divergence, pick the first.
 */
static SSC *
lb_best_fallback(List *ssl)
{
  SSC *best;
  return best;
}

/* If we are n segments short of goal, are there n sequences of
 * segments each of which is shorter than the expected words and whose
 * combined length is approximately the same as the goal?
 */
static int
lbf_short_pairs(Par *p, int nsent, int *ss)
{
  int i;
  int nshort = 0;
  int nsingle = 0;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (p->segs[i]->w < p->re_xwords)
	{
	  if ((i+1) < p->nsegs && p->segs[i+1]->w < p->re_xwords)
	    {
	      ++nshort;
	      int j = i+1;
	      int w = p->segs[i]->w;
	      ss[i] = 1;
	      while (j < p->nsegs && p->segs[j]->w < p->re_xwords)
		{
		  ss[j] = 1;
		  w += p->segs[j++]->w;
		  if (w >= p->re_xwords)
		    break;
		}
	      i = j-1;
	    }
	  else
	    ++nsingle;
	}
      else
	++nsingle;
    }
  if (nshort)
    {
      if (nshort+nsingle != p->lbgoal)
	{
	  memset(ss, '\0', p->nsegs * sizeof(int));
	  return -1;
	}
      else
	return 1;
    }
  else
    return 0;
}

void
lb_merge(Par *p, SSC *s)
{
  int i, head;
  int *ss = s->ss;
  p->choice = s->c;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (ss[i])
	{
	  head = i;
	  int j = i+1;
	  while (j < p->nsegs && ss[j])
	    {
	      p->segs[j-1]->next = p->segs[j];
	      p->segs[head]->w += p->segs[j]->w;
	      p->segs[j]->with = p->segs[head];
	      ++j;
	    }
	  i = j-1;
	}
    }
}
