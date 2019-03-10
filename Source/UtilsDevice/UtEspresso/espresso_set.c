/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_set.h"

static void intcpy(
  register TEspressoUInt32 *d,
  register TEspressoUInt32 *s,
  register long n)
{
    register TEspressoInt32 i;
    for (i = 0; i < n; i++)
    {
	    *d++ = *s++;
    }
}

/* bit_index -- find first bit (from LSB) in a word (MSB=bit n, LSB=bit 0) */
TEspressoInt32 bit_index(
  register TEspressoUInt32 a)
{
    register TEspressoInt32 i;

    if (a == 0)
	    return -1;

    for(i = 0; (a & 1) == 0; a >>= 1, i++)
	;
    return i;
}

/* set_clear -- make "r" the empty set of "size" elements */
TPSet set_clear(
  register TPSet r,
  TEspressoInt32 size)
{
    register TEspressoInt32 i = MLoopInit(size);

    *r = i;
    
    do
        r[i] = 0;
    while (--i > 0);

    return r;
}

/* set_fill -- make "r" the universal set of "size" elements */
TPSet set_fill(
  register TPSet r,
  register TEspressoInt32 size)
{
    register TEspressoInt32 i = MLoopInit(size);

    *r = i;
    r[i] = ~ (TEspressoUInt32) 0;
    r[i] >>= i * CBitsPerInteger - size;

    while (--i > 0)
	    r[i] = ~ (TEspressoUInt32) 0;

    return r;
}

/* set_copy -- copy set a into set r */
TPSet set_copy(
  register TPSet r,
  register TPSet a)
{
    register TEspressoInt32 i = MLoopCopy(a);

    do
        r[i] = a[i];
    while (--i >= 0);
    
    return r;
}

/* set_and -- compute intersection of sets "a" and "b" */
TPSet set_and(
  register TPSet r,
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    MPutLoop(r,i);
    do
        r[i] = a[i] & b[i];
    while (--i > 0);

    return r;
}

/* set_ord -- count number of elements in a set */
TEspressoInt32 set_ord(
  TPCubeContext pCubeContext,
  register TPSet a)
{
    register TEspressoInt32 i, sum = 0;
    register TEspressoUInt32 val;

    for(i = MLoop(a); i > 0; i--)
    {
	    if ((val = a[i]) != 0)
        {
	        sum += MCount_Ones(val, pCubeContext->nBit_Count);
        }
    }

    return sum;
}

/* set_dist -- distance between two sets (# elements in common) */
TEspressoInt32 set_dist(
  TPCubeContext pCubeContext,
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i, sum = 0;
    register TEspressoUInt32 val;

    for(i = MLoop(a); i > 0; i--)
    {
	    if ((val = a[i] & b[i]) != 0)
        {
	        sum += MCount_Ones(val, pCubeContext->nBit_Count);
        }
    }

    return sum;
}

/* set_merge -- compute "a" & "mask" | "b" & ~ "mask" */
TPSet set_merge(
  register TPSet r,
  register TPSet a,
  register TPSet b,
  register TPSet mask)
{
    register TEspressoInt32 i = MLoop(a);

    MPutLoop(r,i);

    do
        r[i] = (a[i]&mask[i]) | (b[i]&~mask[i]);
    while (--i > 0);

    return r;
}

/* set_adjcnt -- adjust the counts for a set by "weight" */
void set_adjcnt(
  register TPSet a,
  register TEspressoInt32 *count,
  register TEspressoInt32 weight)
{
    register TEspressoInt32 i, base;
    register TEspressoUInt32 val;

    for(i = MLoop(a); i > 0; )
    {
	    for(val = a[i], base = --i << CLogBitsPerInteger; val != 0; base++, val >>= 1)
        {
	        if (val & 1)
            {
    		    count[base] += weight;
	        }
	    }
    }
}

/* sf_new -- allocate "num" sets of "size" elements each */
TPSet_Family sf_new(
  TPCubeContext pCubeContext,
  TEspressoInt32 num,
  TEspressoInt32 size)
{
    TPSet_Family A;
    if (pCubeContext->pSet_Family_Garbage == NULL) 
    {
	    A = (TPSet_Family)UtAllocMem(sizeof(TSet_Family));
    } 
    else
    {
	    A = pCubeContext->pSet_Family_Garbage;
	    pCubeContext->pSet_Family_Garbage = A->pNext;
    }
    A->nSF_size = size;
    A->nWSize = MSet_Size(size);
    A->nCapacity = num;
    A->pData = (TPSet)UtAllocMem(sizeof(TSet) * ((long) A->nCapacity * A->nWSize));
    A->nCount = 0;
    A->nActive_Count = 0;
    return A;
}

/* sf_save -- create a duplicate copy of a set family */
TPSet_Family sf_save(
  TPCubeContext pCubeContext,
  register TPSet_Family A)
{
    return sf_copy(sf_new(pCubeContext, A->nCount, A->nSF_size), A);
}

/* sf_addset -- add a set to the end of a set family */
TPSet_Family sf_addset(
  TPSet_Family A,
  TPSet s)
{
    register TPSet p;

    if (A->nCount >= A->nCapacity)
    {
	    A->nCapacity = A->nCapacity + A->nCapacity/2 + 1;
	    A->pData = (A->pData) ? ((TPSet)UtReAllocMem(A->pData, sizeof(TSet) * ((long) A->nCapacity * A->nWSize))) :
	                            ((TPSet)UtAllocMem(sizeof(TSet) * ((long) A->nCapacity * A->nWSize)));
    }

    p = MGetSet(A, A->nCount++);

    MInlineSet_Copy(p, s);

    return A;
}

/* sf_free -- free the storage allocated for a set family */
void sf_free(
  TPCubeContext pCubeContext,
  TPSet_Family A)
{
    if (A->pData)
    {
        UtFreeMem(A->pData);

        A->pData = 0;
    }

    A->pNext = pCubeContext->pSet_Family_Garbage;

    pCubeContext->pSet_Family_Garbage = A;
}

/* sf_cleanup -- free all of the set families from the garbage list */
void sf_cleanup(
  TPCubeContext pCubeContext)
{
    register TPSet_Family p, pnext;
    for(p = pCubeContext->pSet_Family_Garbage; p != NULL; p = pnext)
    {
	    pnext = p->pNext;

        if (p)
        {
	        UtFreeMem(p);
        }
    }
    pCubeContext->pSet_Family_Garbage = NULL;
}

/* sf_copy -- copy a set family */
TPSet_Family sf_copy(
  TPSet_Family R,
  TPSet_Family A)
{
    R->nSF_size = A->nSF_size;
    R->nWSize = A->nWSize;
/*R->capacity = A->nCount;*/
/*R->data = REALLOC(unsigned int, R->data, (long) R->capacity * R->wsize);*/
    R->nCount = A->nCount;
    R->nActive_Count = A->nActive_Count;

    intcpy(R->pData, A->pData, (long) A->nWSize * A->nCount);

    return R;
}

/* sf_join -- join A and B into a single set_family */
TPSet_Family sf_join(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  TPSet_Family B)
{
    TPSet_Family R;
    long asize = A->nCount * A->nWSize;
    long bsize = B->nCount * B->nWSize;

    if (A->nSF_size != B->nSF_size)
    {
        assert(0);
        //fatal("sf_join: sf_size mismatch");
    }

    R = sf_new(pCubeContext, A->nCount + B->nCount, A->nSF_size);
    R->nCount = A->nCount + B->nCount;
    R->nActive_Count = A->nActive_Count + B->nActive_Count;
    intcpy(R->pData, A->pData, asize);
    intcpy(R->pData + asize, B->pData, bsize);
    return R;
}

/* sf_active -- make all members of the set family active */
TPSet_Family sf_active(
  TPSet_Family A)
{
    register TPSet p, last;

    MForeach_Set(A, last, p)
    {
    	MSet(p, CEspressoActive);
    }

    A->nActive_Count = A->nCount;

    return A;
}

/* setp_equal -- check if the set "a" equals set "b" */
TEspressoBool setp_equal(
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    do
    {
        if (a[i] != b[i])
        {
            return CEspressoFalse;
        }
    } while (--i > 0);

    return CEspressoTrue;
}

/* setp_disjoint -- check if intersection of "a" and "b" is empty */
TEspressoBool setp_disjoint(
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    do
    {
        if (a[i] & b[i])
        {
            return CEspressoFalse;
        }
    } while (--i > 0);

    return CEspressoTrue;
}

/* setp_implies -- check if "a" implies "b" ("b" contains "a") */
TEspressoBool setp_implies(
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    do
    {
        if (a[i] & ~b[i])
        {
            return CEspressoFalse;
        }
    } while (--i > 0);

    return CEspressoTrue;
}

/* set_or -- compute union of sets "a" and "b" */
TPSet set_or(
  register TPSet r,
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    MPutLoop(r,i);

    do
        r[i] = a[i] | b[i];
    while (--i > 0);

    return r;
}

/* set_diff -- compute difference of sets "a" and "b" */
TPSet set_diff(
  register TPSet r,
  register TPSet a,
  register TPSet b)
{
    register TEspressoInt32 i = MLoop(a);

    MPutLoop(r,i);
    
    do
        r[i] = a[i] & ~b[i];
    while (--i > 0);

    return r;
}

/* sf_append -- append the sets of B to the end of A, and dispose of B */
TPSet_Family sf_append(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  TPSet_Family B)
{
    long asize = A->nCount * A->nWSize;
    long bsize = B->nCount * B->nWSize;

    if (A->nSF_size != B->nSF_size)
    {
        assert(0);
        /*fatal("sf_append: sf_size mismatch");*/
    }
    A->nCapacity = A->nCount + B->nCount;

    if (A->pData)
    {
        A->pData = (TPSet)UtReAllocMem(A->pData,
                                       sizeof(TSet) * ((long) A->nCapacity * A->nWSize));
    }
    else
    {
        A->pData = (TPSet)UtAllocMem(sizeof(TSet) * ((long) A->nCapacity * A->nWSize));
    }

    intcpy(A->pData + asize, B->pData, bsize);

    A->nCount += B->nCount;
    A->nActive_Count += B->nActive_Count;

    sf_free(pCubeContext, B);

    return A;
}

/* sf_count -- perform a column sum over a set family */
TEspressoInt32 *sf_count(
  TPSet_Family A)
{
    register TPSet p, last;
    register TEspressoInt32 i, base, *count;
    register TEspressoUInt32 val;

    count = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * A->nSF_size);
    for(i = A->nSF_size - 1; i >= 0; i--) 
    {
    	count[i] = 0;
    }

    MForeach_Set(A, last, p)
    {
	    for(i = MLoop(p); i > 0; )
        {
	        for(val = p[i], base = --i << CLogBitsPerInteger; val != 0; base++, val >>= 1)
            {
		        if (val & 1)
                {
		            count[base]++;
		        }
	        }
	    }
    }
    return count;
}

/* sf_count_restricted -- perform a column sum over a set family, restricting
 * to only the columns which are in r; also, the columns are weighted by the
 * number of elements which are in each row
 */
TEspressoInt32 *sf_count_restricted(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  register TPSet r)
{
    register TPSet p;
    register TEspressoInt32 i, base, *count;
    register TEspressoUInt32 val;
    TEspressoInt32 weight;
    TPSet last;

    count = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * A->nSF_size);
    for(i = A->nSF_size - 1; i >= 0; i--)
    {
	    count[i] = 0;
    }

    /* Loop for each set */
    MForeach_Set(A, last, p)
    {
	    weight = 1024 / (set_ord(pCubeContext, p) - 1);
	    for(i = MLoop(p); i > 0; )
        {
	        for(val=p[i]&r[i], base= --i<<CLogBitsPerInteger; val!=0; base++, val >>= 1)
            {
		        if (val & 1)
                {
		            count[base] += weight;
		        }
	        }
	    }
    }
    return count;
}

/*
 *  sf_addcol -- add columns to a set family; includes a quick check to see
 *  if there is already enough room (and hence, can avoid copying)
 */
TPSet_Family sf_addcol(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  TEspressoInt32 firstcol,
  TEspressoInt32 n)
{
    TEspressoInt32 maxsize;

    /* Check if adding columns at the end ... */
    if (firstcol == A->nSF_size)
    {
	    /* If so, check if there is already enough room */
	    maxsize = CBitsPerInteger * MLoopInit(A->nSF_size);
	    if ((A->nSF_size + n) <= maxsize)
        {
	        A->nSF_size += n;
	        return A;
	    }
    }
    return sf_delcol(pCubeContext, A, firstcol, -n);
}

/*
 * sf_delcol -- add/delete columns to/from a set family
 *
 *  if n > 0 then n columns starting from firstcol are deleted if n < 0
 *  then n blank columns are inserted starting at firstcol
 *      (i.e., the first new column number is firstcol)
 *
 *  This is done by copying columns in the array which is a relatively
 *  slow operation.
 */
TPSet_Family sf_delcol(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  register TEspressoInt32 firstcol,
  register TEspressoInt32 n)
{
    register TPSet p, last, pdest;
    register TEspressoInt32 i;
    TPSet_Family B;

    B = sf_new(pCubeContext, A->nCount, A->nSF_size - n);
    MForeach_Set(A, last, p)
    {
	    pdest = MGetSet(B, B->nCount++);
	    MInlineSet_Clear(pdest, B->nSF_size);
	    for(i = 0; i < firstcol; i++)
        {
	        if (MIs_In_Set(p, i))
            {
    		    MSet_Insert(pdest, i);
            }
        }
	    for(i = n > 0 ? firstcol + n : firstcol; i < A->nSF_size; i++)
        {
	        if (MIs_In_Set(p, i))
            {
	    	    MSet_Insert(pdest, i - n);
            }
        }
    }
    sf_free(pCubeContext, A);
    return B;
}

/*
 *  sf_copy_col -- copy column "srccol" from "src" to column "dstcol" of "dst"
 */
TPSet_Family sf_copy_col(
  TPSet_Family dst,
  TEspressoInt32 dstcol,
  TPSet_Family src,
  TEspressoInt32 srccol)
{
    register TPSet last, p, pdest;
    register TEspressoInt32 word_test, word_set;
    TEspressoUInt32 bit_set, bit_test;

    /* CHEAT! form these constants outside the loop */
    word_test = MWhich_Word(srccol);
    bit_test = 1 << MWhich_Bit(srccol);
    word_set = MWhich_Word(dstcol);
    bit_set = 1 << MWhich_Bit(dstcol);

    pdest = dst->pData;
    MForeach_Set(src, last, p)
    {
	    if ((p[word_test] & bit_test) != 0)
        {
	        pdest[word_set] |= bit_set;
        }
/*
 *  equivalent code for this is ...
 *	if (is_in_set(p, srccol)) set_insert(pdest, destcol);
 */
    	pdest += dst->nWSize;
    }
    return dst;
}

/*
 *  sf_compress -- delete columns from a matrix
 */
TPSet_Family sf_compress(
  TPCubeContext pCubeContext,
  TPSet_Family A,			/* will be freed */
  register TPSet c)
{
    register TPSet p;
    register TEspressoInt32 i, bcol;
    TPSet_Family B;

    /* create a clean set family for the result */
    B = sf_new(pCubeContext, A->nCount, set_ord(pCubeContext, c));
    for (i = 0; i < A->nCount; i++)
    {
	    p = MGetSet(B, B->nCount++);
	    MInlineSet_Clear(p, B->nSF_size);
    }

    /* copy each column of A which has a 1 in c */
    bcol = 0;
    for (i = 0; i < A->nSF_size; i++)
    {
	    if (MIs_In_Set(c, i))
        {
	        sf_copy_col(B, bcol++, A, i);
	    }
    }
    sf_free(pCubeContext, A);
    return B;
}

/* sf_inactive -- remove all inactive cubes in a set family */
TPSet_Family sf_inactive(
  TPSet_Family A)
{
    register TPSet p, last, pdest;

    pdest = A->pData;
    MForeach_Set(A, last, p) 
    {
	    if (MTestP(p, CEspressoActive))
        {
	        if (pdest != p)
            {
    		    MInlineSet_Copy(pdest, p);
	        }
	        pdest += A->nWSize;
	    }
        else
        {
	        A->nCount--;
	    }
    }
    return A;
}

/* setp_empty -- check if the set "a" is empty */
TEspressoBool setp_empty(
  register TPSet a)
{
    register TEspressoInt32 i = MLoop(a);

    do
    {
        if (a[i])
        {
            return CEspressoFalse;
        }
    } while (--i > 0);

    return CEspressoTrue;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
