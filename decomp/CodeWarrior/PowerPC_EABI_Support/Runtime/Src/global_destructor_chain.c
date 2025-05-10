
/*
 *	global_destructor_chain.c	-	global destruction support for CodeWarriorC/C++ (PowerPC)
 *
 *	Copyright ゥ 1995-1997 Metrowerks, Inc. All Rights Reserved.
 *
 *
 *	THEORY OF OPERATION
 *
 *	When the compiler generates code to call initializers for global variables and
 *	nonlocal static variables, it calls the function __register_global_object() to
 *	register the constructed object for destruction.
 *
 *	When the program exits, it calls __destroy_global_chain() to call destructors
 *	for the registered objects, in the reverse order of their construction.
 *
 *	For most applications, there is a single global destructor chain for the
 *	entire program, even if the program is composed of multiple fragments (shared libs)
 *	This is necessary to support the proper program termination sequence, wherein
 *	all destructors must be called before e.g. closing open files and cleaning up
 *	the console window.
 *
 *	"Drop-ins" which may load and unload as a program is running should each have
 *	their own global destructor chain which is created when the drop-in is loaded
 *	and destroyed when the drop-in is unloaded.
 *
 */

#include <MWCPlusLib.h>
#include <NMWException.h>


	/*	public data		*/

DestructorChain *__global_destructor_chain;


/************************************************************************/
/*	Purpose..: 	Register a global object for later destruction			*/
/*	Input....:	pointer to global object								*/
/*	Input....:	pointer to destructor function							*/
/*	Input....:	pointer to global registration structure				*/
/*	Return...:	pointer to global object (pass thru)					*/
/************************************************************************/
extern void *__register_global_object(void *object,void *destructor,void *regmem)
{
	((DestructorChain *)regmem)->next=__global_destructor_chain;
	((DestructorChain *)regmem)->destructor=destructor;
	((DestructorChain *)regmem)->object=object;
	__global_destructor_chain=(DestructorChain *)regmem;

	return object;
}


/************************************************************************/
/* Purpose..: Destroy all constructed global objects					*/
/* Input....: ---														*/
/* Return...: ---														*/
/************************************************************************/
void __destroy_global_chain(void)
{
	DestructorChain	*gdc;

	while((gdc=__global_destructor_chain)!=0L)
	{
		__global_destructor_chain=gdc->next;
		DTORCALL_COMPLETE(gdc->destructor,gdc->object);
	}
}

#if __MWERKS__ && __PPC_EABI__ && __dest_os!=__eppc_vxworks
__declspec(section ".dtors") 
	static void * const __destroy_global_chain_reference = __destroy_global_chain;
#endif