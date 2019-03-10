/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_defs_h)
#define espresso_defs_h

/*
 *  set.h -- definitions for packed arrays of bits
 *
 *   This header file describes the data structures which comprise a
 *   facility for efficiently implementing packed arrays of bits
 *   (otherwise known as sets, cf. Pascal).
 *
 *   A set is a vector of bits and is implemented here as an array of
 *   unsigned integers.  The low order bits of set[0] give the index of
 *   the last word of set data.  The higher order bits of set[0] are
 *   used to store data associated with the set.  The set data is
 *   contained in elements set[1] ... set[MLoop(set)] as a packed bit
 *   array.
 *
 *   A family of sets is a two-dimensional matrix of bits and is
 *   implemented with the data type "set_family".
 *
 *   CBitsPerInteger == 32 and CBitsPerInteger == 16 have been tested and work.
 */

/* Define host machine characteristics of "unsigned int" */
#if defined(WIN32) || defined(WIN64)
typedef INT32 TEspressoInt32;
typedef UINT32 TEspressoUInt32;
#else
#error Unknown host machine please define a 32-bit integer and unsigned integer
#endif

#define CBitsPerInteger 32    /* # bits per integer */

#if CBitsPerInteger == 32
#define CLogBitsPerInteger 5  /* log(CBitsPerInteger)/log(2) */
#else
#error Undefined bits per integer
#endif

#define CBitCountSize 256

/* Define the set type */
typedef TEspressoUInt32* TPSet;
typedef TEspressoUInt32 TSet;

/* Define the set family type -- an array of sets */
typedef struct tagTSet_Family
{
    TEspressoInt32         nWSize;        /* Size of each set in 'ints' */
    TEspressoInt32         nSF_size;      /* User declared set size */
    TEspressoInt32         nCapacity;     /* Number of sets allocated */
    TEspressoInt32         nCount;        /* The number of sets in the family */
    TEspressoInt32         nActive_Count; /* Number of "active" sets */
    TPSet                  pData;         /* Pointer to the set data */
    struct tagTSet_Family* pNext;         /* For garbage collection */
} TSet_Family, *TPSet_Family;

/* This is a hack which I wish I hadn't done, but too painful to change */
#define MCubeListSize(T)         (TEspressoInt32)(((TPCube *) T[1] - T) - 3)

/* The pla_type field describes the input and output format of the PLA */
#define F_type 1
#define D_type 2
#define R_type 4
#define FD_type (F_type | D_type)

/* Macros to set and test single elements */
#define MWhich_Word(element) (((element) >> CLogBitsPerInteger) + 1)
#define MWhich_Bit(element) ((element) & (CBitsPerInteger-1))

/* # of ints needed to allocate a set with "size" elements */
#if CBitsPerInteger == 32
#define MSet_Size(size) ((size) <= CBitsPerInteger ? 2 : (MWhich_Word((size)-1) + 1))
#elif CBitsPerInteger == 16
#define MSet_Size(size) ((size) <= CBitsPerInteger ? 3 : (MWhich_Word((size)-1) + 2))
#else
#error Undefined bits per integer
#endif

/*
 *  Three fields are maintained in the first word of the set
 *      MLoop is the index of the last word used for set data
 *      MLoopCopy is the index of the last word in the set
 *      MSize is available for general use (e.g., recording # elements in set)
 *      MNElem retrieves the number of elements in the set
 */
#define MLoop(set)              (set[0] & 0x03ff)
#define MPutLoop(set, i)        (set[0] &= ~0x03ff, set[0] |= (i))

#if CBitsPerInteger == 32
#define MLoopCopy(set)          MLoop(set)
#define MSize(set)              (set[0] >> 16)
#define MPutSize(set, size)     (set[0] &= 0xffff, set[0] |= ((size) << 16))
#elif CBitsPersInteger == 16
#define MLoopCopy(set)          (MLoop(set) + 1)
#define MSize(set)              (set[MLoop(set)+1])
#define MPutSize(set, size)     ((set[MLoop(set)+1]) = (size))
#else
#error Undefined bits per integer
#endif

#define MNElem(set)     (CBitsPerInteger * MLoop(set))
#define MLoopInit(size)	((size <= CBitsPerInteger) ? 1 : MWhich_Word((size)-1))

/*
 *      FLAGS store general information about the set
 */
#define MSet(set, flag)         (set[0] |= (flag))
#define MReset(set, flag)       (set[0] &= ~ (flag))
#define MTestP(set, flag)       (set[0] & (flag))

/* Flag definitions are ... */
#define CEspressoPrime     0x8000          /* cube is prime */
#define CEspressoNoneEssen 0x4000          /* cube cannot be essential prime */
#define CEspressoActive    0x2000          /* cube is still active */
#define CEspressoRedund    0x1000          /* cube is redundant(at this point) */
#define CEspressoCovered   0x0800          /* cube has been covered */
#define CEspressoRelEssen  0x0400          /* cube is relatively essential */

/* Most efficient way to look at all members of a set family */
#define MForeach_Set(R, last, p) \
    for(p=R->pData,last=p+R->nCount*R->nWSize;p<last;p+=R->nWSize)
#define MForeach_Remaining_Set(R, last, pfirst, p) \
    for(p=pfirst+R->nWSize,last=R->pData+R->nCount*R->nWSize;p<last;p+=R->nWSize)
#define MForeach_Active_Set(R, last, p) \
    MForeach_Set(R,last,p) if (MTestP(p, CEspressoActive))

/* Another way that also keeps the index of the current set member in i */
#define MForeachi_Set(R, i, p) \
    for(p=R->pData,i=0;i<R->nCount;p+=R->nWSize,i++)

/* Looping over all elements in a set:
 *      foreach_set_element(pset p, int i, unsigned val, int base) {
 *		.
 *		.
 *		.
 *      }
 */
#define MForeach_Set_Element(p, i, val, base) \
    for(i = MLoop(p); i > 0; ) \
    	for(val = p[i], base = --i << CLogBitsPerInteger; val != 0; base++, val >>= 1) \
	        if (val & 1)

/* Return a pointer to a given member of a set family */
#define MGetSet(family, index)   ((family)->pData + (family)->nWSize * (index))

/* Allocate and deallocate sets */
#define MSet_New(size)  set_clear((TPSet)UtAllocMem(sizeof(TSet) * MSet_Size(size)), size)
#define MSet_Full(size) set_fill((TPSet)UtAllocMem(sizeof(TSet) * MSet_Size(size)), size)
#define MSet_Save(r)	set_copy((TPSet)UtAllocMem(sizeof(TSet) * MSet_Size(MNElem(r))), r)
#define MSet_Free(r)	if (r) { UtFreeMem(r); r = 0; }

/* Check for set membership, remove set element and insert set element */
#define MIs_In_Set(set, e)  (set[MWhich_Word(e)] & (1 << MWhich_Bit(e)))
#define MSet_Remove(set, e) (set[MWhich_Word(e)] &= ~ (1 << MWhich_Bit(e)))
#define MSet_Insert(set, e) (set[MWhich_Word(e)] |= 1 << MWhich_Bit(e))

#define MInlineSet_Copy(r, a)\
    {register TEspressoInt32 i_=MLoopCopy(a); do r[i_]=a[i_]; while (--i_>=0);}
#define MInlineSet_Clear(r, size)\
    {register TEspressoInt32 i_=MLoopInit(size); *r=i_; do r[i_] = 0; while (--i_ > 0);}
#define MInlineSet_Fill(r, size)\
    {register TEspressoInt32 i_=MLoopInit(size); *r=i_; \
    r[i_]=((TEspressoUInt32)(~0))>>(i_*CBitsPerInteger-size); while(--i_>0) r[i_]=(TEspressoUInt32)~0;}
#define MInlineSet_And(r, a, b)\
    {register TEspressoInt32 i_=MLoop(a); MPutLoop(r,i_);\
    do r[i_] = a[i_] & b[i_]; while (--i_>0);}
#define MInlineSet_Or(r, a, b)\
    {register TEspressoInt32 i_=MLoop(a); MPutLoop(r,i_);\
    do r[i_] = a[i_] | b[i_]; while (--i_>0);}
#define MInlineSet_Diff(r, a, b)\
    {register TEspressoInt32 i_=MLoop(a); MPutLoop(r,i_);\
    do r[i_] = a[i_] & ~ b[i_]; while (--i_>0);}
#define MInlineSet_Xor(r, a, b)\
    {register TEspressoInt32 i_=MLoop(a); MPutLoop(r,i_);\
    do r[i_] = a[i_] ^ b[i_]; while (--i_>0);}
#define MInlineSet_Merge(r, a, b, mask)\
    {register TEspressoInt32 i_=MLoop(a); MPutLoop(r,i_);\
    do r[i_] = (a[i_]&mask[i_]) | (b[i_]&~mask[i_]); while (--i_>0);}
#define MInlineSetP_Implies(a, b, when_false)\
    {register TEspressoInt32 i_=MLoop(a); do if (a[i_]&~b[i_]) break; while (--i_>0);\
    if (i_ != 0) when_false;}

#if CBitsPerInteger == 32
#define MCount_Ones(v, data)\
    (data[v & 255] + data[(v >> 8) & 255]\
    + data[(v >> 16) & 255] + data[(v >> 24) & 255])
#elif CBitsPerInteger == 16
#define MCount_Ones(v, data)\
    (data[v & 255] + data[(v >> 8) & 255])
#else
#error Undefined bits per integer
#endif

/* Define a boolean type */
typedef TEspressoInt32 TEspressoBool;
#define CEspressoFalse 0
#define CEspressoTrue 1
#define CEspressoMaybe 2

/* Map many cube/cover types/routines into equivalent set types/routines */
typedef TPSet                            TPCube;
#define MNew_Cube(cube)                  MSet_New(cube->nSize)
#define MFree_Cube(r)                    MSet_Free(r)
typedef TPSet_Family                     TPCover;
#define MNew_Cover(cubeContext, i, cube) sf_new(cubeContext, i, cube->nSize)
#define MFree_Cover(cubeContext, r)      sf_free(cubeContext, r)
#define MFree_Cubelist(T)                if (T[0]) { UtFreeMem(T[0]); T[0] = 0; } if (T) { UtFreeMem(T); T = 0; }

/*
 *  The cube structure is a global structure which contains information
 *  on how a set maps into a cube -- i.e., number of parts per variable,
 *  number of variables, etc.  Also, many fields are pre-computed to
 *  speed up various primitive operations.
 */
#define CCube_Temp 10

// rename to CubeHeader
typedef struct tagTGlobalCube
{
    TEspressoInt32  nSize;            /* set size of a cube */
    TEspressoInt32  nNum_Vars;        /* number of variables in a cube */
    TEspressoInt32  nNum_Binary_Vars; /* number of binary variables */
    TEspressoInt32* pnFirst_Part;     /* first element of each variable */
    TEspressoInt32* pnLast_Part;      /* first element of each variable */
    TEspressoInt32* pnPart_Size;      /* number of elements in each variable */
    TEspressoInt32* pnFirst_Word;     /* first word for each variable */
    TEspressoInt32* pnLast_Word;      /* last word for each variable */
    TPSet           pBinary_Mask;     /* Mask to extract binary variables */
    TPSet           pMV_Mask;         /* mask to get mv parts */
    TPSet*          ppVar_Mask;       /* mask to extract a variable */
    TPSet*          ppTemp;           /* an array of temporary sets */
    TPSet           pFullSet;         /* a full cube */
    TPSet           pEmptySet;        /* an empty cube */
    TEspressoUInt32 nInMask;          /* mask to get odd word of binary part */
    TEspressoInt32  nInWord;          /* which word number for above */
    TEspressoInt32* pnSparse;         /* should this variable be sparse? */
    TEspressoInt32  nNum_MV_Vars;     /* number of multiple-valued variables */
    TEspressoInt32  nOutput;          /* which variable is "output" (-1 if none) */
} TCubeSettings, *TPCubeSettings;

typedef struct tagTCData
{
    TEspressoInt32* pnPart_Zeros;    /* count of zeros for each element */
    TEspressoInt32* pnVar_Zeros;     /* count of zeros for each variable */
    TEspressoInt32* pnParts_Active;  /* number of "active" parts for each var */
    TEspressoBool*  pIs_Unate;       /* indicates given var is unate */
    TEspressoInt32  nVars_Active;    /* number of "active" variables */
    TEspressoInt32  nVars_Unate;     /* number of unate variables */
    TEspressoInt32  nBest;           /* best "binate" variable */
} TCubeData, *TPCubeData;

typedef struct tagTCubeContext
{
    TCubeSettings  CubeSettings;
    TCubeData      CubeData;
    TEspressoInt32 nBit_Count[CBitCountSize];
    TEspressoBool  use_random_order;
    TEspressoInt32 nRp_current;
    TEspressoBool  nToggle;
    TPSet_Family   pSet_Family_Garbage;
    TEspressoBool  nForce_Irredundant;

    TEspressoInt32 nSingle_Expand;
    TEspressoInt32 nRemove_Essential;
    TEspressoInt32 nUse_Super_Gasp;
    TEspressoInt32 nSkip_Make_Sparse;
    TEspressoInt32 nUnwrap_Onset;
    TEspressoBool Use_Random_Order;
} TCubeContext, *TPCubeContext;

/* cost_t describes the cost of a cover */
typedef struct tagTCost
{
    TEspressoInt32 nCubes;		/* number of cubes in the cover */
    TEspressoInt32 nIn;		/* transistor count, binary-valued variables */
    TEspressoInt32 nOut;		/* transistor count, output part */
    TEspressoInt32 nMv;		/* transistor count, multiple-valued vars */
    TEspressoInt32 nTotal;		/* total number of transistors */
    TEspressoInt32 nPrimes;	/* number of prime cubes */
} TCost, *TPCost;


/* pair_t describes bit-paired variables */
typedef struct tagTPair
{
    TEspressoInt32  nCnt;
    TEspressoInt32* pnVar1;
    TEspressoInt32* pnVar2;
} TPair, *TPPair;


/* symbolic_list_t describes a single ".symbolic" line */
typedef struct tagTSymbolic_List
{
    TEspressoInt32            nVariable;
    TEspressoInt32            nPos;
    struct tagTSymbolic_List* pNext;
} TSymbolic_List, *TPSymbolic_List;


/* symbolic_list_t describes a single ".symbolic" line */
typedef struct tagTSymbolic_Label 
{
    LPTSTR                     pszLabel;
    struct tagTSymbolic_Label* pNext;
} TSymbolic_Label, *TPSymbolic_Label;


/* symbolic_t describes a linked list of ".symbolic" lines */
typedef struct tagTSymbolic
{
    TPSymbolic_List      pSymbolic_List;         /* linked list of items */
    TEspressoInt32       nSymbolic_List_Length;  /* length of symbolic_list list */
    TPSymbolic_Label     pSymbolic_Label;        /* linked list of new names */
    TEspressoInt32       nSymbolic_Label_Length; /* length of symbolic_label list */
    struct tagTSymbolic* pNext;
} TSymbolic, *TPSymbolic;


/* PLA_t stores the logical representation of a PLA */
typedef struct tagTPLA
{
    TPCover        pF;               /* on-set */
    TPCover        pD;               /* off-set */
    TPCover        pR;               /* dc-set */
    TEspressoInt32 nPLA_Type;        /* logical PLA format */
    TPCube         pPhase;           /* phase to split into on-set and off-set */
    TPPair         pPair;            /* how to pair variables */
    LPTSTR*        ppszLabel; 		 /* labels for the columns */
    TPSymbolic     pSymbolic;	     /* allow binary->symbolic mapping */
    TPSymbolic     pSymbolic_Output; /* allow symbolic output mapping */
} TPLA, *TPPLA;

#if CBitsPerInteger == 32
#define CDisjoint 0x55555555
#elif CBitsPerInteger == 16
#define CDisjoint 0x5555
#else
#error Undefined bits per integer
#endif

#define MEspressoAbs(a) ((a) > 0 ? (a) : -(a))
#define MEspressoMax(a,b) ((a) > (b) ? (a) : (b))
#define MEspressoMin(a,b) ((a) < (b) ? (a) : (b))

#define MGetInput(c, pos) \
    ((c[MWhich_Word(2*pos)] >> MWhich_Bit(2*pos)) & 3)

#define CEspressoTwo  3
#define CEspressoDash 3
#define CEspressoOne  2
#define CEspressoZero 1

#define CEspressoQSortCallStyle __cdecl

typedef int (CEspressoQSortCallStyle *TPCompareFunc)(void*, const void*, const void*);

#endif /* end of espresso_defs_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
