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
const char *lbfn;
char *logfn;
FILE *logfp;

/* Segments are spans of text between punctuation: they may include
 * text marked with \cX...\cY.  We identify all the possible spans,
 * then count the words in each span, ignoring the words in un-nested
 * parentheses and in \cX...\cY. Then we calculate which spans
 * complete a line by looking for collections of spans that have a
 * word-count just greater than or close to the expected word count
 * for the paragraph's line breaks.  Line break ending spans are
 * flagged in the segment mapping structure in the .lb member.
 */
typedef struct seg
{
  const char *o; /* opening of segment -- pointer to first character included in segment */
  const char *c; /* closing of segment -- pointer to last character included in segment */
  int w;
  int lb;
} Seg;

const char *find_closer(const char *p);
void log_segs(const char *Q, const char *L, const char *G, const char *W, Seg**segs);
Seg **map_segs(const char *p);
const char *next_boundary(const char *p, int *nwords);
const char *skip_XtoY(const char *p);

int
main(int argc, char *const *argv)
{
  lbfn = argv[1];
  fprintf(stderr, "lb: processing %s\n", lbfn);
  Roco *r = roco_load1(lbfn);
  logfn = strdup(lbfn);
  strcpy(&logfn[strlen(logfn)-3], "log");
  logfp = fopen(logfn, "w");
  int i;
  for (i = 0; i < r->nlines; ++i)
    {
      const char **rr = (const char**)r->rows[i];
      Seg **segs = map_segs(lb_P(rr));
      log_segs(lb_Q(rr),lb_L(rr),lb_G(rr),lb_W(rr),segs);
    }
  fclose(logfp);
}

/* This could be modified to handled nested (...(...)...); not clear
   it's necessary */
const char *
find_closer(const char *p)
{
  while (*p)
    if ('(')
      return NULL;
    else if (')' == *p)
      return p;
    else
      ++p;
  return NULL;
}

void
log_segs(const char *Q, const char *L, const char *G, const char *W, Seg **segs)
{
  fprintf(logfp, "&%s\t%s\t%s\t%s\n", Q, L, G, W);
  int i;
  for (i = 0; segs[i]; ++i)
    {
      fprintf(logfp, ">%c\t%d\t", segs[i]->lb ? '+' : '-', segs[i]->w);
      fwrite(segs[i]->o, sizeof(char), segs[i]->c - segs[i]->o, logfp);
      fputc('\n', logfp);
    }
  fputs("======================================================\n", logfp);
}

Seg **
map_segs(const char *p)
{
  Memo *segmem = memo_init(sizeof(Seg), 32);
  List *l = list_create(LIST_SINGLE);
  while (*p)
    {
      Seg *s = memo_new(segmem);
      list_add(l, s);
      if (CTRL_X == *p)
	p = skip_XtoY(p);
      if (*p)
	{
	  s->o = p;
	  p = next_boundary(p, &s->w);
	  if (p)
	    {
	      s->c = p;
	      if (*p)
		++p;
	    }
	  else
	    {
	      s->c = s->o + strlen(s->o);
	      break;
	    }
	}
    }
  Seg **sp = (Seg **)list2array(l);
  list_free(l, NULL);
  return sp;
}

const char *
next_boundary(const char *p, int *nwords)
{
  int ellipsis = 0, nonsp = 0, sp = 1;
  while (isspace(*p))
    ++p;
  const char *start = p; /* start of word count */
  while (*p)
    {
      if (isspace(*p))
	{
	  nonsp = 0;
	  ++sp;
	  do
	    ++p;
	  while (isspace(*p));
	}
      else if (bounds[(unsigned char)*p])
	{
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
		*nwords = sp + ellipsis;
	      return p;
	    }
	  if (*p)
	    ++p;
	}
      else if ('(' == *p)
	{
	  const char *closer = find_closer(p);
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
      else if (CTRL_X == *p)
	p = skip_XtoY(p);
      else if (*p)
	{
	  ++nonsp;
	  ++p;
	}
    }
  /* by definition end-of-string is a boundary */
  if (nonsp)
    return p;
  else
    return NULL;
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
