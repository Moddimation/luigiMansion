/*  Metrowerks Standard Library  */

/*  $Date: 2000/10/30 22:24:02 $ 
 *  $Revision: 1.7.4.12.2.2.2.3 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*************************************************************************
*	Project...:	Draft Standard C9X Library								 *
*	Name......:	tgmath.h		        								 *
*   Author..... Matthew D. Fassiotto                                     *
*	Purpose...:	to provide fast single precision overloads of the        *
*               standard C double functions in math.h 					 *			
*************************************************************************/

#ifndef __tgmath__
#define __tgmath__

#include <cmath>

#pragma cplusplus on 
//#pragma k63d_calls on

inline float exp  (float x)
			{
			 return expf(x);
			}	
								
inline float log  (float x)
			{
			 return logf(x);
			}
			
inline float log10(float x)
			{
			 return log10f(x);
			}
			
inline float pow  (float x, float y)
			{
			 return powf(x, y);
			}
			
#if __PPCGEKKO__ /* this #ifdef is a temporary fix to work around a compiler bug
                   in the bottom up inlining routine */
# if __LESS_ACCURATE_FP__       /* results may be 1 bit off in this case */
extern inline float sqrt(float x)
{
static const float _half=.5f;
static const float _three=3.0f;
 if(x > 0.0f)
 {


   float guess = (float)__frsqrte((double)x);   // returns an approximation to   
   guess = _half*guess*(_three - guess*guess*x);  // now have 12 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 24 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits


   return x*guess ;
 }
  return x ;
}   

# else
extern inline float sqrt(float x)
{
static const double _half=.5;
static const double _three=3.0;
volatile float y;
 if(x > 0.0f)
 {


   double guess = __frsqrte((double)x);   // returns an approximation to   
   guess = _half*guess*(_three - guess*guess*x);  // now have 12 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 24 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits
   guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits

   y=(float)(x*guess);
   return y ;
 }
  return x ;
}   

# endif  /* __LESS_ACCURATE_FP__ */

#else						
inline float sqrt (float x)
			{
             return sqrtf(x);     
			}

#endif /* __PPCGEKKO__ */
inline float fabs(float x)
            {
#if __MIPS__
			 return fabsf(x);
#else
             (*(_INT32*)&x)&=0x7fffffff;
             return  x;
#endif
            }

#if !(__MIPS__ && __MIPS_ISA3__ && __MIPS_single_fpu__)
inline float ceil (float x)
			{
			 _INT32 i=(_INT32)x;   
             float y=x-(float)i; 
//           if((!__HI(y)) || (__HI(x)&0x7f800000) >= 0x4B800000) 
             if(!y || x > 8388608.0f)
               return x ;               // x is already an int
             if(x < 0) 
                   return (float)i;
                   
             return (float)++i;
			}		
			
inline float floor(float x)
			{
			 _INT32 i=(_INT32)x;   
             float y=x-(float)i; 
//           if((!__HI(y)) || (__HI(x)&0x7f800000) >= 0x4B800000) 
             if(!y || x > 8388608.0f)
               return x ;               // x is already an int 

             if(x < 0) 
               return (float)--i;  
                   // x < 0 -> int conversion of x above rounded toward zero(so decrement)                
             return (float)i;
			}
#else  // this reserved for fpu's w/special instructions for optimally handling these functions
inline float ceil (register float x)
		{		
			return ceilf(x);	
		}
		
inline float floor(register float x)
		{	
			return floorf(x);	
		}			
#endif

/* 
ldexp:			
when underflow code is enabled this will return results identical to the exteneded precision 
fscale instruction on any x86 fpu 100% of the time. 			
*/

inline float ldexp(float x, _INT32 n)  
			{  			
			 _INT32 new_biased_exp = (_INT32)((0x7f800000&(*(_UINT32*)&x))>>23);
			 // takes care of C9X inf/nan compliance and MUST be first to filter out these cases     
			 switch((*(_INT32*)&x)&0x7f800000)  
              {
               case 0x7f800000:
                return x;         
                // takes care of nan and inf
                break;
               case 0:            
               // may be either 0 or subnormal
#ifdef __No_Gradual_Underflow__
                return 0.0f ;     
                // flush subnormal to zero(as if x was originally 0) and -0 -> 0
                break;
#else                                            
                if(!((*(_INT32*)&x)&0x007fffff)) 
                  return x;    // return only if x==0              
                 do
                  {
                      (*(_UINT32*)&x)= (*(_UINT32*)&x)<<1; //the more we shift the less significance
                      n-- ;
                    }
                   while(!((*(_UINT32*)&x)&0x00800000) ) ;                  
                // no break so we drop down, x is now guaranteed to be normal 
                // The multiplication of x and 2^^n may still produce a denormalized result               
#endif                 
              } // end of switch

           new_biased_exp+=n ; 
           switch(new_biased_exp)
            {
#ifndef __No_Gradual_Underflow__
             case 0:   //barely subnormal(only lowest bit will be lost)
              (*(_UINT32*)&x)=((0x00800000|(*(_UINT32*)&x)&0x007fffff)>>1) + 
              ((*(_UINT32*)&x)&0x80000000);
              return x;
              break;
#endif  
             case 255:  //infinity
              (*(_UINT32*)&x)=( (*(_UINT32*)&x)&0x80000000)|0x7f800000; 
              return x;             
              break;
             default:
              if(!(new_biased_exp&0xffffff00)) // for normal neither the sign bit(underflow) 
               {                               // nor any bit above 7(overflow) should be set
                                               // (255 or 0x000000ff is a full exponent)
                (*(_UINT32*)&x)=((*(_UINT32*)&x)&0x807fffff) + (((_UINT32)new_biased_exp)<<23);        
                return x ;                                                             
               }
               break;
            } 

              if(n > 0)//exponent has overflowed
               {              
                (*(_UINT32*)&x)=( (*(_UINT32*)&x)&0x80000000)|0x7f800000; 
                return x;           
               }
   
#ifdef __No_Gradual_Underflow__
               (*(_UINT32*)&x)=((*(_UINT32*)&x)&0x80000000);
                return x;         //result of x*2^^n is subnormal
#else
               if(new_biased_exp <= -24) 
                {
                 (*(_UINT32*)&x)=((*(_UINT32*)&x)&0x80000000);
                 return x;                                                         
                }                     
                new_biased_exp=(_INT32)(0xffffffff - (_UINT32)new_biased_exp + 1);                        
                (*(_UINT32*)&x)=((0x00800000|(*(_UINT32*)&x)&0x007fffff)>>(new_biased_exp+1)) + 
                ((*(_UINT32*)&x)&0x80000000);
                return x;
#endif                

}
   
inline float frexp(float x, int* exp)
{
  switch( (*(_INT32*)&x)&0x7f800000 )
  {
   case 0x7f800000:
   case 0:
    *exp=0;  // here if zero,inf,or nan
    return x;
    break;
  }
  
  _INT32 tmp_int=(_INT32)(0x3F000000 + ((*(_INT32*)&x)&0x807fffff));
  *exp=(((*(_INT32*)&x)&0x7F800000)>>23)-126;
  return *((float*)&tmp_int);
}
							
inline float tan  (float x)
			{
			 return tanf(x);
			}
			
inline float acos (float x)
			{
			 return acosf(x);
			}
			
inline float asin (float x)
			{
			 return asinf(x);
			}	
								
inline float atan2(float x, float y)
			{
			 return atan2f(x, y);
			}
			
inline float atan (float x)
			{
			 return atanf(x);
			}
											
inline float sin  (float x)
			{
			 return sinf(x);
			}
								
inline float cos  (float x)
			{
			 return cosf(x);
			}
						
inline float cosh (float x)
			{
			 return coshf(x);
			}
			
inline float sinh (float x)
			{
			 return sinhf(x);
			}
			
inline float tanh (float x)
			{
			 return tanhf(x);
			}

inline float fmod (float x, float y)
	{
	  return fmodf(x,y);
	}
				
/*
inline float fmod (float x, float y)
	{
	  
      _UINT32 exp_remainder;  
      float tmp;  
              
     if(((*(_INT32*)&x)&0x7fffffff) < ((*(_INT32*)&y)&0x7fffffff))
       return x;
       
     exp_remainder=((*(_INT32*)&y)&0x7f800000)- 0x3f000000;
     
    (*(_INT32*)&y)=((*(_INT32*)&y)&0x007fffff)+ 0x3f000000;
    (*(_INT32*)&x)=((*(_INT32*)&x)&0x007fffff)+ 0x3f000000;
    tmp=x-y;
    
    if(*((_UINT32*)&tmp)&0x80000000)
     {
       (*(_INT32*)&x)|=0x00800000;  //multiply by 2
       tmp=x-y;
     }
    
    (*(_UINT32*)&tmp)+=exp_remainder;
    return tmp;
}
*/

// long double argument overloads, here we assume sizeof(double)=sizeof(long double)
// as is currently the case except on 68K macos.
		inline long double acos(long double x)
			{return acosl(x);}
		inline long double asin(long double x)
			{return asinl(x);}
		inline long double atan(long double x)
			{return atanl(x);}
		inline long double atan2(long double y, long double x)
			{return atan2l(y, x);}
		inline long double cos(long double x)
			{return cosl(x);}
		inline long double sin(long double x)
			{return sinl(x);}
		inline long double tan(long double x)
			{return tanl(x);}
		inline long double cosh(long double x)
			{return coshl(x);}
		inline long double sinh(long double x)
			{return sinhl(x);}
		inline long double tanh(long double x)
			{return tanhl(x);}
        inline long double fmod(long double x, long double y)
			{return fmodl(x, y);}			
        inline long double ceil(long double x)
			{return ceill(x);}
        inline long double floor(long double x)
			{return floorl(x);}
        inline long double exp(long double x)
			{return expl(x);}
		inline long double frexp(long double x, int* exp)
			{return frexpl(x, exp);}
		inline long double ldexp(long double x, int exp)
			{return ldexpl(x, exp);}
		inline long double log(long double x)
			{return logl(x);}
		inline long double log10(long double x)
			{return log10l(x);}
		inline long double fabs(long double x)
			{return fabsl(x);}
		inline long double pow(long double x, long double y)
			{return powl(x, y);}
		inline long double sqrt(long double x)
			{return sqrtl(x);}			


// these should take care of any combination of any other arithmetic type
template <class T, class U>
inline
double
	fmod(const T& x, const U& y)
	{
		return fmod((double)x, (double)y);
	}

template <class T>
inline
double
ldexp(const T& x, int exp)
	{
		return ldexp((double)x, exp);
	}				
	
template <class T>
inline
double
	ceil(const T& x)
	{
		return (double)x;  // intentional optimization
	}

template <class T>
inline
double
	floor(const T& x)
	{
		return (double)x;  // intentional optimization
	}
	
template <class T>
inline
double
	acos(const T& x)
	{
		return acos((double)x);
	}

template <class T>
inline
double
	asin(const T& x)
	{
		return asin((double)x);
	}

template <class T>
inline
double
	atan(const T& x)
	{
		return atan((double)x);
	}

template <class T, class U>
inline
double
	atan2(const T& y, const U& x)
	{
		return atan2((double)y, (double)x);
	}

template <class T>
inline
double
	cos(const T& x)
	{
		return cos((double)x);
	}

template <class T>
inline
double
	sin(const T& x)
	{
		return sin((double)x);
	}

template <class T>
inline
double
	tan(const T& x)
	{
		return tan((double)x);
	}

template <class T>
inline
double
	cosh(const T& x)
	{
		return cosh((double)x);
	}

template <class T>
inline
double
	sinh(const T& x)
	{
		return sinh((double)x);
	}

template <class T>
inline
double
	tanh(const T& x)
	{
		return tanh((double)x);
	}
	
	
template <class T>
inline
double
	exp(const T& x)
	{
		return exp((double)x);
	}

template <class T>
inline
double
	frexp(const T& x, int* exp)
	{
		return frexp((double)x, exp);
	}


template <class T>
inline
double
	log(const T& x)
	{
		return log((double)x);
	}

template <class T>
inline
double
	log10(const T& x)
	{
		return log10((double)x);
	}
	
template <class T>
inline
double
	fabs(const T& x)
	{
		return fabs((double)x);
	}

template <class T, class U>
inline
double
	pow(const T& x, const U& y)
	{
		return pow((double)x, (double)y);
	}

template <class T>
inline
double
	sqrt(const T& x)
	{
		return sqrt((double)x);
	}
//#pragma k63d_calls reset					
#pragma cplusplus reset
#pragma warn_unusedarg reset
#pragma warn_unusedvar reset


#endif /* __tgmath__ */
/* 012799  mf-- including cmath instead of math.h per C++ standard,
                also added header to top of file
   021799  mf-- added faster version of ldexp, also added a complete
                set of int overloads for two argument functions
   031099  mf-- tgmath.h is a general C++ header-- let cmath turn on k6 calling convention 
   051999  mf-- fixed implicit type conversion warnings.               
*/
