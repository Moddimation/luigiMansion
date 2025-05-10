#ifndef __math_ppc__
#define __math_ppc__

#pragma cplusplus on
#ifndef _No_Floating_Point_Regs
#include <cmath>

#if __LESS_ACCURATE_FP__
extern inline float sqrtf(float x)
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

#else
extern inline float sqrtf(float x)
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
   y=(float)(x*guess);
   return y ;
 }
  return x ;
}   

#endif

  
 

extern inline float _inv_sqrtf(float x)
{
 
 if(x > 0.0f)
 {


   float guess = (float)__frsqrte((double)x);   // returns an approximation to   
   guess = .5f*guess*(3.0f - guess*guess*x);  // now have 8  sig bits
   guess = .5f*guess*(3.0f - guess*guess*x);  // now have 16 sig bits
   guess = .5f*guess*(3.0f - guess*guess*x);  // now have >24 sig bits
   return guess ;
 }
 else if(x)
    return NAN ;
  
 return INFINITY ; 
 }
#ifndef __USING_FDLIBM__


// added for backwards compatibility with Nintendo libraries.
// Since sqrt was made inline, older versions of NTD libs built
// with a reference to sqrt as a symbol would not be able to link.
// swells on 06-26-00
#if !__option(dont_inline)
extern inline double sqrt(double x)
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
#endif

extern inline double fabs(double x)
{
 
   return __fabs(x) ;
}

#endif /* __USING_FDLIBM__ */
extern inline float fabsf(float x)
{
 
   return __fabsf(x) ;
}   

extern inline float fmodf(register float x, register float y)
{
//note this works only when the exponents of x and y differ by less
//than 64. Will add arrays of ints to handle general case.

 long long quotient;  //we're looking at x/y
 if(fabsf(y) > fabsf(x))
  return x;
  
  quotient=(long long)(x/y);
  return x-(y*(float)quotient) ;
 }
 


#else
static const _UINT32 inv_sqrt_guess[]={
0x3FFF017E,
0x3FFE05EC,0x3FFD0D3E,0x3FFC1764,0x3FFB2452,0x3FFA33F9,
0x3FF9464E,0x3FF85B42,0x3FF772CB,0x3FF68CDC,0x3FF5A969,
0x3FF4C867,0x3FF3E9CB,0x3FF30D8A,0x3FF23399,0x3FF15BEF,
0x3FF08681,0x3FEFB345,0x3FEEE232,0x3FEE133E,0x3FED4660,
0x3FEC7B90,0x3FEBB2C5,0x3FEAEBF5,0x3FEA271A,0x3FE9642A,
0x3FE8A31D,0x3FE7E3ED,0x3FE72691,0x3FE66B02,0x3FE5B139,
0x3FE4F92E,0x3FE442DB,0x3FE38E39,0x3FE2DB41,0x3FE229ED,
0x3FE17A36,0x3FE0CC16,0x3FE01F87,0x3FDF7483,0x3FDECB04,
0x3FDE2305,0x3FDD7C7F,0x3FDCD76E,0x3FDC33CC,0x3FDB9193,
0x3FDAF0BF,0x3FDA514A,0x3FD9B330,0x3FD9166B,0x3FD87AF7,
0x3FD7E0CF,0x3FD747EE,0x3FD6B051,0x3FD619F2,0x3FD584CD,
0x3FD4F0DF,0x3FD45E22,0x3FD3CC93,0x3FD33C2E,0x3FD2ACEE,
0x3FD21ED0,0x3FD191D1,0x3FD105EC,0x3FD07B1D,0x3FCFF162,
0x3FCF68B6,0x3FCEE116,0x3FCE5A7F,0x3FCDD4ED,0x3FCD505E,
0x3FCCCCCD,0x3FCC4A38,0x3FCBC89B,0x3FCB47F4,0x3FCAC83F,
0x3FCA497A,0x3FC9CBA2,0x3FC94EB3,0x3FC8D2AB,0x3FC85787,
0x3FC7DD45,0x3FC763E2,0x3FC6EB5A,0x3FC673AC,0x3FC5FCD6,
0x3FC586D3,0x3FC511A3,0x3FC49D42,0x3FC429AF,0x3FC3B6E6,
0x3FC344E6,0x3FC2D3AD,0x3FC26337,0x3FC1F383,0x3FC1848F,
0x3FC11659,0x3FC0A8DE,0x3FC03C1C,0x3FBFD012,0x3FBF64BD,
0x3FBEFA1C,0x3FBE902C,0x3FBE26EB,0x3FBDBE58,0x3FBD5671,
0x3FBCEF34,0x3FBC889F,0x3FBC22B1,0x3FBBBD67,0x3FBB58C0,
0x3FBAF4BA,0x3FBA9154,0x3FBA2E8C,0x3FB9CC60,0x3FB96ACE,
0x3FB909D6,0x3FB8A975,0x3FB849AA,0x3FB7EA74,0x3FB78BD0,
0x3FB72DBF,0x3FB6D03D,0x3FB6734A,0x3FB616E4,0x3FB5BB09,
0x3FB55FBA,0x3FB504F3,0x3FB4AAB4};

static const _UINT32 inv_sqrt_guess2[]={
0x3FB450FC,
0x3FB39F19,0x3FB2EF41,0x3FB2416A,0x3FB19589,0x3FB0EB96,
0x3FB04387,0x3FAF9D53,0x3FAEF8F2,0x3FAE565C,0x3FADB587,
0x3FAD166C,0x3FAC7904,0x3FABDD46,0x3FAB432A,0x3FAAAAAB,
0x3FAA13C0,0x3FA97E62,0x3FA8EA8C,0x3FA85835,0x3FA7C759,
0x3FA737F0,0x3FA6A9F4,0x3FA61D5F,0x3FA5922C,0x3FA50855,
0x3FA47FD3,0x3FA3F8A2,0x3FA372BD,0x3FA2EE1D,0x3FA26ABE,
0x3FA1E89B,0x3FA167AF,0x3FA0E7F5,0x3FA06968,0x3F9FEC04,
0x3F9F6FC4,0x3F9EF4A4,0x3F9E7AA0,0x3F9E01B3,0x3F9D89D9,
0x3F9D130E,0x3F9C9D4E,0x3F9C2896,0x3F9BB4E1,0x3F9B422C,
0x3F9AD073,0x3F9A5FB2,0x3F99EFE6,0x3F99810C,0x3F991320,
0x3F98A61F,0x3F983A05,0x3F97CED0,0x3F97647C,0x3F96FB06,
0x3F96926C,0x3F962AA9,0x3F95C3BC,0x3F955DA2,0x3F94F857,
0x3F9493D9,0x3F943026,0x3F93CD3A,0x3F936B13,0x3F9309AF,
0x3F92A90B,0x3F924925,0x3F91E9F9,0x3F918B87,0x3F912DCA,
0x3F90D0C3,0x3F90746D,0x3F9018C6,0x3F8FBDCE,0x3F8F6381,
0x3F8F09DD,0x3F8EB0E0,0x3F8E5889,0x3F8E00D5,0x3F8DA9C2,
0x3F8D534F,0x3F8CFD79,0x3F8CA83F,0x3F8C539F,0x3F8BFF97,
0x3F8BAC25,0x3F8B5948,0x3F8B06FD,0x3F8AB544,0x3F8A641A,
0x3F8A137D,0x3F89C36D,0x3F8973E8,0x3F8924EC,0x3F88D677,
0x3F888889,0x3F883B1E,0x3F87EE37,0x3F87A1D2,0x3F8755ED,
0x3F870A87,0x3F86BF9E,0x3F867532,0x3F862B40,0x3F85E1C7,
0x3F8598C7,0x3F85503E,0x3F85082A,0x3F84C08B,0x3F84795F,
0x3F8432A5,0x3F83EC5C,0x3F83A682,0x3F836117,0x3F831C1A,
0x3F82D788,0x3F829362,0x3F824FA6,0x3F820C52,0x3F81C967,
0x3F8186E2,0x3F8144C4,0x3F81030A,0x3F80C1B4,0x3F8080C1,
0x3F804030,0x3F800000,0x3F7F8060};


static const _UINT32 __two_inv_x[]={
0x407E03F8,
0x407C0FC1,0x407A232D,0x40783E10,0x4076603E,0x4074898D,0x4072B9D6,0x4070F0F1,0x406F2EB7,
0x406D7304,0x406BBDB3,0x406A0EA1,0x406865AC,0x4066C2B4,0x40652598,0x40638E39,0x4061FC78,
0x40607038,0x405EE95C,0x405D67C9,0x405BEB62,0x405A740E,0x405901B2,0x40579436,0x40562B81,
0x4054C77B,0x4053680D,0x40520D21,0x4050B6A0,0x404F6475,0x404E168A,0x404CCCCD,0x404B8728,
0x404A4588,0x404907DA,0x4047CE0C,0x4046980C,0x404565C8,0x40443730,0x40430C31,0x4041E4BC,
0x4040C0C1,0x403FA030,0x403E82FA,0x403D6910,0x403C5264,0x403B3EE7,0x403A2E8C,0x40392144,
0x40381703,0x40370FBB,0x40360B61,0x403509E7,0x40340B41,0x40330F63,0x40321643,0x40311FD4,
0x40302C0B,0x402F3ADE,0x402E4C41,0x402D602B,0x402C7692,0x402B8F6A,0x402AAAAB,0x4029C84A,
0x4028E83F,0x40280A81,0x40272F05,0x402655C4,0x40257EB5,0x4024A9CF,0x4023D70A,0x4023065E,
0x402237C3,0x40216B31,0x4020A0A1,0x401FD80A,0x401F1166,0x401E4CAD,0x401D89D9,0x401CC8E1,
0x401C09C1,0x401B4C70,0x401A90E8,0x4019D723,0x40191F1A,0x401868C8,0x4017B426,0x4017012E,
0x40164FDA,0x4015A025,0x4014F209,0x40144581,0x40139A86,0x4012F114,0x40124925,0x4011A2B4,
0x4010FDBC,0x40105A38,0x400FB824,0x400F177A,0x400E7835,0x400DDA52,0x400D3DCB,0x400CA29C,
0x400C08C1,0x400B7034,0x400AD8F3,0x400A42F8,0x4009AE41,0x40091AC7,0x40088889,0x4007F781,
0x400767AB,0x4006D905,0x40064B8A,0x4005BF37,0x40053408,0x4004A9FA,0x40042108,0x40039930,
0x4003126F,0x40028CC0,0x40020821,0x4001848E,0x40010204,0x40008081,0x40000000,0x3FFF00FF,
};
static const _UINT32 __inv_x_sqr[]={
0x407C0BE0,
0x40782F05,0x407468B9,0x4070B84D,0x406D1D1B,0x4069967F,0x406623E0,0x4062C4A7,0x405F7842,
0x405C3E29,0x405915D3,0x4055FEBF,0x4052F871,0x40500270,0x404D1C48,0x404A4588,0x40477DC4,
0x4044C493,0x40421991,0x403F7C5B,0x403CEC92,0x403A69DC,0x4037F3E0,0x40358A48,0x40332CC3,
0x4030DAFF,0x402E94B0,0x402C598B,0x402A2946,0x4028039C,0x4025E849,0x4023D70A,0x4021CFA0,
0x401FD1CD,0x401DDD55,0x401BF1FD,0x401A0F8E,0x401835CF,0x4016648D,0x40149B93,0x4012DAB0,
0x401121B2,0x400F706C,0x400DC6AE,0x400C244D,0x400A891D,0x4008F4F5,0x400767AB,0x4005E119,
0x40046116,0x4002E77F,0x4001742E,0x40000700,0x3FFD3FA7,0x3FFA7D0C,0x3FF7C5EE,0x3FF51A0D,
0x3FF2792D,0x3FEFE311,0x3FED577F,0x3FEAD63D,0x3FE85F15,0x3FE5F1D0,0x3FE38E39,0x3FE1341E,
0x3FDEE34C,0x3FDC9B93,0x3FDA5CC3,0x3FD826AE,0x3FD5F928,0x3FD3D404,0x3FD1B717,0x3FCFA238,
0x3FCD953E,0x3FCB9001,0x3FC9925B,0x3FC79C25,0x3FC5AD3C,0x3FC3C579,0x3FC1E4BC,0x3FC00AE1,
0x3FBE37C6,0x3FBC6B4C,0x3FBAA552,0x3FB8E5B9,0x3FB72C62,0x3FB57931,0x3FB3CC07,0x3FB224C9,
0x3FB0835A,0x3FAEE79F,0x3FAD517F,0x3FABC0DF,0x3FAA35A6,0x3FA8AFBB,0x3FA72F05,0x3FA5B36E,
0x3FA43CDE,0x3FA2CB3E,0x3FA15E79,0x3F9FF679,0x3F9E9327,0x3F9D3471,0x3F9BDA41,0x3F9A8484,
0x3F993326,0x3F97E614,0x3F969D3C,0x3F95588B,0x3F9417EF,0x3F92DB58,0x3F91A2B4,0x3F906DF2,
0x3F8F3D01,0x3F8E0FD3,0x3F8CE657,0x3F8BC07D,0x3F8A9E36,0x3F897F75,0x3F88642A,0x3F874C46,
0x3F8637BD,0x3F852680,0x3F841883,0x3F830DB7,0x3F820610,0x3F810182,0x3F800000,0x3F7E02FC,
};


extern inline float __inv(float y,float x)  // computes y/x 
{


  const _UINT32 numbits = (sizeof(__inv_x_sqr))/(4*64) + 5;
  // calculated at compile time(hopefully)--assumes minimal # of
 //    elements in inv_sqrt_guess is 32 or an integral (power of two)*32 
  //   
  const _UINT32 bit_shift=23-numbits;
  const _UINT32 bit_mask=0x007fffff&(~( sizeof(__inv_x_sqr)>>2) << bit_shift );
  const _UINT32 first_several_sig_bits_of_x=(*(_UINT32*)&x) & bit_mask;
  const _INT32  biased_exp=(*(_UINT32*)&x) & 0x7f800000;
  
  float guess;
  float scaled_x;
  (*(_UINT32*)&scaled_x) = 0x3F000000 + ((*(_UINT32*)&x)&0x007fffff);  //scaled_x in [0.5,1.0)
   __HI(y) -= (biased_exp- 0x3ff00000);  //subtract off binary exponent of x from expnt(y)
  //(*(_UINT32*)&guess)    = inv_guess[(first_several_sig_bits_of_x >>bit_shift)]; 
  guess=__two_inv_x[(first_several_sig_bits_of_x >>bit_shift)] + 
        __inv_x_sqr[(first_several_sig_bits_of_x >>bit_shift)]*scaled_x ;
        
//  guess *=(2.0f + guess*scaled_x);  // now have 12 sig bits
  guess *=(2.0f + guess*scaled_x);  // now have 24 sig bits

  return y*guess;  //guess= 1/x
  
} 

extern inline float _inv_sqrtf(float x)
{

  const _UINT32 numbits = (sizeof(inv_sqrt_guess))/(4*64) + 5;

  /* calculated at compile time(hopefully)--assumes minimal # of
     elements in inv_sqrt_guess is 32 or an integral (power of two)*32 
  */
     
  const _UINT32 bit_shift=23-numbits;
  const _UINT32 bit_mask=0x007fffff&(~(sizeof(inv_sqrt_guess)>>2)<<bit_shift);
  const _UINT32 first_several_sig_bits_of_x=(*(_UINT32*)&x) & bit_mask;
  const _INT32 biased_exp=(*(_UINT32*)&x) & 0x7f800000;
  float guess;
  float scaled_x;
 
 
   if( biased_exp & 0x00800000)  // if biased_exp is odd then the sqrt of the exponent is 2^^intsqrt(2)
   {
    (*(_UINT32*)&scaled_x)=0x3E800000 + ((*(_UINT32*)&x)&0x007fffff);  //scaled_x in [.25,.5)
    (*(_UINT32*)&guess)=inv_sqrt_guess[(first_several_sig_bits_of_x >>bit_shift)]; 
    
   }
   else
   {
    (*(_UINT32*)&scaled_x)=0x3f000000 + ((*(_INT32*)&x)&0x007fffff);  //scaled_x in [. 5,1.0)
    (*(_UINT32*)&guess)=inv_sqrt_guess2[(first_several_sig_bits_of_x>>bit_shift)]; 
   } 
 
/*   guess = .5f*guess*(3.0f - guess*guess*scaled_x);  // now have 12 sig bits
     guess = .5f*guess*(3.0f - guess*guess*scaled_x);  // now have 24 sig bits
   */
      guess *= (3.0f - guess*guess*scaled_x);  // now have 12 sig bits
      guess *= (3.0f - guess*guess*scaled_x);  // now have 24 sig bits

              
   /* we now reduce x to 2^^n*y  where y is in [.5,1) we then calculate sqrt(x)=sqrt(2^^n)*sqrt(y) 
      where if n is even we simply shift the exponent of guess appropriately or if n is odd we shift
      and multiply by sqrt(2) if n > 0 and 1/sqrt(2) if n > 0
   */
 
  if(biased_exp > 0x3f000000)
    (*(_INT32*)&guess)-=(((biased_exp - 0x3e800000)>>1)&0xffbfffff) ;  // this subtracts off bias(127=0x3f80...)                                                              // from biased_exp and one more which divides by two                
   else
    (*(_INT32*)&guess)+=((0x3f000000 - biased_exp)>>1)&0xffbfffff;  

  // __HI(guess)-=0x01000000;  /* eliminates two multiplies */   
  return guess;
} 
extern inline float sqrtf(float x)
{

if(x <= 0.0f) /* either < 0 or -0 */
    {
     if((*(_UINT32*)&x) & 0x7fffffff) 
      return NAN;
     else 
      return x;  /* x = -0 */
    }  
  
return x*_inv_sqrtf(x);
  
} 
#endif  //_No_Floating_Point_Regs
#pragma cplusplus reset
#endif