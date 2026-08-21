#include <oraccsys.h>
#include <list.h>
#include <roco.h>
#include <memo.h>

#include "lb.h"

unsigned char bounds[256] =
  {
    ['.'] = 1,
    ['!'] = 1,
    ['?'] = 1,
    [','] = 1,
    [';'] = 1,
    [':'] = 1,
    ['-'] = 1,
  };

const char *cnames[] =
  {
    "NONE" , "IDENT", "MULTI", "IDENT_S", "MULTI_S",
    "FB_SHORT_PAIRS" ,
    NULL
  };

Memo *m_ssc;
const char *lbfn;
char *logfn;
FILE *logfp;

int count_words(Seg **s);
const char *find_closer(const char *p);
Seg **map_segs(Par *p);
const char *next_boundary(const char *p, int *nwords, int *b);
const char *skip_tag_and_arg(const char *p);
const char *skip_XtoY(const char *p);

int
main(int argc, char *const *argv)
{
  lbfn = argv[1];
  fprintf(stderr, "lb: processing %s\n", lbfn);
  Roco *r = roco_load1(lbfn);
  m_ssc = memo_init(sizeof(SSC),32);
  logfn = strdup(lbfn);
  strcpy(&logfn[strlen(logfn)-3], "log");
  logfp = fopen(logfn, "w");

  Tra *tp = calloc(1, sizeof(Tra));
  tp->Q = (ccp)r->rows[0][0];
  tp->pars = calloc(r->nlines, sizeof(Par));
  tp->npars = r->nlines;

  int i;
  for (i = 0; i < r->nlines; ++i)
    {
      int nsegs;
      const char **rr = (const char**)r->rows[i];
      tp->pars[i].t = tp;
      tp->pars[i].label = lb_L(rr);
      tp->pars[i].lbgoal = atoi(lb_G(rr));
      tp->pars[i].xwords = atoi(lb_W(rr));
      tp->pars[i].text = lb_P(rr);
      tp->pars[i].labels = (const char **)vec_from_str(strdup(lb_R(rr)),NULL,NULL);
      tp->pars[i].segs = map_segs(&tp->pars[i]);
      int new_nW = count_words(tp->pars[i].segs);
      tp->pars[i].re_xwords = new_nW / tp->pars[i].lbgoal;
      lb_log_segs(&tp->pars[i]);
      lb_choose(&tp->pars[i]);
    }
  fclose(logfp);
  lb_tra_report(stderr, tp);
  lb_print(stdout, tp);
}

int
count_words(Seg **s)
{
  int i, nW;
  for (i = nW = 0; s[i]; ++i)
    nW += s[i]->w;
  return nW;
}

/* This could be modified to handled nested (...(...)...); not clear
   it's necessary */
const char *
find_closer(const char *p)
{
  while (*p)
    if ('(' == *p)
      return NULL;
    else if (')' == *p)
      {
	++p;
	while (isspace(*p))
	  ++p;
	return p;
      }
    else
      ++p;
  return NULL;
}

const char *
map_gap(Memo *segmem, Par *par, const char *p, List *l)
{
  const char *s = p;
  while (*s && !isdigit(*s) && 0x19 != *s)
    ++s;
  if (isdigit(*s))
    {
      /*fprintf(stderr, "found digit(s) in gap %s\n", p);*/
      const char *line = strstr(s, "line");
      if (line)
	{
	  while (*line && !isspace(*line))
	    ++line;
	  while (isspace(*line))
	    ++line;
	  const char *end = line;
	  while (*end && '}' != *end)
	    ++end;
	  int ngap = atoi(s);
	  int i;
	  for (i = 0; i < ngap; ++i)
	    {
	      Seg *s = memo_new(segmem);
	      list_add(l, s);
	      s->w = 0;
	      s->b = '0';
	      s->o = line;
	      s->c = end;
	      ++par->ngaps;
	    }
	}
      else
	fprintf(stderr, "no 'line' in @gap\n"); /* never happens in ETCSL TEI corpus */
    }
  else
    fprintf(stderr, "no digit in gap %s\n", p);

  while (CTRL_Y != *p)
    ++p;
 
  return p + 1;
}

Seg **
map_segs(Par *par)
{
  Memo *segmem = memo_init(sizeof(Seg), 32);
  List *l = list_create(LIST_SINGLE);
  const char *p = par->text;
  while (*p)
    {
#if 0
      if (CTRL_X == *p)
	p = skip_XtoY(p);
      else
#endif
      if ('(' == *p)
	p = find_closer(++p);
      if (*p)
	{
	  const char *start = p;
	  Seg *s = memo_new(segmem);
	  list_add(l, s);
	  p = next_boundary(p, &s->w, &s->b);
	  if (p)
	    {
	      if (p - start)
		{
		  s->o = start;
		  s->c = p;
		}
	      else
		list_pop(l);

	      if (CTRL_X == *p) /* boundary was a \cX@gap */
		p = map_gap(segmem, par, p, l);
	      else if (*p)
		++p;
	    }
	  else
	    {
	      if (s->w)
		{
		  s->o = start;
		  s->c = s->o + strlen(s->o);
		}
	      break;
	    }
	}
    }
  Seg **sp = (Seg **)list2array(l);
  par->nsegs = list_len(l);
  list_free(l, NULL);
  return sp;
}

const char *
next_boundary(const char *p, int *nwords, int *b)
{
  int ellipsis = 0, nonsp = 0, sp = 1, words = 0;
  while (isspace(*p))
    ++p;
  const char *start = p; /* start of word count */
  while (*p)
    {
      if (isspace(*p))
	{
	  if (nonsp)
	    {
	      ++words;
	      nonsp = 0;
	    }
	  ++sp;
	  do
	    ++p;
	  while (isspace(*p));
	}
      else if (bounds[(unsigned char)*p])
	{
	  *b = *p;
	  int ok = 0;
	  if (EMDASH(p))
	    {
	      p += 2;
	      while (p[1] && isspace(p[1]))
		++p;
	      ok = 1;
	    }
	  else
	    {
	      ++p;
	      while (QUOTE(p) || CLOSER(p))
		{
		  if (0xE2 == (unsigned char)*p)
		    p += 3;
		  else
		    ++p;
		}
	      if (!*p)
		ok = 1;
	      else if (isspace(*p))
		{
		  while (p[1] && isspace(p[1]))
		    ++p;
		  ok = 1;
		}
	      else
		/* do not consider a punctuation character not
		   followed by a space to be a boundary */
	      	;
	    }
	  if (ok && nonsp) /* if it counts as a boundary and we also saw non-spaces */
	    {
	      if (nwords)
		*nwords = sp + (ellipsis*2);
	      return p;
	    }
	  *b = 0;
	  if (*p)
	    ++p;
	}
      else if ('(' == *p)
	{
	  const char *closer = find_closer(++p);
	  if (closer)
	    p = closer + 1;
	  else if (*p)
	    ++p;
	}
      else if (ELLIPSIS(p))
	{
	  ++nonsp;
	  ++ellipsis;
	  p += 3;
	}
      else if ('@' == *p)
	p = skip_tag_and_arg(++p);
      else if (CTRL_X == *p)
	{
	  if (p > start && isspace(p[-1]))
	    --sp;
	  if (!strncmp(p+1, "@gap", 4))
	    {
	      /* For @gap we need to guard against a lack of
		 punctuation before the ^X; if there are already words
		 in the line, set an EOL punct, '*' for the current
		 segment */
	      if (sp || ellipsis)
		{
		  *b = '*';
		  *nwords = sp + (ellipsis*2);
		}
	      else
		{
		  *b = '0';
		  *nwords = 0;
		}
	      return p;
	    }
	  else
	    p = skip_XtoY(p);
	}
      else if (*p)
	{
	  ++nonsp;
	  ++p;
	}
    }
  /* by definition end-of-string is a boundary */
  if (words)
    {
      *b = '*';
      if (nwords)
	*nwords = sp + (ellipsis*2);
      return p;
    }
  else
    {
      if (nwords)
	*nwords = 0;
      return NULL;
    }
}

const char *
skip_tag_and_arg(const char *p)
{
  while (isalpha(*p))
    ++p;
  if ('[' == *p)
    {
      while (*p && ']' != *p)
	++p;
      if (*p)
	++p;
    }
  if ('{' == *p)
    ++p;
  /* @-commands are usually followed by whitespace but they don't count as words */
  while (isspace(*p))
    ++p;
  return p;
}

/* Skip from \cX..\cY
 */
const char *
skip_XtoY(const char *p)
{
  while (*p && CTRL_Y != *p)
    ++p;
  return *p ? ++p : p;
}

void
lb_tra_report(FILE *fp, Tra *t)
{
  int i;
  int bad = 0;
  for (i = 0; i < t->npars; ++i)
    if (t->pars[i].choice == C_NONE)
      ++bad;
  if (bad)
    {
      fprintf(fp, "%s: %d/%d para%s failed line break selection\n",
	      t->Q, bad, t->npars, bad>1?"s":"");
      fputc('\t', fp);
      int printed = 0;
      for (i = 0; i < t->npars; ++i)
	{
	  if (t->pars[i].choice == C_NONE)
	    {
	      if (printed++)
		fputs("; ", fp);
	      fputs(t->pars[i].label, fp);
	    }
	}
      fputc('\n', fp);
    }
}
