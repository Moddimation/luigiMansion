#include <math.h>
#define __PI_O2 1.570796327f  
#define __PI  3.1415926535897932384626433832795f  
/*
   Author:  Matthew D. Fassiotto
   Date:    first written 4/15/99
   Purpose: non-optimal single precision version of standard inverse trig functions
   Theory:  atan is a well known algorithm taken straight from "Computer Approximations" 
            by Hart et. al. 
            1978 reprint
            pgs. 125-130.  
                 
*/
/* for _MIPS_ISA4/PPC we use the inverse sqrt instruction-- these are implemented in math_mips.h, for software fpu I've
   implemented _inv_sqrtf  in file sqrtf.c
*/

extern float _inv_sqrtf(float x);
float atan2f(float y, float x)
{
// need to figure out which quadrant we're in first
// note |atanf(y/x)| < pi/2 and will return quadrant 1 if y and x
// have the same sign and quadrant 4 if the signs differ.
const _INT32 _sign_x=__HI(x)&0x80000000;
const _INT32 _sign_y=__HI(y)&0x80000000;
if(_sign_x == _sign_y)
{
  if(_sign_x)  //we know both x, y<0 and angle is in quad 3
	return atanf(y/x) -__PI;
  if(x)
   return atanf(y/x);
  else
   return __PI_O2 ;  
}
// signs differ
if( x < 0.0f)  // (x,y) in quadrant 2
 {
  return __PI + atanf(y/x);
 }
 // (x,y) in quadrant 4
 if(x)
  return atanf(y/x);
 

 __HI(y)=_sign_y+ 0x3FC90FDB;
  return y;
}  
 
 

float acosf(float x)
{
  return __PI_O2 - atan(x*_inv_sqrtf(1.0f- x*x));  
  /* note if |x| > 1 a quiet NaN will be returned via the sqrtf function */
}


float asinf(float x)
{
  return atan(x*_inv_sqrtf(1.0f- x*x)); 
   /* note if |x| > 1 a quiet NaN will be returned via the sqrtf function */
 
}

float atanf(float x)
{  
  float z,z_square;
  int index=-1,inv=0;
  const  int sign=(*(_INT32*)&x)&0x80000000;
  
  /* poly # 4964-- poly for  [0,tan(pi/8)]  */                      
  
  static const float  atan_coeff[]={.999999999f,-.3333333213f,.19999886356f,-.14281650536f,
                                    .11041179874f, -.084597554152f,.04714243524f/*.76f */};     
                     
/* data for argument reduction  */ 
  //static const float  one_over_xi[]={2.414213562f, 1.49660575f,1.00000000f,
    //                                   .668178618f,  .414213568f,.198912367f};
                                     
  static const  float   onep_one_over_xisqr_hi[]={6.82842f,3.239828f,2.0f,    /*   6.828427125f  */
                                                 1.446462f,1.17157292f,1.039566130f};
                                       
  static const  float   onep_one_over_xisqr_lo[]={.000007135f,0.00000082f,.0f,  
                                                 0.00000063f, 0.0f, 0.0f};
                                                                                 
  static const  float   atan_xi_hi[]= {0.0f,.39269f,.5890486f,  .7853981f,  /* .3926990817f  */
                                        .981747f,1.178097f,1.374446f};
  static const  float   atan_xi_lo[]= {0.0f,.000009081698724f,.000000023f,.000000063f,
                                        .000000704f, .00000025f,.00000079f};                                     
 
  static const  float   one_over_xi_hi[]={2.414213f, 1.49660575f,1.00000000f,
                                         .668178618f,.414213568f,.198912367f};
                                       
  static const float    one_over_xi_lo[]={.000000562f, 0.0f,0.0f,
                                          0.0f, 0.0f,0.0f};  
                                                                                                              
  
  (*(_INT32*)&x) &=0x7fffffff;  /*  |x| */
    
  if(x >=  2.414213565f)             /* x is <= 1.0 */ //0x401A827A
  {
   z=1.0f/x;  
   inv++ ;
  } 
  else if(.4142135624f <  x)     //0x3ED413CD  //(4.14213583e-01
  {
    index++;  // index is now 0
    switch(__HI(x)&0x7f800000)
    {  
      case 0x3F000000: /* .5  <= x < 1 */       
      if(__HI(x) >= 0x3F08D5B9) 
        index++;  //0.5345111 == tan(5pi/32)
      if(__HI(x) >= 0x3F521801)
        index++;   //0.8206788
      break;
      case 0x3F800000: /* 1 <= x < 2    */
       index+=2;
      if(__HI(x) >= 0x3F9bf7ec)
        index++;   //1.2185035
      if(__HI(x) >= 0x3FEF789E)
        index++;  //1.8708684
      break;
     case 0x40000000: /* 2 <= x <  2.414213565f   */
      index+=4;
      break;       
   }
   
   z=1.0f/(one_over_xi_hi[index]+(one_over_xi_lo[index]+x));
   z=(one_over_xi_hi[index] -  onep_one_over_xisqr_hi[index]*z) +
     (one_over_xi_lo[index] -  onep_one_over_xisqr_lo[index]*z);
  
 }
 else
   z=x;
 
  z_square=z*z;
  z+= z*z_square*(atan_coeff[1]+ z_square*(atan_coeff[2]+ 
        z_square*(atan_coeff[3]+ z_square*(atan_coeff[4]+  
        z_square*(atan_coeff[5]+ z_square*atan_coeff[6])) )) 
  );  
   z+=atan_xi_lo[index+1];
   z+=atan_xi_hi[index+1];
    
  if(inv) 
  {
    z-=__PI_O2;
    if(sign)   
      return z; 
     
   return -z;
  }
  
    (*(_INT32*)&z)|=sign;
   return z;
    
}