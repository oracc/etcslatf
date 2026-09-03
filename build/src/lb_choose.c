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
static void lb_sentence_orphans(Par *p);

unsigned char sentch[256] = { ['.'] = 1, ['!'] = 1, ['?'] = 1 };

static int
count_sentences(Seg **segs)
{
  int i, s;
  for (i = s = 0; segs[i]; ++i)
    if (sentch[segs[i]->b])
      ++s;
  return s;
}

#define NSEGS(x) (x)->nsegs
#define NSEGSG(x) ((x)->nsegs-(x)->ngaps)
#define NSEGSL(x) ((x)->nlabs)

#define GOAL(x) (x)->lbgoal
#define GOALG(x) ((x)->lbgoal-(x)->ngaps)
#define RE_GOAL(x) ((x)->re_goal)

void
lb_choose(Par *p)
{
  if (NSEGSL(p) == RE_GOAL(p))
    lbc_identity(p);
  else if (GOALG(p) && (NSEGSL(p) % RE_GOAL(p)) == 0)
    lbc_multiple(p, NSEGSL(p) / RE_GOAL(p));
  else
    {
      int nsent = count_sentences(p->segs);
      if (nsent == GOALG(p))
	lbc_identity_sent(p);
      else if ((nsent % GOALG(p)) == 0)
	lbc_multiple_sent(p, nsent / GOALG(p));
      else
	{
	  extern Memo *segmem;
	  lb_sentence_orphans(p);
	  if (p->nlabs < p->re_goal)
	    lb_split_segs(segmem, p);
	  if (p->nlabs == p->re_goal)
	    lbc_identity(p);
	  else if (p->nlabs > p->re_goal)
	    lbc_fallbacks(p, nsent);
	  else
	    fprintf(stderr, "lb_choose: internal error: p->nlabs %d < p->re_goal %d; this can't happen\n",
		    p->nlabs, p->re_goal);
	}
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
      if (!p->segs[i]->unlabeled)
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
      if (p->segs[i]->unlabeled)
	{
	  nlabel += p->segs[i]->unlabeled;
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
      if (p->segs[i]->unlabeled)
	p->segs[i]->label = p->labels[nlabel += p->segs[i]->unlabeled];
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
      if (sentch[p->segs[i]->b])
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
      if (sentch[p->segs[i]->b])
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
	      while (j < NSEGS(p) && '0' != p->segs[j]->b && (want==2||p->segs[j]->w < p->re_xwords))
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
      if ((NSEGSL(p) - nmerge) == RE_GOAL(p))
	break;
    }
  if (nshort)
    {
      if ((NSEGSL(p) - nmerge) != RE_GOAL(p))
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

static int
lbf_find_merge(Par *p)
{
  int i;
  int wmin = 100; /* smallest seg[i]->w for merge */
  int wsum = 1000; /* current sum seg[i]->w+seg[i+1]->w */
  int next = -1;
  for (i = 0; i < p->nsegs; ++i)
    {
      if ('0' == p->segs[i]->b)
	continue;
      
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
  while (NSEGSL(p) != RE_GOAL(p))
    {
      int m = lbf_find_merge(p);
      if (m >= 0)
	lb_merge_segs(p, m);
      else
	{
	  fprintf(stderr, "%s:%s: lbf_find_merge failed\n", p->t->Q, p->label);
	  break;
	}
    }
  p->choice = C_FB_BRUTE;
  int i;
  int nlabel = 0;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (p->segs[i]->unlabeled)
	nlabel += p->segs[i]->unlabeled;
      else
	p->segs[i]->label = p->labels[nlabel++];
    }
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
      else if (p->segs[i]->unlabeled)
	nlabel += p->segs[i]->unlabeled;
      else
	p->segs[i]->label = p->labels[nlabel++];
    }
}

/* Look for short segments that end a sentence and merge them with the
   preceding seg */
static void
lb_sentence_orphans(Par *p)
{
  int i;
  int few = p->re_xwords / 2;
  if (few < 1)
    few = 1;
  for (i = 0; i < p->nsegs; ++i)
    {
      if (i && sentch[p->segs[i]->b] && p->segs[i]->w <= few)
	{
	  if (',' == p->segs[i-1]->b || p->segs[i-1]->w <= few)
	    {
	      lb_merge_segs(p, i-1); /* arg is the segment to overwrite */
	      /* take another look at the newly merged seg to catch multiple shorts in a row */
	      if (i > 1)
		i -= 2;
	    }
	}
    }
}
