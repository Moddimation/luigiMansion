/*  Single precision hyperbolic functions tanh,sinh,cosh
    NOTE: these are just temporary implementations which will be rewritten.
*/
    
#include <math.h>
#include <float.h>



float sinhf( float arg)
{
	
	if(fabsf(arg) > FLT_EPSILON)
    	return .5f*(expf(arg)- expf(-arg));
    else
     return arg ;    	
}	

float coshf( float arg)
{
   	return .5f*(expf(arg)+ expf(-arg));

}

float tanhf( float arg)
{
if(fabsf(arg) > FLT_EPSILON)	
	return 1.0f - 2.0f/(expf(2.0f*arg)+1.0f);
else
    return arg ;
}
