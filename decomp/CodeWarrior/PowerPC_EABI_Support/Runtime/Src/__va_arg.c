/****************************************************************************/
/*	Project...:	Metrowerks Standard C Library								*/
/*	Name......:	__va_arg.c													*/
/*	Purpose...:	support function for va_arg macro with PPC EABI				*/
/*				Design from 'Workplace Application Binary Interface for		*/
/*				the PowerPC'.												*/
/*	Copyright.:	竭クopyright 1993-1997 by metrowerks inc. All rights reserved.*/
/****************************************************************************/

typedef struct {
	char gpr;
	char fpr;
	char reserved[2];
	char *input_arg_area;
	char *reg_save_area;
} va_list[1];

typedef enum {
    arg_ARGPOINTER,					/* a pointer to the real 'parameter						*/
    arg_WORD,						/* a 4 byte fixed point number or pointer				*/
    arg_DOUBLEWORD,					/* long long and SW doubles								*/
    arg_REAL,						/* 8 byte floating point								*/
    arg_VECTOR						/* AltiVec vector										*/
#if __option(incompatible_sfpe_double_params)
	,arg_SFPEDOUBLE	= 10			/* not treated like a arg_DOUBLEWORD 					*/
#endif
} _va_arg_type;

#define MAX_SAVED_REGS	8
#define SIZEOF_GPR		4
#define SIZEOF_FPR		8
#define SIZEOF_DWRD		8
#define SIZEOF_VECTOR	16
#define ALIGN(addr,amount)	(((unsigned int)(addr)+((amount)-1))&~((amount)-1))

void *__va_arg(va_list ap, _va_arg_type type);
void *__va_arg(va_list ap, _va_arg_type type)
{
	char 	*addr;					/* return value - the address of the next 'parameter'	*/
	char	*r = &(ap->gpr); 		/* address of current register number					*/
	int		rr = ap->gpr; 			/* current register										*/
	int		max = MAX_SAVED_REGS;	/* maximum number of registers saved					*/
	int		size = SIZEOF_GPR; 		/* size of 'type'									 	*/
	int		inc = 1;				/* num of registers to increment per type				*/
	int		even = 0;				/* do we have a type that takes 2 registers?			*/
	int		fpr_offset = 0;			/* where do the floating point registers live			*/
	int		size_reg = SIZEOF_GPR;	/* what is the size of this type's register				*/

#if __ALTIVEC__
	if (type==arg_VECTOR) {
		size = SIZEOF_VECTOR;
		addr = ap->input_arg_area;
		addr = (char*)ALIGN(addr,size);
		ap->input_arg_area = addr + size;
		return addr;
	}
#endif	
	if (type==arg_REAL) {
		r = &(ap->fpr);
		rr = ap->fpr;
		size = SIZEOF_FPR;
		fpr_offset = MAX_SAVED_REGS*SIZEOF_GPR;
		size_reg = SIZEOF_FPR;
	}
	if (type==arg_DOUBLEWORD) {
		size = SIZEOF_DWRD;
		max = max-1;
		if (rr & 1)
			even = 1; 
		inc = 2;
	}
#if __option(incompatible_sfpe_double_params)
	if (type==arg_SFPEDOUBLE) {
		size = SIZEOF_DWRD;
		max = max-1;
		inc = 2;
	}
#endif
	if (rr < max) {
		rr += even;
		addr = ap->reg_save_area + fpr_offset + (rr*size_reg);
		*r = rr + inc;
	} else {
		*r = MAX_SAVED_REGS; /* if arg_DOUBLEWORD, make sure that gprs won't be used again */
		addr = ap->input_arg_area;
		addr = (char*)ALIGN(addr,size);
		ap->input_arg_area = addr + size;
	}
    if (type==arg_ARGPOINTER)
        addr = *((char**)addr);

	return addr;
}