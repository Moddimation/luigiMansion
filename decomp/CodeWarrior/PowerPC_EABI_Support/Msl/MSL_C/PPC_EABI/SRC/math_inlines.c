//
// Math_Inlines.c
//

// Added for Nintendo since linking was not working
// when upgrading from an older MSL without the same
// inlines to the newer version where sqrt was inlined.

// When a function is made inline, the previous version
// of the function should be placed here, so that older
// versions of the library will link. Also, the header
// should have a #if !__option(dont_inline) that has
// the inline version of the function. If the user 
// is inlining, then the header version will be used.
// If not, this version will be used.
// swells on 06-26-00

#include <math.h>

double sqrt(double x)
{
  if(x > 0.0f)
 {

   double guess = __frsqrte(x);   // returns an approximation to    
   guess = .5*guess*(3.0 - guess*guess*x);  // now have 8 sig bits
   guess = .5*guess*(3.0 - guess*guess*x);  // now have 16 sig bits
   guess = .5*guess*(3.0 - guess*guess*x);  // now have 32 sig bits
   guess = .5*guess*(3.0 - guess*guess*x);  // now have > 53 sig bits
   return x*guess ;
 }
 else if ( x == 0 )
 	return 0;
 else if ( x )
 	return NAN;

	return INFINITY;
}

