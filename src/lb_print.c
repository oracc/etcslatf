#include <oraccsys.h>
#include "lb.h"

void
lbp_print_seg(FILE *fp, Seg *s)
{
  if (s->o)
    {
      const char *o = s->o;
      while (isspace(*o))
	++o;
      if ('0' == s->b)
	{
	  int len = s->c - s->o;
	  char buf[len+1];
	  strncpy(buf, s->o, len);
	  buf[len] = '\0';
	  fprintf(fp, "($%s$)",buf);
	}
      else
	{
	  while (o < s->c)
	    {
	      if (CTRL_Q == *o)
		fputs(" (?)", fp);
	      else
		fputc(*o, fp);
	      ++o;
	    }
	}
    }
  fputc(' ', fp);
}

void
lbp_seg(FILE *fp, Seg *s)
{
  if (s->with)
    return;
  
  if (s->label)
    fprintf(fp, "%s.\t", s->label);
  lbp_print_seg(fp, s);
  while ((s = s->next))
    lbp_print_seg(fp, s);
  fputc('\n', fp);
}

void
lbp_par(FILE *fp, Par *p)
{
  int i;
  fprintf(fp, "\n{%s}\n", p->label);
  if (!p->nsegs)
    {
      const char *group_label = p->label;
      const char *eq = strchr(group_label, '=');
      if (eq)
	group_label = eq + 2;
      fprintf(fp, "%s.\t%s\n", group_label, p->text);
    }
  else
    {
      for (i = 0; i < p->nsegs; ++i)
	lbp_seg(fp, p->segs[i]);
    }
}

void
lb_print(FILE *fp, Tra *t)
{
  fprintf(fp, "&%s\n", t->Q);
  int i;
  for (i = 0; i < t->npars; ++i)
    lbp_par(fp, &t->pars[i]);
}
