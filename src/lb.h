#ifndef LB_H
#define LB_H

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

#endif/*LB_H*/


