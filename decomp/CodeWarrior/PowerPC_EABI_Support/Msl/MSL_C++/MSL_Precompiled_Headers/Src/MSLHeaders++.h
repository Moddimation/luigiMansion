/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:57:44 $ 
 *  $Revision: 1.1.8.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	Prefix file for MSL C++ projects which precompiles in the most common
 *	MSL Header files.
 *
 *	This prefix file is ONLY for C++ projects, it will not work for C projects unless
 *	C++ is activated.
 *
 */

/* static initializations */

#if __CFM68K__
	#include <MSLHeaders.CFM68K.mch>
#elif __MC68K__
	#include <MSLHeaders.68K.mch>
#elif __POWERPC__
	#include <MSLHeaders.PPC.mch>
#elif __INTEL__
	#include <MSLHeaders.x86.mch>
#else
	#error "target currently unsupported.mch"
#endif

//#include <MSLPrefixPartial.h>

#if 0

	#if __option(far_data) || __option(far_strings) || __option(far_vtables)
		#if __option(mpwc_newline)
			#if __option(fourbyteints)
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_4i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_4i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_4i_F).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_4i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_4i_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_4i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_4i).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_4i)A4.mch>
						#endif
					#endif
				#endif
			#else
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_2i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_2i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_2i_F).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_2i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_2i_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_2i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(NL_2i).mch>
						#else
							#include <MSL C++ PCH.68kFa(NL_2i)A4.mch>
						#endif
					#endif
				#endif
			#endif
		#else
			#if __option(fourbyteints)
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(4i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(4i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(4i_F).mch>
						#else
							#include <MSL C++ PCH.68kFa(4i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(4i_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(4i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(4i).mch>
						#else
							#include <MSL C++ PCH.68kFa(4i)A4.mch>
						#endif
					#endif
				#endif
			#else
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(2i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(2i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(2i_F).mch>
						#else
							#include <MSL C++ PCH.68kFa(2i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68kFa(2i_8d).mch>
						#else
							#include <MSL C++ PCH.68kFa(2i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68kFa(2i).mch>
						#else
							#include <MSL C++ PCH.68kFa(2i)A4.mch>
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if __option(mpwc_newline)
			#if __option(fourbyteints)
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(NL_4i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68k(NL_4i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(NL_4i_F).mch>
						#else
							#include <MSL C++ PCH.68k(NL_4i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(NL_4i_8d).mch>
						#else
							#include <MSL C++ PCH.68k(NL_4i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(NL_4i).mch>
						#else
							#include <MSL C++ PCH.68k(NL_4i)A4.mch>
						#endif
					#endif
				#endif
			#else
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(NL_2i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68k(NL_2i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(NL_2i_F).mch>
						#else
							#include <MSL C++ PCH.68k(NL_2i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(NL_2i_8d).mch>
						#else
							#include <MSL C++ PCH.68k(NL_2i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(NL_2i).mch>
						#else
							#include <MSL C++ PCH.68k(NL_2i)A4.mch>
						#endif
					#endif
				#endif
			#endif
		#else
			#if __option(fourbyteints)
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(4i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68k(4i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(4i_F).mch>
						#else
							#include <MSL C++ PCH.68k(4i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(4i_8d).mch>
						#else
							#include <MSL C++ PCH.68k(4i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(4i).mch>
						#else
							#include <MSL C++ PCH.68k(4i)A4.mch>
						#endif
					#endif
				#endif
			#else
				#if __option(code68881)
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(2i_F_8d).mch>
						#else
							#include <MSL C++ PCH.68k(2i_F_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(2i_F).mch>
						#else
							#include <MSL C++ PCH.68k(2i_F)A4.mch>
						#endif
					#endif
				#else
					#if __option(IEEEdoubles)
						#if __A5__
							#include <MSL C++ PCH.68k(2i_8d).mch>
						#else
							#include <MSL C++ PCH.68k(2i_8d)A4.mch>
						#endif
					#else
						#if __A5__
							#include <MSL C++ PCH.68k(2i).mch>
						#else
							#include <MSL C++ PCH.68k(2i)A4.mch>
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif


#endif
