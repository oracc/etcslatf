#include <oraccsys.h>
#include <roco.h>
#include <memo.h>

#include "lb.h"

const char *lbfn;

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

int
main(int argc, char *const *argv)
{
  lbfn = argv[1];
  fprintf(stderr, "lb: processing %s\n", lbfn);
  Roco *r = roco_load1(lbfn);
  int i;
  for (i = 0; i < r->nlines; ++i)
    {
      const char **rr = r->rows[i];
      Seg *segs = map_segs(lb_P(rr));
      log_segs(lb_Q(rr),lb_L(rr),lb_G(rr),segs);
    }
}
