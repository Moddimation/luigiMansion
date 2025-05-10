
/*
 *	CPlusLibPPC.cp		-	C++ Runtime Support Routines for Metrowerks C++ (PowerPC)
 *
 *	Copyright 1993-1997 Metrowerks, Inc. All Rights Reserved.
 *
 */

#include <MWCPlusLib.h>


/*
 *	__copy		-	copy 'size' bytes data from 'from' to 'to'
 *
 *	We return the pointer to the destination. If it is 0, no copy is performed.
 *
 */

void *__copy(char *to, char *from, size_t size)
{
	char *p;
	
	if (to && size) {
		p = to;
		do {
			*p = *from;
			++p;
			++from;
			--size;
		} while (size);
	}
	
	return(to);
}


/*
 *	__init_arr	-	initialize an array of objects
 *
 *	Given a pointer to space for an array of 'nobjects' elements of size 'objectsize',
 *	and space to store 'nobjects' and 'objectsize' (for later deletion), we call
 *	the given constructor for each object in the array.
 *
 */
 
void *__init_arr(void *memptr, ConstructorDestructor constructor, size_t objectsize, size_t nobjects)
{
	char *p;
	
	if ((p = (char *) memptr) != 0) {
		((size_t *) p)[0] = objectsize;
		((size_t *) p)[1] = nobjects;
		p += 2 * sizeof(size_t);
		if (constructor) {
			for (; nobjects--; p += objectsize)
				CTORCALL_COMPLETE(constructor, p);
		}
	}
	
	return(memptr);
}


/*
 *	__new_arr	-	allocate and construct an array of objects
 *
 *	We allocate space for an array of 'nobjects' elements of size 'objectsize', and
 *	space to store 'nobjects' and 'objectsize' (for later deletion). Then we call
 *	the given constructor for each object in the array.
 *
 */
 
void *__new_arr(ConstructorDestructor constructor, size_t objectsize, size_t nobjects)
{
	char *memptr, *p;
	
	if ((memptr = (char *) ::operator new(2*sizeof(size_t) + nobjects*objectsize)) != 0) {
		memptr += 2*sizeof(size_t);
		((size_t *) memptr)[-2] = objectsize;
		((size_t *) memptr)[-1] = nobjects;
		if (constructor) {
			for (p = memptr; nobjects--; p += objectsize)
				CTORCALL_COMPLETE(constructor, p);
		}
	}
	
	return(memptr);
}


/*
 *	__del_arr	-	destroy and deallocate an array of objects
 *
 *	We use the previously saved 'nobjects' and 'objectsize' values to call the
 *	destructor for each element of the array. Then we delete the space allocated
 *	to it.
 *
 */
 
void __del_arr(void *memptr, ConstructorDestructor destructor)
{
	size_t nobjects, objectsize;
	char *p;
	
	if (memptr) {
		if (destructor) {
			objectsize = ((size_t *) memptr)[-2];
			nobjects = ((size_t *) memptr)[-1];
			for (p = (char *) memptr+objectsize*nobjects; nobjects--;) {
				p -= objectsize;
				DTORCALL_COMPLETE(destructor, p);
			}
		}
		::delete (&((size_t *) memptr)[-2]);
	}
}


/*
 *	__dc_arr	-	construct or destroy a statically allocated array of objects
 *
 *	We call the given constructor or destructor for each of 'nobjects' elements of size
 *	'objectsize' in a statically allocated array.
 *
 */
 
void __dc_arr(void *memptr, ConstructorDestructor constructordestructor, short objectsize, short nobjects)
{
	char *p;
	
	//	DTORCALL_COMPLETE isn't quite correct for constructions, 
	//	but this function is not used by the current compilers.
	for (p = (char *) memptr; nobjects--; p += objectsize)
		DTORCALL_COMPLETE(constructordestructor, p);
}
