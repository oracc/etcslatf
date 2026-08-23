#include <oraccsys.h>
#include "lb.h"

#if 0
static void lb_label(Par *p);
#endif

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

#define NSEGS(x) (x)->nsegs
#define NSEGSG(x) ((x)->nsegs-(x)->ngaps)

#define GOAL(x) (x)->lbgoal
#define GOALG(x) ((x)->lbgoal-(x)->ngaps)

void
lb_choose(Par *p)
{
  if (NSEGSG(p) == GOALG(p) || p->lbgoal == p->ngaps)
    lbc_identity(p);
  else if (GOALG(p) && (NSEGSG(p) % GOALG(p)) == 0)
    lbc_multiple(p, NSEGSG(p) / GOALG(p));
  else
    {
      int nsent = count_sentences(p->segs);
      if (nsent == GOALG(p))
	lbc_identity_sent(p);
      else if ((nsent % GOALG(p)) == 0)
	lbc_multiple_sent(p, nsent / GOALG(p));
      else if (p->nsegs > GOALG(p))
	lbc_fallbacks(p, nsent);
    }
#if 0
  if (p->choice != C_NONE)
    lb_label(p);
#endif
  lb_log_segs2(p);
}

#if 0
void
lb_label(Par *p)
{
  int i;
  int n = 0;
  int j = -1;
  /*p->segs[0]->label = p->labels[0];*/
  for (i = 0; i < p->nsegs; ++i)
    {
      if (-1 == j)
	j = i;

      if (p->segs[i]->lb)
	{
	  p->segs[j]->label = p->labels[n++];
	  j = -1;
	}
      else if ('0' == p->segs[i]->b)
	p->segs[i]->label = p->labels[n++];
    }
}
#endif

static void
lbc_identity(Par *p)
{
  int i;
  p->choice = C_IDENT;
  for (i = 0; i < p->nsegs; ++i)
    {
      p->segs[i]->label = p->labels[i];
      if ('0' != p->segs[i]->b)
	p->segs[i]->lb = 1;
    }
}

static void
lbc_multiple(Par *p, int n)
{
  p->choice = C_MULTI;
  int i;
  int j = 0;
  int head;
  int nlabel = 0;
  for (i = 0; i < p->nsegs; ++i)
    {
      if ('0' == p->segs[i]->b)
	{
	  p->segs[i]->label = p->labels[nlabel++];
	  continue;
	}
      else if (j == 0)
	{
	  p->segs[i]->label = p->labels[nlabel++];
	  head = i;
	  p->segs[i]->next = p->segs[i+1];
	  ++j;
	}
      else
	{
	  if (++j < n)
	    {
	      p->segs[i]->next = p->segs[i+1];
	      p->segs[i]->with = p->segs[head];
	    }
	  else
	    {
	      p->segs[i]->with = p->segs[head];
	      p->segs[i]->lb = 1;
	      j = 0;
	    }
	}
#if 0
      if (++j == n)
	{
	  p->segs[i]->with = p->segs[head];
	  p->segs[i]->lb = 1;
	  j = 0;
	}
#endif
    }
}

static void
lb_label_sent(Par *p)
{
  int head = 0;
  int nlabel = 0;
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      if ('0' == p->segs[i]->b)
	p->segs[i]->label = p->labels[nlabel++];
      else
	{
	  p->segs[i]->label = p->labels[nlabel++];
	  head = i;
	  while ((i+1) < p->nsegs && !p->segs[i]->lb)
	    {
	      p->segs[i]->next = p->segs[i+1];
	      ++i;
	      p->segs[i]->with = p->segs[head];
	    }
	  head = -1;
	}
    }
}

static void
lbc_identity_sent(Par *p)
{
  p->choice = C_IDENT_SENT;
  int i;
  for (i = 0; i < p->nsegs; ++i)
    {
      if ('0' == p->segs[i]->b)
	continue;
      if ('.' == p->segs[i]->b)
	p->segs[i]->lb = 1;
    }
  lb_label_sent(p);
}

static void
lbc_multiple_sent(Par *p, int n)
{
  p->choice = C_MULTI_SENT;
  int i;
  int j = 0;
  for (i = 0; i < p->nsegs; ++i)
    {
      if ('0' == p->segs[i]->b)
	continue;
      if ('.' == p->segs[i]->b)
	{
	  if (++j == n)
	    {
	      p->segs[i]->lb = 1;
	      j = 0;
	    }
	}
    }
  lb_label_sent(p);
}

/**********************************************************************
 *
 * If the easy choices don't work, we try a variety of fallbacks to
 * find the best set of choices, defined as the set of choices that
 * has the least total divergence from the expected words for this
 * para.
 *
 */

static void lbf_brute_pairs(Par *p);
static int lbf_short_pairs(Par *p, int nset, int *ss);
static SSC *lb_best_fallback(List *ssl);
static void lbc_maybe_fb(Par *p, List *ssl, int *ss, Choice c);
static void lb_merge(Par *p, SSC*s);
static int lb_new_ss(List *ssl, int *ss, int nss);

static void
lbc_fallbacks(Par *p, int nsent)
{
  List *ssl = list_create(LIST_SINGLE);
  int *ss = calloc(NSEGS(p), sizeof(int));

  if (lbf_short_pairs(p, nsent, ss) > 0)
    lbc_maybe_fb(p, ssl, ss, C_FB_SHORT_PAIRS);

  if (list_len(ssl) == 0)
    lbf_brute_pairs(p);

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
  else if (lb_new_ss(ssl, ss, NSEGS(p)))
    {
      list_add(ssl, ss_c(ss, c));
      ss = calloc(NSEGS(p), sizeof(int));
    }
  else
    memset(ss, '\0', NSEGS(p) * sizeof(int));
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
  SSC *best = NULL;
  return best;
}

static int
want_next(Par *p, int i)
{
  if ((i+1) < NSEGS(p) && '0' == p->segs[i+1]->lb)
    return 0;
  if (i == 0 && p->segs[i]->w && p->segs[i]->w < (p->re_xwords/2))
    return 2;
  else if ((i+1) < NSEGS(p) && p->segs[i]->w && p->segs[i+1]->w < p->re_xwords)
    return 1;
  return 0;
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
  int nmerge = 0;
  for (i = 0; i < NSEGS(p); ++i)
    {
      if ('0' == p->segs[i]->b)
	continue;
      if (p->segs[i]->w && p->segs[i]->w < p->re_xwords)
	{
	  int want = want_next(p, i);
	  if (want)
	    {
	      ++nshort;
	      int j = i+1;
	      int w = p->segs[i]->w;
	      ss[i] = 2;
	      while (j < NSEGS(p) && (want==2||p->segs[j]->w < p->re_xwords))
		{
		  ++nmerge;
		  ss[j] = 1;
		  w += p->segs[j++]->w;
		  if (w >= p->re_xwords || (NSEGSG(p) - nmerge) == GOALG(p))
		    break;
		}
	      i = j-1;
	    }
	}
      if ((NSEGSG(p) - nmerge) == GOALG(p))
	break;
    }
  if (nshort)
    {
      if ((NSEGSG(p) - nmerge) != GOALG(p))
	{
	  memset(ss, '\0', NSEGS(p) * sizeof(int));
	  return -1;
	}
      else
	{
#if 0
	  fprintf(stderr, "lbf_short_pairs: %s: ", p->label);
	  for (i = 0; i < NSEGS(p); ++i)
	    fputc(ss[i]?(ss[i]==2?'2':'1'):'0',stderr);
	  fputc('\n',stderr);
#endif
	  return 1;
	}
    }
  else
    return 0;
}

static void
lbf_exec_merge(Par *p, int m)
{
  /* we are going to overwrite m with m+1 so adjust m+1's data
   * accordingly */
  p->segs[m+1]->o = p->segs[m]->o;
  p->segs[m+1]->w += p->segs[m]->w;
  memmove(&p->segs[m], &p->segs[m+1], (p->nsegs-m-1)*sizeof(Seg*));
  --p->nsegs;
}

static int
lbf_find_merge(Par *p)
{
  int i;
  int wmin = 100; /* smallest seg[i]->w for merge */
  int wsum = 1000; /* current sum seg[i]->w+seg[i+1]->w */
  int next = -1;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (p->segs[i]->w < wmin)
	{
	  if ((i+1) < p->nsegs && '0' != p->segs[i+1]->b)
	    {
	      if (p->segs[i]->w + p->segs[i+1]->w < wsum)
		{
		  next = i;
		  wmin = p->segs[i]->w;
		  wsum = p->segs[i]->w + p->segs[i+1]->w;
		}
	    }
	}
    }
  return next;
}

static void
lbf_brute_pairs(Par *p)
{
  while (NSEGSG(p) != GOALG(p))
    {
      int m = lbf_find_merge(p);
      if (m >= 0)
	lbf_exec_merge(p, m);
      else
	{
	  fprintf(stderr, "%s:%s: lbf_find_merge failed\n", p->t->Q, p->label);
	  break;
	}
    }
  p->choice = C_FB_BRUTE;
  int i;
  for (i = 0; i < p->nsegs; ++i)
    p->segs[i]->label = p->labels[i];
}

static char *
ss_str(int *ss, int n)
{
  int i;
  char s[n+1];
  for (i = 0; i < n; ++i)
    s[i] = ss[i]?(ss[i]==2?'2':'1'):'0';
  s[i] = '\0';
  return strdup(s);
}

void
lb_merge(Par *p, SSC *s)
{
  int i, head;
  int *ss = s->ss;
  p->choice = s->c;
  p->ss_str = ss_str(ss, NSEGS(p));
  int nlabel = 0;
  for (i = 0; i < NSEGS(p); ++i)
    {
      if (ss[i] == 2)
	{
	  p->segs[i]->label = p->labels[nlabel++];
	  head = i;
	  int j = i+1;
	  while (j < NSEGS(p) && ss[j] == 1)
	    {
	      p->segs[j-1]->next = p->segs[j];
	      p->segs[head]->w += p->segs[j]->w;
	      p->segs[j]->with = p->segs[head];
	      ++j;
	    }
	  i = j-1;
	  
	}
      else
	p->segs[i]->label = p->labels[nlabel++];
    }
}
