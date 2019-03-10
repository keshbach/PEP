/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_unate.h"
#include "espresso_set.h"
#include "espresso_contain.h"

/*
 *  abs_covered -- after selecting a new column for the selected set,
 *  create a new matrix which is only those rows which are still uncovered
 */
static TPSet_Family abs_covered(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  register TEspressoInt32 pick)
{
    register TPSet last, p, pdest;
    register TPSet_Family Aprime;

    Aprime = sf_new(pCubeContext, A->nCount, A->nSF_size);
    pdest = Aprime->pData;
    MForeach_Set(A, last, p)
	if (! MIs_In_Set(p, pick))
    {
	    MInlineSet_Copy(pdest, p);
	    Aprime->nCount++;
	    pdest += Aprime->nWSize;
	}
    return Aprime;
}

/*
 *  abs_covered_many -- after selecting many columns for ther selected set,
 *  create a new matrix which is only those rows which are still uncovered
 */
static TPSet_Family abs_covered_many(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  register TPSet pick_set)
{
    register TPSet last, p, pdest;
    register TPSet_Family Aprime;

    Aprime = sf_new(pCubeContext, A->nCount, A->nSF_size);
    pdest = Aprime->pData;
    MForeach_Set(A, last, p)
	if (setp_disjoint(p, pick_set))
    {
	    MInlineSet_Copy(pdest, p);
	    Aprime->nCount++;
	    pdest += Aprime->nWSize;
	}
    return Aprime;
}

/*
 *  abs_select_restricted -- select the column of maximum column count which
 *  also belongs to the set "restrict"; weight each column of a set as
 *  1 / (set_ord(p) - 1).
 */
static TEspressoInt32 abs_select_restricted(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  TPSet restrict)
{
    register TEspressoInt32 i, best_var, best_count, *count;

    /* Sum the elements in these columns */
    count = sf_count_restricted(pCubeContext, A, restrict);

    /* Find which variable has maximum weight */
    best_var = -1;
    best_count = 0;
    for(i = 0; i < A->nSF_size; i++) 
    {
	    if (count[i] > best_count)
        {
	        best_var = i;
	        best_count = count[i];
	    }
    }

    if (count)
    {
        UtFreeMem(count);
        count = 0;
    }

    if (best_var == -1)
    {
        assert(0);
    	//fatal("abs_select_restricted: should not have best_var == -1");
    }

    return best_var;
}

/*
 *  Assume SIZE(p) records the size of each set
 */
TPSet_Family unate_complement(
  TPCubeContext pCubeContext,
  TPSet_Family A)			/* disposes of A */
{
    TPSet_Family Abar;
    register TPSet p, p1, restrict;
    register TEspressoInt32 i;
    TEspressoInt32 max_i, min_set_ord, j;

    /* Check for no sets in the matrix -- complement is the universe */
    if (A->nCount == 0)
    {
	    sf_free(pCubeContext, A);
	    Abar = sf_new(pCubeContext, 1, A->nSF_size);
	    set_clear(MGetSet(Abar, Abar->nCount++), A->nSF_size);

        /* Check for a single set in the maxtrix -- compute de Morgan complement */
    }
    else if (A->nCount == 1)
    {
	    p = A->pData;
	    Abar = sf_new(pCubeContext, A->nSF_size, A->nSF_size);
	    for(i = 0; i < A->nSF_size; i++)
        {
	        if (MIs_In_Set(p, i))
            {
		        p1 = set_clear(MGetSet(Abar, Abar->nCount++), A->nSF_size);
		        MSet_Insert(p1, i);
	        }
	    }
	    sf_free(pCubeContext, A);
    }
    else
    {

	/* Select splitting variable as the variable which belongs to a set
	 * of the smallest size, and which has greatest column count
	 */
	restrict = MSet_New(A->nSF_size);
	min_set_ord = A->nSF_size + 1;
	MForeachi_Set(A, i, p)
    {
	    if (MSize(p) < (TEspressoUInt32)min_set_ord)
        {
		    set_copy(restrict, p);
		    min_set_ord = MSize(p);
	    }
        else if (MSize(p) == (TEspressoUInt32)min_set_ord) 
        {
		   set_or(restrict, restrict, p);
	    }
	}

	/* Check for no data (shouldn't happen ?) */
	if (min_set_ord == 0) 
    {
	    A->nCount = 0;
	    Abar = A;

	/* Check for "essential" columns */
	}
    else if (min_set_ord == 1)
    {
	    Abar = unate_complement(pCubeContext, abs_covered_many(pCubeContext, A, restrict));
	    sf_free(pCubeContext, A);
	    MForeachi_Set(Abar, i, p)
        {
		    set_or(p, p, restrict);
	    }

	/* else, recur as usual */
	}
    else
    {
	    max_i = abs_select_restricted(pCubeContext, A, restrict);

	    /* Select those rows of A which are not covered by max_i,
	     * recursively find all minimal covers of these rows, and
	     * then add back in max_i
	     */
	    Abar = unate_complement(pCubeContext, abs_covered(pCubeContext, A, max_i));
	    MForeachi_Set(Abar, i, p)
        {
    		MSet_Insert(p, max_i);
	    }

	    /* Now recur on A with all zero's on column max_i */
	    MForeachi_Set(A, i, p)
        {
		    if (MIs_In_Set(p, max_i))
            {
		        MSet_Remove(p, max_i);
		        j = MSize(p) - 1;
		        MPutSize(p, j);
		    }
	    }

	    Abar = sf_append(pCubeContext, Abar, unate_complement(pCubeContext, A));
	}
	MSet_Free(restrict);
    }

    return Abar;
}

/*
 *  unate_compl
 */

TPSet_Family unate_compl(
  TPCubeContext pCubeContext,
  TPSet_Family A)
{
    register TPSet p, last;

    /* Make sure A is single-cube containment minimal */
/*    A = sf_rev_contain(A);*/

    MForeach_Set(A, last, p)
    {
	    MPutSize(p, set_ord(pCubeContext, p));
    }

    /* Recursively find the complement */
    A = unate_complement(pCubeContext, A);

    /* Now, we can guarantee a minimal result by containing the result */
    A = sf_rev_contain(pCubeContext, A);
    return A;
}

TPCover map_cover_to_unate(
  TPCubeContext pCubeContext,
  TPCube *T)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TEspressoUInt32 word_test, word_set, bit_test, bit_set;
    register TPCube p, pA;
    TPSet_Family A;
    TPCube *T1;
    TEspressoInt32 ncol, i;

    A = sf_new(pCubeContext, MCubeListSize(T), pCubeData->nVars_Unate);
    A->nCount = MCubeListSize(T);
    MForeachi_Set(A, i, p)
    {
	    set_clear(p, A->nSF_size);
    }
    ncol = 0;

    for(i = 0; i < pCubeSettings->nSize; i++) 
    {
	    if (pCubeData->pnPart_Zeros[i] > 0)
        {
	        assert(ncol <= pCubeData->nVars_Unate);

	        /* Copy a column from T to A */
	        word_test = MWhich_Word(i);
	        bit_test = 1 << MWhich_Bit(i);
	        word_set = MWhich_Word(ncol);
	        bit_set = 1 << MWhich_Bit(ncol);

	        pA = A->pData;
	        for(T1 = T+2; (p = *T1++) != 0; )
            {
		        if ((p[word_test] & bit_test) == 0) 
                {
		            pA[word_set] |= bit_set;
		        }
		        pA += A->nWSize;
	        }

	        ncol++;
	    }
    }

    return A;
}

TPCover map_unate_to_cover(
  TPCubeContext pCubeContext,
  TPSet_Family A)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TEspressoInt32 i, ncol, lp;
    register TPCube p, pB;
    TEspressoInt32 var, nunate, *unate;
    TPCube last;
    TPSet_Family B;

    B = sf_new(pCubeContext, A->nCount, pCubeSettings->nSize);
    B->nCount = A->nCount;

    /* Find the unate variables */
    unate = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    nunate = 0;
    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    if (pCubeData->pIs_Unate[var])
        {
	        unate[nunate++] = var;
	    }
    }

    /* Loop for each set of A */
    pB = B->pData;
    MForeach_Set(A, last, p)
    {
	/* Initialize this set of B */
	MInlineSet_Fill(pB, pCubeSettings->nSize);

	/* Now loop for the unate variables; if the part is in A,
	 * then this variable of B should be a single 1 in the unate
	 * part.
	 */
	for(ncol = 0; ncol < nunate; ncol++)
    {
	    if (MIs_In_Set(p, ncol))
        {
		    lp = pCubeSettings->pnLast_Part[unate[ncol]];
		    for(i = pCubeSettings->pnFirst_Part[unate[ncol]]; i <= lp; i++)
            {
		        if (pCubeData->pnPart_Zeros[i] == 0)
                {
			        MSet_Remove(pB, i);
		        }
		    }
	    }
	}
	pB += B->nWSize;
    }

    if (unate)
    {
        UtFreeMem(unate);
        unate = 0; 
    }

    return B;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
