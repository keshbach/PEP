/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPortTypeDefs_H)
#define UtPortTypeDefs_H

#if defined(__XC8) || defined(__18CXX)

#define _In_
#define _Out_

#define TRUE 1
#define FALSE 0

#define ULONG unsigned long
#define LONGLONG long long

#define INT8 char
#define UINT8 unsigned char

#define INT16 int
#define UINT16 unsigned int

#define INT32 long
#define UINT32 unsigned long

#define BOOLEAN unsigned char
#define BOOL unsigned char
#define PBOOLEAN unsigned char*

#define VOID void
#define PVOID void*

#define PUCHAR unsigned char*

#define _IRQL_requires_max_(PASSIVE_LEVEL)

typedef struct tagU
{
    UINT32 LowPart;
    INT32 HighPart;
} U;

typedef union
{
    U u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

#define PLARGE_INTEGER LARGE_INTEGER*

#endif /* defined(__XC8) || defined(__18CXX) */

#endif /* !defined(UtPortTypeDefs_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
