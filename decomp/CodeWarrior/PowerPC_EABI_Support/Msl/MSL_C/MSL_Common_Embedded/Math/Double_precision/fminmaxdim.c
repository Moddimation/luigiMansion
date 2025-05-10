/* $Id: fminmaxdim.c,v 1.2.4.1 1999/12/06 19:47:53 fassiott Exp $ */

#ifndef _No_Floating_Point

#include "fdlibm.h"
#include <math.h>
double fmax ( double x, double y )
 {
 if(x >= y) return x;
 else if(x < y) return y;
 else
 { /* since x is neither ge or l y they are unordered */
  if(isnan(x)) return y;
  else return x;
  }
 }
double fmin ( double x, double y )
 {
 if(x >= y) return y;
 else if(x < y) return x;
 else
 { /* since x is neither ge or l y they are unordered */
  if(isnan(x)) return y;
  else return x;
  }
 }
double fdim ( double x, double y )
{
 if(x >= y) return (x-y);
 else if(x < y) return 0.0;
 else
 {
 if(isnan(x)) return x;
 else return y;
 }
}
#endif /* _No_Floating_Point */
