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

#endif/*LB_H*/
