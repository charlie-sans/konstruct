#ifndef _STDDEF_H
#define _STDDEF_H

/* Basic type definitions for our OS */

/* size_t is used for memory object sizes */
typedef unsigned long size_t;

/* ptrdiff_t is the signed version of size_t */
typedef long ptrdiff_t;

/* NULL pointer definition */
#define NULL ((void*)0)

/* offsetof - Calculates the offset of a struct member */
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
