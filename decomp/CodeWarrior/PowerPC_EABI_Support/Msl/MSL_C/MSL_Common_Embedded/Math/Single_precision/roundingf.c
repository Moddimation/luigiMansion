#if !( __MIPS__ && __MIPS_ISA3__  && __MIPS_single_fpu__)

#include <math.h>
/*
 Author:  Matthew D. Fassiotto
 Date:    first written 4/15/99
 Purpose: non-optimal single precision version of standard ceil and floor functions
 Assumptions: --IEEE 754 single precision float format
     *fp difference should never produce -0
     *casting a float to an int always truncates regardless of fp rounding mode.
     *the type _INT32 is 32 bits(i.e. sizeof(_INT32)=sizeof(float)
*/

 /* ceilf--  returns smallest integer(as a float) not less than x(round x -> infinity)
    floorf-- returns largest integer(as a float) not greater than x(round x -> -infinity)
 */ 
extern float ceilf (float x)
			{
			 _INT32 i=x;   
             float y=x-(float)i; 
             if((!__HI(y)) || (__HI(x)&0x7f800000) >= 0x4B800000) 
               return x ;               // x is already an int
             else if(__HI(x)&0x80000000)
                   return (float)i;                   
             return (float)++i;
			}		
			
extern float floorf(float x)
			{
			 _INT32 i=x;               // signed int 
             float y=(float)i-x;       // since order of evaluation is NOT guaranteed
                                       // this is not guaranteed to work with all compilers for -0
                                       // I currently have no "cheap" work around to this.                          
                                       
             if((!__HI(y)) || (__HI(x)&0x7f800000) >= 0x4B800000) 
              return x ;               // x is already an int
             else if(__HI(x)&0x80000000) 
                   return (float)--i;  
                   // x < 0 -> int conversion of x above rounded toward zero(so decrement)                
             return (float)i;
			}

#endif /* !( __MIPS__ && __MIPS_ISA3__  && __MIPS_single_fpu__) */
