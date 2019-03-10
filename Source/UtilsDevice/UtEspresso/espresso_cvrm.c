/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cvrm.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_contain.h"

static void cb_unravel(
  TPCubeContext pCubeContext,
  register TPCube c,
  TEspressoInt32 start,
  TEspressoInt32 end,
  TPCube startbase,
  TPCover B1)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCube base = pCubeSettings->ppTemp[0], p, last;
    TEspressoInt32 expansion, place, skip, var, size, offset;
    register TEspressoInt32 i, j, k, n;

    /* Determine how many cubes it will blow up into, and create a mask
	for those parts that have only a single coordinate
    */
    expansion = 1;
    set_copy(base, startbase);
    for(var = start; var <= end; var++)
    {
	    if ((size = set_dist(pCubeContext, c, pCubeSettings->ppVar_Mask[var])) < 2)
        {
	        set_or(base, base, pCubeSettings->ppVar_Mask[var]);
	    }
        else 
        {
	        expansion *= size;
	    }
    }
    set_and(base, c, base);

    /* Add the unravelled sets starting at the last element of B1 */
    offset = B1->nCount;
    B1->nCount += expansion;
    MForeach_Remaining_Set(B1, last, MGetSet(B1, offset-1), p) 
    {
	    MInlineSet_Copy(p, base);
    }

    place = expansion;
    for(var = start; var <= end; var++)
    {
	    if ((size = set_dist(pCubeContext, c, pCubeSettings->ppVar_Mask[var])) > 1)
        {
	        skip = place;
	        place = place / size;
	        n = 0;
	        for(i = pCubeSettings->pnFirst_Part[var]; i <= pCubeSettings->pnLast_Part[var]; i++)
            {
		        if (MIs_In_Set(c, i))
                {
		            for(j = n; j < expansion; j += skip)
                    {
			            for(k = 0; k < place; k++)
                        {
			                p = MGetSet(B1, j+k+offset);
			                MSet_Insert(p, i);
			            }
		            }
		            n += place;
		        }
	        }
	    }
    }
}

TPCover unravel_range(
  TPCubeContext pCubeContext,
  const TPCover B,
  TEspressoInt32 nStart,
  TEspressoInt32 nEnd)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCover B1;
    TEspressoInt32 var, total_size, expansion, size;
    register TPCube p, last, startbase = pCubeSettings->ppTemp[1];

    /* Create the starting base for those variables not being unravelled */
    set_copy(startbase, pCubeSettings->pEmptySet);
    for(var = 0; var < nStart; var++)
    {
	    set_or(startbase, startbase, pCubeSettings->ppVar_Mask[var]);
    }

    for(var = nEnd+1; var < pCubeSettings->nNum_Vars; var++)
    {
	    set_or(startbase, startbase, pCubeSettings->ppVar_Mask[var]);
    }

    /* Determine how many cubes it will blow up into */
    total_size = 0;
    MForeach_Set(B, last, p)
    {
	    expansion = 1;
	    for(var = nStart; var <= nEnd; var++)
        {
	        if ((size = set_dist(pCubeContext, p, pCubeSettings->ppVar_Mask[var])) >= 2)
            {
		        if ((expansion *= size) > 1000000)
                {
                    assert(0);
		            //fatal("unreasonable expansion in unravel");
                }
            }
        }
	    total_size += expansion;
    }

    /* We can now allocate a cover of exactly the correct size */
    B1 = MNew_Cover(pCubeContext, total_size, pCubeSettings);
    MForeach_Set(B, last, p)
    {
	    cb_unravel(pCubeContext, p, nStart, nEnd, startbase, B1);
    }

    MFree_Cover(pCubeContext, B);

    return B1;
}


TPCover unravel(
  TPCubeContext pCubeContext,
  const TPCover B,
  TEspressoInt32 nStart)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

    return unravel_range(pCubeContext, B, nStart, pCubeSettings->nNum_Vars-1);
}

/*
 *  cubelist_partition -- take a cubelist T and see if it has any components;
 *  if so, return cubelist's of the two partitions A and B; the return value
 *  is the size of the partition; if not, A and B
 *  are undefined and the return value is 0
 */
TEspressoInt32 cubelist_partition(
  TPCubeContext pCubeContext,
  TPCube *T,			/* a list of cubes */
  TPCube **A,		/* cubelist of partition and remainder */
  TPCube **B)		/* cubelist of partition and remainder */
{
    register TPCube *T1, p, seed, cof;
    TPCube *A1, *B1;
    TEspressoBool change;
    TEspressoInt32 count, numcube;

    numcube = MCubeListSize(T);

    /* Mark all cubes -- covered cubes belong to the partition */
    for(T1 = T+2; (p = *T1++) != NULL; )
    {
    	MReset(p, CEspressoCovered);
    }

    /*
     *  Extract a partition from the cubelist T; start with the first cube as a
     *  seed, and then pull in all cubes which share a variable with the seed;
     *  iterate until no new cubes are brought into the partition.
     */
    seed = MSet_Save(T[2]);
    cof = T[0];
    MSet(T[2], CEspressoCovered);
    count = 1;

    do
    {
	    change = CEspressoFalse;
	    for(T1 = T+2; (p = *T1++) != NULL; )
        {
	        if (! MTestP(p, CEspressoCovered) && ccommon(pCubeContext, p, seed, cof))
            {
    		    MInlineSet_And(seed, seed, p);
	    	    MSet(p, CEspressoCovered);
		        change = CEspressoTrue;
    		    count++;
	        }
	    }
    } while (change);

    MSet_Free(seed);

    if (count != numcube)
    {
	    /* Allocate and setup the cubelist's for the two partitions */
	    *A = A1 = (TPCube*)UtAllocMem(sizeof(TPCube) * numcube+3);
	    *B = B1 = (TPCube*)UtAllocMem(sizeof(TPCube) * numcube+3);
	    (*A)[0] = MSet_Save(T[0]);
	    (*B)[0] = MSet_Save(T[0]);
	    A1 = *A + 2;
	    B1 = *B + 2;

	    /* Loop over the cubes in T and distribute to A and B */
	    for(T1 = T+2; (p = *T1++) != NULL; )
        {
	        if (MTestP(p, CEspressoCovered))
            {
	    	    *A1++ = p;
	        }
            else
            {
    		    *B1++ = p;
	        }
	    }

	    /* Stuff needed at the end of the cubelist's */
	    *A1++ = NULL;
	    (*A)[1] = (TPCube) A1;
	    *B1++ = NULL;
	    (*B)[1] = (TPCube) B1;
    }

    return numcube - count;
}

/*
 *  quick cofactor against a single output function
 */
TPCover cof_output(
  TPCubeContext pCubeContext,
  TPCover T,
  register TEspressoInt32 i)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCover T1;
    register TPCube p, last, pdest, mask;

    mask = pCubeSettings->ppVar_Mask[pCubeSettings->nOutput];
    T1 = MNew_Cover(pCubeContext, T->nCount, pCubeSettings);
    MForeach_Set(T, last, p)
    {
	    if (MIs_In_Set(p, i))
        {
	        pdest = MGetSet(T1, T1->nCount++);
	        MInlineSet_Or(pdest, p, mask);
	        MReset(pdest, CEspressoPrime);
	    }
    }
    return T1;
}

TPCover random_order(
  register TPCover F)
{
    TPSet temp;
    register TEspressoInt32 i, k;
#ifdef RANDOM
    long random();
#endif

    temp = MSet_New(F->nSF_size);
    for(i = F->nCount - 1; i > 0; i--)
    {
	    /* Choose a random number between 0 and i */
#ifdef RANDOM
    	k = random() % i;
#else
	    /* this is not meant to be really used; just provides an easy
	       "out" if random() and srandom() aren't around
	    */
	    k = (i*23 + 997) % i;
#endif
	    /* swap sets i and k */
	    set_copy(temp, MGetSet(F, k));
	    set_copy(MGetSet(F, k), MGetSet(F, i));
	    set_copy(MGetSet(F, i), temp);
    }

    MSet_Free(temp);

    return F;
}

/*  mini_sort -- sort cubes according to the heuristics of mini */
TPCover mini_sort(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCompareFunc pCompareFunc)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 *count, cnt, n = pCubeSettings->nSize, i;
    register TPCube p, last;
    TPCover F_sorted;
    TPCube *F1;

    /* Perform a column sum over the set family */
    count = sf_count(F);

    /* weight is "inner product of the cube and the column sums" */
    MForeach_Set(F, last, p)
    {
	    cnt = 0;
	    for(i = 0; i < n; i++)
        {
	        if (MIs_In_Set(p, i))
            {
    		    cnt += count[i];
            }
        }
	    MPutSize(p, cnt);
    }

    if (count)
    {
        UtFreeMem(count);
        count = NULL;
    }

    /* use qsort to sort the array */
    qsort_s((F1 = sf_list(F)), F->nCount, sizeof(TPCube), pCompareFunc, pCubeContext);
    F_sorted = sf_unlist(pCubeContext, F1, F->nCount, F->nSF_size);
    MFree_Cover(pCubeContext, F);

    return F_sorted;
}

/* sort_reduce -- Espresso strategy for ordering the cubes before reduction */
TPCover sort_reduce(
  TPCubeContext pCubeContext,
  const TPCover T)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, last, largest = NULL;
    register TEspressoInt32 bestsize = -1, size, n = pCubeSettings->nNum_Vars;
    TPCover T_sorted;
    TPCube *T1;

    if (T->nCount == 0)
	    return T;

    /* find largest cube */
    MForeach_Set(T, last, p)
	    if ((size = set_ord(pCubeContext, p)) > bestsize)
        {
	        largest = p, bestsize = size;
        }

    MForeach_Set(T, last, p)
	    MPutSize(p, ((n - cdist(pCubeContext, largest,p)) << 7) + MEspressoMin(set_ord(pCubeContext, p),127));

    qsort_s((T1 = sf_list(T)), T->nCount, sizeof(TPCube), descend, pCubeContext);
    T_sorted = sf_unlist(pCubeContext, T1, T->nCount, T->nSF_size);
    MFree_Cover(pCubeContext, T);

    return T_sorted;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
