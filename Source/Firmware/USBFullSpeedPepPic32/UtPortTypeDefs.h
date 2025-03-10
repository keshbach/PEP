/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPortTypeDefs_H)
#define UtPortTypeDefs_H

#if defined(__32MX250F128B__) || defined(__32MX440F256H__)
#define _In_
#define _Out_

#define _IRQL_requires_max_(PASSIVE_LEVEL)
#endif

#if defined(__32MX250F128B__)
#define TRUE 1
#define FALSE 0

#define ULONG unsigned long int
#define LONGLONG __extension__ signed long long

#define INT8 signed char
#define UINT8 unsigned char

#define INT16 short int
#define UINT16 unsigned short int

#define INT32 signed long
#define UINT32 unsigned long

#define INT64 __extension__ signed long long 
#define UINT64 __extension__ unsigned long long 

#define BOOLEAN unsigned char
#define BOOL unsigned char
#define PBOOLEAN unsigned char*

#define VOID void
#define PVOID void*

#define PUINT8 unsigned char*
#elif defined(__32MX440F256H__)
#define TRUE 1
#define FALSE 0

#define UINT8 unsigned char

#define UINT16 unsigned short int

#define INT32 signed long
#define UINT32 unsigned long

#define UINT64 __extension__ unsigned long long 

#define BOOLEAN unsigned char
#define BOOL unsigned char
#define PBOOLEAN unsigned char*

#define VOID void
#define PVOID void*

#define PUINT8 unsigned char*
#else
#error Unknown device configuration
#endif

#endif /* !defined(UtPortTypeDefs_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
