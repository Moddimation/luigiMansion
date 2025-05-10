#include <cmath>
#if ( __POWERPC__ && defined(_No_Floating_Point_Regs) )
extern float fabsf(float x)
{
 __HI(x)&=0x7fffffff;
 return x;
}
#endif
extern float fmodf( float x, float y)
{
                                         // some unique integer Q  such that R < y
                                         // we do this indirectly by computing Q*y
                                         // and return x-Q*y
     _UINT32 exp_shift;                  // shift only enough to fit in eax::edx
     _UINT32 exp_diff,quotient=0,scaled_x,scaled_y;  //quotient=quotient_hi+quotient_lo
     float z=fabsf(x), y_hi,y_lo, f_quotient[3];  
#if !__INTEL__      
     unsigned long long  ltmp;
#endif
     y=fabsf(y);
     if(z < y)
       return x;
    
       
     exp_diff=((__HI(x)&0x7f800000)-(__HI(y)&0x7f800000))>>23;
     exp_shift=exp_diff&0x0000001f ;  // shift at most 31 to insure quotient is < 2^^32 -1 
                                      // i.e. quotient will fit in 32 bit register.
     scaled_x=(__HI(x)&0x007fffff) + 0x00800000;
     scaled_y=(__HI(y)&0x007fffff) + 0x00800000;
     exp_diff-=exp_shift;
     exp_diff<<=23;
#if __INTEL__
 asm
   {
    mov eax,0
    mov edx,scaled_x
    mov ecx,scaled_y
    shl ecx,1                        // multiply divisor by two, to avoid overflow(i.e. to insure quotient is < 2^^32 -1 )
    div ecx
    mov quotient, eax
   }
   
#else
   ltmp=(unsigned long long )scaled_x;
   ltmp<<=32;
   quotient=(_UINT32)(ltmp/(unsigned long long)(scaled_y<<1));
#endif
   quotient >>= (31 - exp_shift);    // truncates instead of round to nearest(this is correct since we want 
                                     // largest integer such that (x-Q*y < y).  Round to nearest may produce one too large.
                                     // multiply quotient term by 2^^exp_diff , if exp_diff > 32 this product will be rounded and
                                     // will not be affected by the least significant bits of the original y.
 
  __HI(z)-=exp_diff; 
  __HI(y_hi)=__HI(y)&0x7ffff000;     // the division of y into two floats(y_lo,y_hi) w/ at most 12 significant bits
                                     // and quotient in 3 parts each w/12 or fewer sig bits guarantees
                                     // that all intermediate products are exact.  This also guarantees
                                     // that the final subtraction operation when ordered correctly is
                                     // exact.  Also note that we use a full 32 bit quotient.  This gives
                                     // an extra 8 bits of precision for larger quotients which cannot be
                                     // represented exactly as an IEEE float(which has only 24 significant bits).
                                     // still this quotient is inadequate for dividend/divisor pairs whose exponents differ 
                                     // by more than 32 
                                   
  y_lo=y-y_hi; 
  
  
  f_quotient[0]=(float)(quotient&0x00000fff);
 

 /*if(quotient&0xfffff000)
 {

 // this is what we have to do to take advantage of the 32 bit quotient.*/
  f_quotient[1]=(float)(quotient&0x00fff000);
  f_quotient[2]=(float)(quotient&0xff000000);


  z= fabsf((float)((((y_hi*f_quotient[2] -z) + y_hi*f_quotient[1]) + y_hi*f_quotient[0]) +
                     (y_lo*f_quotient[2]    +( y_lo*f_quotient[1]  + y_lo*f_quotient[0])))); 
  
 /* }
  else
   z=fabsf( (y_hi*f_quotient[0] -z) + y_lo*f_quotient[0]);
   */
  __HI(z)|=signbit(x);
  return z;                     
  
}	