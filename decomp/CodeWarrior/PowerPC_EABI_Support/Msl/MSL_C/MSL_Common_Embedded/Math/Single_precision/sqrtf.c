/*
 Author:  Matthew D. Fassiotto
 Date:    first written 4/15/99
 Purpose: single precision sqrt function for architecures without fp hardware
 Assumptions: --
     *IEEE 754 single precision float format
     *fp difference should never produce -0
     *the type _INT32 is 32 bits(i.e. sizeof(_INT32)=sizeof(float))
     
Notes:These two alternate versions of sqrtf are intended for architectures not having
      floating point hardware.  Typically, floating point division is quite expensive 
      on such architectures and therefore by default we use the sqrt algorithm that avoids
      division in the newton iteration sequence.  To use the IEEE 754 compliant square root 
      function you must define the macro  __IEEE754_COMPLIANT__ before compiling this file.    
      The version that uses _inv_sqrt has been tested over millions of distinct fp values 
      and misses the correctly rounded result by at most the last two bits and does so only
      2% of the time.  It also gets the exact result for obvious cases such as perfect squares.
      The largest errors occur near 0.  
      
      The _inv_sqrt is also used by acos and asin. Therefore it's tables are necessary by default.
      Using the IEEE 754 compliant sqrt requires the overhead of the two extra tables as well.
      

Left to do:  

            * Due to potential memory constraints we need to be able to configure the table sizes 
             according to available memory.  The parameters have already been generalized within
             the algorithms but the tables have not been setup yet.  About 1 days work to do this.
             
            * The _INT32 entires in the tables assume there exists some built in 32 bit integral
              type which is not the case for some DSP architecures.  The tables will have
              to be reformated for such architectures.
             

      
Change History:
          09/15/99-- added implementation of _inv_sqrt-- to be used by asin/acos and
                     sqrt(x)=x*_inv_sqrt(x).      
*/
/*#if ( __POWERPC__ && defined(_No_Floating_Point_Regs) )
this macro trick seems to work some of the time, but it aint clear yet as to what the eppc compiler
is doing when Software FPU is selected in the processor pref panel, so for now we just remove this
file from the hardware library targets.  We use the same algorithm but through the compiler intrinsic
__feinvsqrt.
*/

#pragma cplusplus off
#include <math.h>
#if __IEEE754_COMPLIANT__
# if 0
static _INT32 sqrt_guess[]=  {0x3F3504F3,0x3F37D375,0x3F3A9728,0x3F3D5087,
                              0x3F400000,0x3F42A5FE,0x3F4542E1,0x3F47D706,
                              0x3F4A62C2,0x3F4CE665,0x3F4F623A,0x3F51D689,
                              0x3F544395,0x3F56A99B,0x3F5908D9,0x3F5B6186,
                              0x3F5DB3D7,0x3F600000,0x3F624630,0x3F648695,
                              0x3F66C15A,0x3F68F6A9,0x3F6B26A9,0x3F6D517F,
                              0x3F6F7751,0x3F71983E,0x3F73B46A,0x3F75CBF2,
                              0x3F77DEF6,0x3F79ED91,0x3F7BF7DF,0x3F7DFDFC};
                           
# elif 0                          
static _INT32 sqrt_guess[]=  {0x3F3504F3,0x3F366D96,0x3F37D375,0x3F3936A1,0x3F3A9728,
                0x3F3BF51B,0x3F3D5087,0x3F3EA979,0x3F400000,0x3F415428,0x3F42A5FE,
				0x3F43F58D,0x3F4542E1,0x3F468E06,0x3F47D706,0x3F491DEC,0x3F4A62C2,
				0x3F4BA592,0x3F4CE665,0x3F4E2545,0x3F4F623A,0x3F509D4E,0x3F51D689,
				0x3F530DF3,0x3F544395,0x3F557775,0x3F56A99B,0x3F57DA10,0x3F5908D9,
				0x3F5A35FE,0x3F5B6186,0x3F5C8B77,0x3F5DB3D7,0x3F5EDAAE,0x3F600000,
				0x3F6123D4,0x3F624630,0x3F636719,0x3F648695,0x3F65A4A9,0x3F66C15A,
				0x3F67DCAE,0x3F68F6A9,0x3F6A0F50,0x3F6B26A9,0x3F6C3CB7,0x3F6D517F,
				0x3F6E6507,0x3F6F7751,0x3F708862,0x3F71983E,0x3F72A6EA,0x3F73B46A,
				0x3F74C0C0,0x3F75CBF2,0x3F76D603,0x3F77DEF6,0x3F78E6CE,0x3F79ED91,
				0x3F7AF340,0x3F7BF7DF,0x3F7CFB72,0x3F7DFDFC,0x3F7EFF7F};
                            
                            
# else
/* Note: these values are calculated at the midpoint of each sub-interval in [.5,2] */
static _INT32 sqrt_guess[]={0x3F35B99E,0x3F366D96,0x3F3720DD,0x3F37D375,0x3F388560,0x3F3936A1,0x3F39E738,
0x3F3A9728,0x3F3B4673,0x3F3BF51B,0x3F3CA321,0x3F3D5087,0x3F3DFD4E,0x3F3EA979,0x3F3F5509,0x3F400000,
0x3F40AA5F,0x3F415428,0x3F41FD5C,0x3F42A5FE,0x3F434E0D,0x3F43F58D,0x3F449C7E,0x3F4542E1,0x3F45E8B9,
0x3F468E06,0x3F4732CA,0x3F47D706,0x3F487ABC,0x3F491DEC,0x3F49C098,0x3F4A62C2,0x3F4B046A,0x3F4BA592,
0x3F4C463A,0x3F4CE665,0x3F4D8613,0x3F4E2545,0x3F4EC3FC,0x3F4F623A,0x3F500000,0x3F509D4E,0x3F513A26,
0x3F51D689,0x3F527278,0x3F530DF3,0x3F53A8FD,0x3F544395,0x3F54DDBC,0x3F557775,0x3F5610BF,0x3F56A99B,
0x3F57420B,0x3F57DA10,0x3F5871A9,0x3F5908D9,0x3F599FA0,0x3F5A35FE,0x3F5ACBF5,0x3F5B6186,0x3F5BF6B1,
0x3F5C8B77,0x3F5D1FD9,0x3F5DB3D7,0x3F5E4773,0x3F5EDAAE,0x3F5F6D87,0x3F600000,0x3F609219,0x3F6123D4,
0x3F61B531,0x3F624630,0x3F62D6D3,0x3F636719,0x3F63F704,0x3F648695,0x3F6515CC,0x3F65A4A9,0x3F66332E,
0x3F66C15A,0x3F674F2F,0x3F67DCAE,0x3F6869D6,0x3F68F6A9,0x3F698327,0x3F6A0F50,0x3F6A9B26,0x3F6B26A9,
0x3F6BB1D9,0x3F6C3CB7,0x3F6CC744,0x3F6D517F,0x3F6DDB6B,0x3F6E6507,0x3F6EEE53,0x3F6F7751,0x3F700000,
0x3F708862,0x3F711076,0x3F71983E,0x3F721FBA,0x3F72A6EA,0x3F732DCF,0x3F73B46A,0x3F743ABA,0x3F74C0C0,
0x3F75467E,0x3F75CBF2,0x3F76511E,0x3F76D603,0x3F775AA0,0x3F77DEF6,0x3F786305,0x3F78E6CE,0x3F796A52,
0x3F79ED91,0x3F7A708B,0x3F7AF340,0x3F7B75B1,0x3F7BF7DF,0x3F7C79CA,0x3F7CFB72,0x3F7D7CD8,0x3F7DFDFC,
0x3F7E7EDE,0x3F7EFF7F,0x3F7F7FE0,0x3F800000,0x3F803FF0};

static _INT32 sqrt_guess2[]={0x3F00FF02,0x3F017DC7,0x3F01FC10,0x3F0279DF,0x3F02F734,0x3F037413,0x3F03F07B,
0x3F046C6F,0x3F04E7EE,0x3F0562FC,0x3F05DD98,0x3F0657C5,0x3F06D182,0x3F074AD3,0x3F07C3B6,0x3F083C2F,
0x3F08B43D,0x3F092BE3,0x3F09A320,0x3F0A19F6,0x3F0A9067,0x3F0B0672,0x3F0B7C1A,0x3F0BF15E,0x3F0C6641,
0x3F0CDAC3,0x3F0D4EE4,0x3F0DC2A7,0x3F0E360B,0x3F0EA912,0x3F0F1BBD,0x3F0F8E0C,0x3F100000,0x3F10719A,
0x3F10E2DC,0x3F1153C4,0x3F11C456,0x3F123491,0x3F12A476,0x3F131406,0x3F138341,0x3F13F229,0x3F1460BE,
0x3F14CF01,0x3F153CF2,0x3F15AA92,0x3F1617E3,0x3F1684E4,0x3F16F196,0x3F175DFA,0x3F17CA11,0x3F1835DC,
0x3F18A15A,0x3F190C8C,0x3F197774,0x3F19E211,0x3F1A4C65,0x3F1AB66F,0x3F1B2032,0x3F1B89AC,0x3F1BF2DF,
0x3F1C5BCB,0x3F1CC471,0x3F1D2CD1,0x3F1D94EC,0x3F1DFCC2,0x3F1E6455,0x3F1ECBA4,0x3F1F32AF,0x3F1F9979,
0x3F200000,0x3F206646,0x3F20CC4A,0x3F21320E,0x3F219792,0x3F21FCD7,0x3F2261DC,0x3F22C6A3,0x3F232B2B,
0x3F238F75,0x3F23F383,0x3F245753,0x3F24BAE7,0x3F251E3E,0x3F25815A,0x3F25E43B,0x3F2646E1,0x3F26A94D,
0x3F270B7F,0x3F276D77,0x3F27CF36,0x3F2830BC,0x3F28920A,0x3F28F31F,0x3F2953FD,0x3F29B4A4,0x3F2A1514,
0x3F2A754D,0x3F2AD550,0x3F2B351D,0x3F2B94B5,0x3F2BF417,0x3F2C5345,0x3F2CB23E,0x3F2D1104,0x3F2D6F95,
0x3F2DCDF3,0x3F2E2C1E,0x3F2E8A16,0x3F2EE7DB,0x3F2F456F,0x3F2FA2D0,0x3F300000,0x3F305CFF,0x3F30B9CC,
0x3F31166A,0x3F3172D6,0x3F31CF13,0x3F322B20,0x3F3286FE,0x3F32E2AC,0x3F333E2C,0x3F33997C,0x3F33F49F,
0x3F344F93,0x3F34AA5A,0x3F3504F3,0x3F355F5F,0x3F35B99E
};

#endif
#else
static _INT32 inv_sqrt_guess[]={
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
static _INT32 inv_sqrt_guess2[]={
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


#endif  /* __IEEE754_COMPLIANT__ */
 
#if !( __MIPS__ && __MIPS_ISA2__ && __MIPS_single_fpu__)
float _inv_sqrtf(float x);
float sqrtf(float x);
float sqrtf(float x)
{
#if __IEEE754_COMPLIANT__
  const _UINT32 numbits = (sizeof(sqrt_guess))/(4*64) + 5;

  /* calculated at compile time(hopefully)--assumes minimal # of
     elements in sqrt_guess is 32 or an integral (power of two)*32 
  */
     
  const _UINT32 bit_shift=23-numbits;
  const _UINT32 bit_mask=0x007fffff&(~(sizeof(sqrt_guess)>>2)<<bit_shift);
  const _UINT32 first_several_sig_bits_of_x=(*(_UINT32*)&x) & bit_mask;
  const _INT32 biased_exp=(*(_UINT32*)&x) & 0x7f800000;
  float guess;
  float scaled_x;
 
  if(*(_UINT32*)&x & 0x80000000) /* either < 0 or -0 */
    {
     if((*(_UINT32*)&x) & 0x7fffffff) 
      return NAN;
     else 
      return x;  /* x = -0 */
    }  
  
  //if(!biased_exp) return 0.0f;  //flush denormal to 0.0
  /* the condition below insures that we round x so that ||sqrt(x)-guess||<=||sqrt(x)-y|| for all y in sqrt_guess[](round to nearest)
     since sqrt is monotonically increasing --> ||sqrt(x)-sqrt(guess)|| <= ||sqrt(x)-sqrt(y)||
     we look at the remaining low order significant bits of x below the bit_mask.
  */
 
   if( biased_exp & 0x00800000)  // if biased_exp is odd then the sqrt of the exponent is 2^^intsqrt(2)
   {
    (*(_UINT32*)&scaled_x)=0x3E800000 + ((*(_UINT32*)&x)&0x007fffff);  //scaled_x in [.25,.5)
    (*(_UINT32*)&guess)=sqrt_guess2[(first_several_sig_bits_of_x>>bit_shift)]; 
    
   }
   else
   {
    (*(_UINT32*)&scaled_x)=0x3f000000 + ((*(_INT32*)&x)&0x007fffff);  //scaled_x in [.5,1.0)
    (*(_UINT32*)&guess)=sqrt_guess[(first_several_sig_bits_of_x>>bit_shift)]; 
   } 
   
   guess += scaled_x/guess;  // now have 12 sig bits
   guess =.25f*guess + (scaled_x/guess);  // now we have about 24 sig bits
              
   /* we now reduce x to 2^^n*y  where y is in [.5,1) we then calculate sqrt(x)=sqrt(2^^n)*sqrt(y) 
      where if n is even we simply shift the exponent of guess appropriately or if n is odd we shift
      and multiply by sqrt(2) if n > 0 and 1/sqrt(2) if n > 0
   */
   
   if(biased_exp > 0x3f000000)
    (*(_INT32*)&guess)+=(((biased_exp - 0x3e800000)>>1)&0xffbfffff) ;  // this subtracts off bias(127=0x3f80...)                                                              // from biased_exp and one more which divides by two                
   else
    (*(_INT32*)&guess)-=((0x3f000000 - biased_exp)>>1)&0xffbfffff;  
    
  return guess;
#else
if(x <= 0.0f) /* either < 0 or -0 */
    {
     if((*(_UINT32*)&x) & 0x7fffffff) 
      return NAN;
     else 
      return x;  /* x = -0 */
    }  
  
return x*_inv_sqrtf(x);
#endif
  
}  


double __frsqrte(double);

extern inline float _inv_sqrtf(float x)
{
#if 1
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
 
  //if(*(_UINT32*)&x & 0x80000000) /* either < 0 or -0 */
 // {
  // if((*(_UINT32*)&x) & 0x7fffffff) return NAN;
 //  else return x;  /* x = -0 */
 // }  
  
  //if(!biased_exp) return 0.0f;  //flush denormal to 0.0
  /* the condition below insures that we round x so that ||sqrt(x)-guess||<=||sqrt(x)-y|| for all y in inv_sqrt_guess[](round to nearest)
     since sqrt is monotonically increasing --> ||sqrt(x)-sqrt(guess)|| <= ||sqrt(x)-sqrt(y)||
     we look at the remaining low order significant bits of x below the bit_mask.
  */
 
   if( biased_exp & 0x00800000)  // if biased_exp is odd then the sqrt of the exponent is 2^^intsqrt(2)
   {
    (*(_UINT32*)&scaled_x)=0x3E800000 + ((*(_UINT32*)&x)&0x007fffff);  //scaled_x in [.25,.5)
    (*(_UINT32*)&guess)=inv_sqrt_guess[(first_several_sig_bits_of_x >>bit_shift)]; 
    
   }
   else
   {
    (*(_UINT32*)&scaled_x)=0x3f000000 + ((*(_INT32*)&x)&0x007fffff);  //scaled_x in [.5,1.0)
    (*(_UINT32*)&guess)=inv_sqrt_guess2[(first_several_sig_bits_of_x>>bit_shift)]; 
   } 
#else
double y=x;
float guess = __frsqrte(y);   
#endif   
   guess = .5f*guess*(3.0f - guess*guess*scaled_x);  // now have 12 sig bits
   guess = .5f*guess*(3.0f - guess*guess*scaled_x);  // now have 24 sig bits
   
              
   /* we now reduce x to 2^^n*y  where y is in [.5,1) we then calculate sqrt(x)=sqrt(2^^n)*sqrt(y) 
      where if n is even we simply shift the exponent of guess appropriately or if n is odd we shift
      and multiply by sqrt(2) if n > 0 and 1/sqrt(2) if n > 0
   */
#if 1   
   if(biased_exp > 0x3f000000)
    (*(_INT32*)&guess)-=(((biased_exp - 0x3e800000)>>1)&0xffbfffff) ;  // this subtracts off bias(127=0x3f80...)                                                              // from biased_exp and one more which divides by two                
   else
    (*(_INT32*)&guess)+=((0x3f000000 - biased_exp)>>1)&0xffbfffff;  
#endif    
  return guess;
}  
#endif
#pragma cplusplus reset
/*#endif */ /* #if (__POWERPC__ && _No_Floating_Point_Regs) */