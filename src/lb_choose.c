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

#define NSEGS(x) (x)->nsegs
#define NSEGSG(x) ((x)->nsegs-(x)->ngaps)

#define GOAL(x) (x)->lbgoal
#define GOALG(x) ((x)->lbgoal-(x)->ngaps)

void
lb_choose(Par *p)
{
  if (NSEGSG(p) == GOALG(p))
    lbc_identity(p);
  else if ((NSEGSG(p) % GOALG(p)) == 0)
    lbc_multiple(p, NSEGSG(p) / GOALG(p));
  else
    {
      int nsent = count_sentences(p->segs);
      if (nsent == GOALG(p))
	lbc_identity_sent(p);
      else if ((nsent % GOALG(p)) == 0)
	lbc_multiple_sent(p, nsent / GOALG(p));
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
    if ('0' != p->segs[i]->b)
      p->segs[i]->lb = 1;
}

static void
lbc_multiple(Par *p, int n)
{
  p->choice = C_MULTI;
  int i;
  int j = 0;
  int head;
  int nlabel = 0;
  for (i = 0; p->segs[i]; ++i)
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
lbc_identity_sent(Par *p)
{
  p->choice = C_IDENT_SENT;
  int i;
  int head = 0;
  int nlabel = 0;
  for (i = 0; p->segs[i]; ++i)
    {
      if ('0' == p->segs[i]->b)
	continue;
      if ('.' == p->segs[i]->b)
	p->segs[i]->lb = 1;
    }

  for (i = 0; p->segs[i]; ++i)
    {
      if ('0' == p->segs[i]->b)
	p->segs[i]->label = p->labels[nlabel++];
      else
	{
	  p->segs[i]->label = p->labels[nlabel++];
	  head = i;
	  while (i < p->nsegs && '.' != p->segs[i]->b)
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
lbc_multiple_sent(Par *p, int n)
{
  p->choice = C_MULTI_SENT;
  int i;
  int j = 0;
  for (i = 0; p->segs[i]; ++i)
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
  int *ss = calloc(NSEGS(p), sizeof(int));

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
#if 1
      if ((NSEGSG(p) - nmerge) == GOALG(p))
	break;
#endif
    }
  if (nshort)
    {
#if 0
      if (nshort+nsingle > GOAL(p))
	{
	  int shortest[nshort];
	  int sindex = 0;
	  for (i = 0; i < NSEGS(p); ++i)
	    {
	      if (ss[i] == 2)
		{
		  shortest[sindex] = p->segs[i]->w;
		  int j = i+1;
		  while (ss[j] == 1)
		    {
		      if (p->segs[j]->w < shortest[sindex])
			shortest[sindex] = p->segs[j]->w;
		      ++j;
		    }
		  i = j - 1;
		}
	    }
	  while (nshort && nshort+nsingle > GOAL(p))
	    {
	      int j = 0;
	      int smax = 0, lose = 0;
	      while (j < nshort)
		{
		  if (shortest[j] > smax)
		    {
		      smax = shortest[j];
		      lose = j;
		    }
		  ++j;
		}
	      int nth_short = 0;
	      for (i = 0; i < NSEGS(p); ++i)
		{
		  if (ss[i] == 2)
		    {
		      if (nth_short++ == lose)
			{
			  ss[i++] = 0;
			  while (ss[i] == 1)
			    ss[i++] = 0;
			  --nshort;
			}
		    }
		}
	    }
	}
#endif
      if /*(nshort+nsingle != GOAL(p))*/ ((NSEGSG(p) - nmerge) != GOALG(p))
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
