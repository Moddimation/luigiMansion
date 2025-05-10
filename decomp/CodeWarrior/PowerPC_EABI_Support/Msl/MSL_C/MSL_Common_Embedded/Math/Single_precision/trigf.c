// fast sin and cos

#include <math.h>

#define __PIO2__ 1.57079632679489661923132169163975f
#define __PI  3.1415926535897932384626433832795f
#define __two_over_pi .636619772367581343075535053490057f
#define __four_over_pi 1.27323954473516268615107010698011f

#define __SQRT_FLT_EPSILON__ 3.4526698300e-04f
#define __sqrt2_m1  .41421356237f
#define __ln2  0.6931471805599f

#pragma cplusplus on
extern "C" {
extern const float __sincos_poly[];  
//sin(n*pi/4),cos(n*pi/4),sin(n*pi/4)
                                    
extern const float __sincos_on_quadrant[];
}
const int tmp_float[4]={0x3E800000,0x3Cbe6080,0x34372200,0x2da44152};

static const float  __four_over_pi_m1[]={*(float*)&tmp_float[0],*(float*)&tmp_float[1],
                                         *(float*)&tmp_float[2],*(float*)&tmp_float[3]}; 
                                         
extern "C" float sinf(float x)
{           
                                
  float z=__two_over_pi*x ; 
//note: |z-n|<=.5 i.e. we choose n so that |n*pi/2 - x|< .5*(pi/2)=pi/4, then use the poly for [0,pi/4], but
//      also notice that n(mod(4)) gives which quadrant we're in and enables us to use the trig addition identities.
//      Exactly one of these terms drops out because either sin(n*pi/2) or cos(n*pi/2) is 0.
#define __FAST_MATH_MODE__ 0
 
#if __FAST_MATH_MODE__
  _INT32 n= (_INT32)z; //note: truncation insures |n|<z and cos/sin(x)=cos/sin(n*pi/4+frac_part)
  const float frac_part= z -(float)(n<<1) ;
#else
  
                                    

//note we multiply n by 2 below because the polynomial we are using is for [o,pi/4]. n is the nearest multiple
// of pi/2 not pi/4.
//  frac_part is the remainder(mod(pi/4))
// i.e. the actual arg that will be evaluated is frac_part*(pi/4)
// note: since n is signed n<<1 may pad rightmost bit w/a one.
 _INT32 n= (__HI(x)&0x80000000) ? (_INT32)(z -.5f) : (_INT32)(z + .5f) ;  

 const float frac_part= ((((x - (float)(n*2)) + __four_over_pi_m1[0]*x) + __four_over_pi_m1[1]*x)+
	                                            __four_over_pi_m1[2]*x) + __four_over_pi_m1[3]*x/*) +
  	                                               __four_over_pi_m1[4]*x  */; 
#endif  

    float xsq; 
   
   //assumes 2's complement integer storage for negative numbers.
    n&=0x00000003;

   if(fabsf(frac_part) < __SQRT_FLT_EPSILON__)
   {
    n<<=1;   //index into __sincos_on_quadrant array
    return __sincos_on_quadrant[n] + (__sincos_on_quadrant[n+1]*frac_part*__sincos_poly[9]);
   }
   
   xsq=frac_part*frac_part;
   if(n&0x00000001)  // we are at a multiple of pi/2 thus cos(n*pi/4)= 0
   {

    n<<=1;   //index into __sincos_on_quadrant array
    z=((( __sincos_poly[0]*xsq +__sincos_poly[2])*xsq + __sincos_poly[4])*xsq + __sincos_poly[6])*xsq + 
    __sincos_poly[8];

    return z*__sincos_on_quadrant[n];// sin(frac_part)*cos(n*pi/4);  note: n*pi/4 is a multiple of pi/2(not pi)
    //return z;// sin(frac_part)*cos(n*pi/4);  note: n*pi/4 is a multiple of pi/2(not pi)
   }
    
// if here we are near a multiple of pi so sin(n*pi/4) =0
    n<<=1;   //index into __sincos_on_quadrant array

    z= ((((__sincos_poly[1]*xsq +__sincos_poly[3])*xsq + __sincos_poly[5])*xsq + __sincos_poly[7])*xsq + __sincos_poly[9])*frac_part;
    //return z;// sin(frac_part)*cos(n*pi/4);  note: n*pi/4 is a multiple of pi/2(not pi)
   
    return z*__sincos_on_quadrant[n+1];// sin(frac_part)*cos(n*pi/4);  note: n*pi/4 is a multiple of pi/2(not pi)
    

}


float cosf(float x)
{

    float z=__two_over_pi*x ; 
    #define __FAST_MATH_MODE__ 0
 
#if __FAST_MATH_MODE__
  _INT32 n= (_INT32)z; //note: truncation insures |n|<z and cos/sin(x)=cos/sin(n*pi/4+frac_part)
  const float frac_part= z -(float)(n<<1) ;
#else
  
                                    

//note we multiply n by 2 below because the polynomial we are using is for [o,pi/4]. n is the nearest multiple
// of pi/2 not pi/4.
//  frac_part is the remainder(mod(pi/4))
// i.e. the actual arg that will be evaluated is frac_part*(pi/4)
// note: since n is signed n<<1 may pad rightmost bit w/a one.
 _INT32 n= (__HI(x)&0x80000000) ? (_INT32)(z -.5f) : (_INT32)(z + .5f) ;  

 const float frac_part= ((((x - (float)(n*2)) + __four_over_pi_m1[0]*x) + __four_over_pi_m1[1]*x)+
  	                                              __four_over_pi_m1[2]*x) + __four_over_pi_m1[3]*x/*) +
  	                                               __four_over_pi_m1[4]*x  */; 
#endif  

	float xsq;
    n&=0x00000003;

   if(fabsf(frac_part) < __SQRT_FLT_EPSILON__)
   {
     n<<=1;   //index into __sincos_on_quadrant array
     return __sincos_on_quadrant[n+1] - (__sincos_on_quadrant[n]*frac_part);
   }
    
  
   //use identity cos(x)=cos(n*pi/4 + frac_part)=cos(n*pi/4)cos(frac_part)- sin(n*pi/4)sin(frac_part)
   xsq=frac_part*frac_part;
   if(n&0x00000001)  // we are at a multiple of pi/2 thus cos(n*pi/4)= 0
   {   
     n<<=1;   //index into __sincos_on_quadrant array 
     z= -((((__sincos_poly[1]*xsq +__sincos_poly[3])*xsq + __sincos_poly[5])*xsq + __sincos_poly[7])*xsq + __sincos_poly[9])*frac_part;
     return z*__sincos_on_quadrant[n];
   }

    n<<=1;   //index into __sincos_on_quadrant array
    // if here we are near a multiple of pi so sin(n*pi/4) =0
    
     z=((( __sincos_poly[0]*xsq +__sincos_poly[2])*xsq + __sincos_poly[4])*xsq + __sincos_poly[6])*xsq + __sincos_poly[8];
     return z*__sincos_on_quadrant[n+1];// sin(frac_part)*cos(n*pi/4);  note: n*pi/4 is a multiple of pi/2(not pi)
}

float tanf(float x)
 {
  
   return sin(x)/cos(x); 
  
 }


#pragma cplusplus reset