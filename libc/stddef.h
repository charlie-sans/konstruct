#ifndef _STDDEF_H
#define _STDDEF_H

/* Basic type definitions for our OS */

/* size_t is used for memory object sizes */
typedef unsigned int size_t;

/* time_t is used for time-related values */
typedef unsigned long time_t;

/* ptrdiff_t is the signed version of size_t */
//typedef long ptrdiff_t;

/* NULL pointer definition */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* Boolean defines */
#define true 1
#define false 0
// typedef unsigned char bool;

/* offsetof - Calculates the offset of a struct member */
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
