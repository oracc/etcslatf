#ifndef LB_H
#define LB_H

typedef enum choice
  { C_NONE , C_IDENT, C_MULTI, C_IDENT_SENT, C_MULTI_SENT,
    C_FB_SHORT_PAIRS, C_FB_BRUTE,
    C_top
  } Choice;

/* A translation is a series of paras each on one row of the .tsv */
typedef struct tra
{
  const char *Q;
  struct par *pars;
  int npars;
} Tra;

/* A para has a label and an array of segments; it also tracks which
   method succeeded in choosing line breaks, if any */
typedef struct par
{
  struct tra *t;
  Choice choice;
  const char *label;
  const char **labels;
  int lbgoal;
  int xwords;
  const char *text;
  int re_xwords; /* recomputed xwords based on words found in segments */
  struct seg **segs;
  int nsegs;
  int ngaps;
  const char *ss_str;
} Par;

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
  struct par *p;
  const char *o; /* opening of segment -- pointer to first character included in segment */
  const char *c; /* closing of segment -- pointer to last character included in segment */
  const char *label; /* heads of segment chains have non-NULL labels */
  int w;
  int b;		/* punctuation at end of this segment; '0' for segments from @gap */
  int lb;
  struct seg *next; 	/* if a segment goes with subsequent segs this
		           is set; no last is used, we just traverse
		           the list when adding segs */
  struct seg *with;     /* if a segment has been assigned to previous
		           segs this is a pointer to the head */
} Seg;

/* This is how we track the fallback methods we have tried to choose line breaks */
typedef struct ssc
{
  int *ss;
  Choice c;
} SSC;
extern Memo *m_ssc;

/* Macros to access the columns of the lbpp output
#   Q-number
#   Label
#   Goal line count
#   Expected words per line segment
#   Range of line numbers, expanded from label
#   Translation paragraph as a single string * 
 */
#define lb_Q(r) (r)[0]
#define lb_L(r) (r)[1]
#define lb_G(r) (r)[2]
#define lb_W(r) (r)[3]
#define lb_R(r) (r)[4]
#define lb_P(r) (r)[5]

#define CTRL_Q 0x11
#define CTRL_X 0x18
#define CTRL_Y 0x19

#define CLOSER(x)   (')' == *x || '}' == *x)

#define ELLIPSIS(x) (x[0] && x[1] && x[2]		\
		     && ((unsigned char)x[0])==0xE2	\
		     && ((unsigned char)x[1])==0x80	\
		     && ((unsigned char)x[2])==0xA6)

#define EMDASH(x)   ('-' == x[0] && '-' == x[1])

#define QUOTE(x)    (x[0] && x[1] && x[2]		\
		     && ((unsigned char)x[0])==0xE2	\
		     && ((unsigned char)x[1])==0x80	\
		     && ((unsigned char)x[2])==0x9D)

extern FILE *logfp;
extern const char *cnames[];

extern void lb_choose(Par *p);
extern void lb_log_segs(Par *p);
extern void lb_log_segs2(Par *p);
extern void lb_print(FILE *fp, Tra *t);
extern void lb_split_segs(Memo *segmem, Par *p);
extern void lb_tra_report(FILE *fp, Tra *t);
#endif/*LB_H*/


