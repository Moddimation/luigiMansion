/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 00:45:30 $ 
 *  $Revision: 1.10.4.3 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	pool_alloc.h
 */
 
#include <ansi_parms.h>                 /* mm 970905*/

#ifndef _MSL_PRO4_MALLOC

#ifndef __pool_alloc__
#define __pool_alloc__

#include <ansi_parms.h>                 /* mm 970905*/
#include <size_t.h>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#define __heap_version	2	/* jg 980115 */ /* hh 990227 */

#ifdef __cplusplus
extern "C" {
#endif

void *	__sys_alloc(__std(size_t) size);
void	__sys_free(void *ptr);

void	__pool_free_all();
#ifdef _No_Alloc_OS_Support
int 	init_alloc(void * heap_ptr, __std(size_t) heap_size); /* hh 990504 */
#endif

#ifdef __cplusplus
}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif

#else /* _MSL_PRO4_MALLOC */

#ifndef __pool_alloc__
#define __pool_alloc__

#include <ansi_parms.h>                 /* mm 970905*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

typedef signed long tag_word;

typedef struct block_header {
	tag_word				tag;
	struct block_header *	prev;
	struct block_header *	next;
} block_header;

typedef struct list_header {
	block_header *		rover;
	block_header		header;
} list_header;

typedef struct heap_header {
	struct heap_header* 	prev;
	struct heap_header*		next;
} heap_header;

typedef unsigned long	mem_size;

typedef void *	(*sys_alloc_ptr)(mem_size size, struct mem_pool_obj*);
typedef void	(*sys_free_ptr)(void * ptr, struct mem_pool_obj*);

typedef struct pool_options{
	sys_alloc_ptr	sys_alloc_func;
	sys_free_ptr	sys_free_func;
	mem_size		min_heap_size;
#if !__ALTIVEC__								
	int				always_search_first;
#else
	char			always_search_first;			/* for ALTIVEC */
	char			block_alignment;
	char			reserved1;
	char			reserved2;
#endif
} pool_options;

typedef struct mem_pool_obj {
	list_header		free_list;
#ifndef _No_Alloc_OS_Support
	pool_options	options;
	heap_header*	heap_list;
	void*			userData;
#endif
} mem_pool_obj;

#define __heap_version	1	/* jg 980115 */

#ifdef __cplusplus
extern "C" {
#endif

_MSL_DLLDATA extern mem_pool_obj __malloc_pool;	/*  vss 980901 */

#if __ALTIVEC__
void	__init_align_pool_obj(mem_pool_obj * pool_obj, char align);
#endif
void	__init_pool_obj(mem_pool_obj * pool_obj);
int		__pool_preallocate(mem_pool_obj * pool_obj, mem_size size);
void	__pool_preassign(mem_pool_obj * pool_obj, void * ptr, mem_size size);
void *	__pool_alloc(mem_pool_obj * pool_obj, mem_size size);
void *	__pool_alloc_clear(mem_pool_obj * pool_obj, mem_size size);
void *	__pool_realloc(mem_pool_obj * pool_obj, void * ptr,	mem_size size);
void	__pool_free(mem_pool_obj * pool_obj, void * ptr);
void	__pool_free_all(mem_pool_obj * pool_obj);

void *	__sys_alloc(mem_size size, mem_pool_obj*);
void	__sys_free(void *ptr, mem_pool_obj*);

#ifdef _No_Alloc_OS_Support
int 	init_alloc(void * heap_ptr, mem_size heap_size);
#endif

#ifdef __cplusplus
}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif

#endif

/*	   Change record
 *	20-Jul-97 MEA   Changed __no_os to _No_Alloc_OS_Support.
 * mm 970905  added include of ansi_parms.h to avoid need for prefix file
 * jg 980115  added __heap_version definition
 * vss 980901 add extern __malloc_pool
 * hh 990227  rewrote allocation functions
 * bobc 990302  support for AltiVec (specify alignment for the pool).
 * hh 990504 Corrected prototype for init_alloc()
 */
