/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_contain.h"
#include "espresso_set.h"
#include "espresso_setc.h"

/* d1_rm_equal -- distance-1 merge (merge cubes which are equal under a mask) */
TEspressoInt32 d1_rm_equal(
  register TPSet *A1,			/* array of set pointers */
  TPCompareFunc pCompareFunc)
{
    register TEspressoInt32 i, j, dest;

    dest = 0;
    if (A1[0] != NULL)
    {
	    for(i = 0, j = 1; A1[j] != NULL; j++)
	        if ( (*pCompareFunc)(NULL, &A1[i], &A1[j]) == 0)
            {
		        /* if sets are equal (under the mask) merge them */
		        set_or(A1[i], A1[i], A1[j]);
	        }
            else
            {
		        /* sets are unequal, so save the set i */
		        A1[dest++] = A1[i];
		        i = j;
	        }
	    A1[dest++] = A1[i];
    }
    A1[dest] = NULL;
    return dest;
}

/* rm_equal -- scan a sorted array of set pointers for duplicate sets */
TEspressoInt32 rm_equal(
  TPSet *A1,                 /* updated in place */
  TPCompareFunc pCompareFunc)
{
    register TPSet *p, *pdest = A1;

    if (*A1 != NULL)                  /* If more than one set */
    {
	    for (p = A1+1; *p != NULL; p++)
        {
	        if ((*pCompareFunc)(NULL, p, p-1) != 0)
            {
		        *pdest++ = *(p-1);
            }
        }

        *pdest++ = *(p-1);
	    *pdest = NULL;
    }

    return (TEspressoInt32)(pdest - A1);
}

/* rm_contain -- perform containment over a sorted array of set pointers */
TEspressoInt32 rm_contain(
  TPSet *A1)                 /* updated in place */
{
    register TPSet *pa, *pb, *pcheck, a, b;
    TPSet *pdest = A1;
    TEspressoInt32 last_size = -1;

    pcheck = NULL;

    /* Loop for all cubes of A1 */
    for(pa = A1; (a = *pa++) != NULL; )
    {
	    /* Update the check pointer if the size has changed */
	    if ((TEspressoInt32)MSize(a) != last_size)
        {
	        last_size = MSize(a), pcheck = pdest;
        }

	    for(pb = A1; pb != pcheck; )
        {
	        b = *pb++;
	        MInlineSetP_Implies(a, b, /* when_false => */ continue);
	        goto lnext1;
	    }
	    /* set a was not contained by some larger set, so save it */
	    *pdest++ = a;
        lnext1: ;
    }

    *pdest = NULL;

    return (TEspressoInt32)(pdest - A1);
}

/* rm_rev_contain -- perform rcontainment over a sorted array of set pointers */
TEspressoInt32 rm_rev_contain(
  TPSet *A1)                 /* updated in place */
{
    register TPSet *pa, *pb, *pcheck, a, b;
    TPSet *pdest = A1;
    TEspressoInt32 last_size = -1;

    pcheck = NULL;

    /* Loop for all cubes of A1 */
    for(pa = A1; (a = *pa++) != NULL; ) 
    {
	    /* Update the check pointer if the size has changed */
	    if ((TEspressoInt32)MSize(a) != last_size)
        {
	        last_size = MSize(a), pcheck = pdest;
        }

	    for (pb = A1; pb != pcheck; )
        {
	        b = *pb++;
	        MInlineSetP_Implies(b, a, /* when_false => */ continue);
	        goto lnext1;
	    }
	    /* the set a did not contain some smaller set, so save it */
	    *pdest++ = a;
        lnext1: ;
    }

    *pdest = NULL;

    return (TEspressoInt32)(pdest - A1);
}

/* rm2_equal -- check two sorted arrays of set pointers for equal cubes */
TEspressoInt32 rm2_equal(
  register TPSet *A1,           /* updated in place */
  register TPSet *B1,           /* updated in place */
  TPSet *E1,
  TPCompareFunc pCompareFunc)
{
    register TPSet *pda = A1, *pdb = B1, *pde = E1;

    /* Walk through the arrays advancing pointer to larger cube */
    for(; *A1 != NULL && *B1 != NULL; )
    	switch((*pCompareFunc)(NULL, A1, B1))
        {
	        case -1:    /* "a" comes before "b" */
		        *pda++ = *A1++;
                break;
	        case 0:     /* equal cubes */
		        *pde++ = *A1++; B1++;
                break;
	        case 1:     /* "a" is to follow "b" */
		        *pdb++ = *B1++;
                break;
	    }

    /* Finish moving down the pointers of A and B */
    while (*A1 != NULL)
    {
    	*pda++ = *A1++;
    }

    while (*B1 != NULL)
    {
	    *pdb++ = *B1++;
    }

    *pda = *pdb = *pde = NULL;

    return (TEspressoInt32)(pde - E1);
}

/* rm2_contain -- perform containment between two arrays of set pointers */
TEspressoInt32 rm2_contain(
  TPSet *A1,                 /* updated in place */
  TPSet *B1)                    /* unchanged */
{
    register TPSet *pa, *pb, a, b, *pdest = A1;

    /* for each set in the first array ... */
    for(pa = A1; (a = *pa++) != NULL; ) 
    {
	    /* for each set in the second array which is larger ... */
	    for(pb = B1; (b = *pb++) != NULL && MSize(b) > MSize(a); ) 
        {
	        MInlineSetP_Implies(a, b, /* when_false => */ continue);
	        /* set was contained in some set of B, so don't save pointer */
	        goto lnext1;
	    }
	    /* set wasn't contained in any set of B, so save the pointer */
	    *pdest++ = a;
        lnext1: ;
    }

    *pdest = NULL;                      /* sentinel */

    return (TEspressoInt32)(pdest - A1); /* # elements in A1 */
}

/*
    sf_contain -- perform containment on a set family (delete sets which
    are contained by some larger set in the family).  No assumptions are
    made about A, and the result will be returned in decreasing order of
    set size.
*/
TPSet_Family sf_contain(
  TPCubeContext pCubeContext,
  TPSet_Family A)            /* disposes of A */
{
    TEspressoInt32 cnt;
    TPSet *A1;
    TPSet_Family R;

    A1 = sf_sort(pCubeContext, A, descend); /* sort into descending order */
    cnt = rm_equal(A1, descend);        /* remove duplicates */
    cnt = rm_contain(A1);               /* remove contained sets */
    R = sf_unlist(pCubeContext, A1, cnt, A->nSF_size); /* recreate the set family */
    sf_free(pCubeContext, A);
    return R;
}

/* sf_sort -- sort the sets of A */
TPSet *sf_sort(
  TPCubeContext pCubeContext,
  TPSet_Family A,
  TPCompareFunc pCompareFunc)
{
    register TPSet p, last, *pdest, *A1;

    /* Create a single array pointing to each cube of A */
    pdest = A1 = (TPSet*)UtAllocMem(sizeof(TPSet) * (A->nCount + 1));
    MForeach_Set(A, last, p)
    {
	    MPutSize(p, set_ord(pCubeContext, p));         /* compute the set size */
	    *pdest++ = p;                   /* save the pointer */
    }
    *pdest = NULL;                      /* Sentinel -- never seen by sort */

    /* Sort cubes by size */
    qsort_s(A1, A->nCount, sizeof(TPSet), pCompareFunc, NULL);

    return A1;
}

/* sf_list -- make a list of pointers to the sets in a set family */
TPSet *sf_list(
  register const TPSet_Family A)
{
    register TPSet p, last, *pdest, *A1;

    /* Create a single array pointing to each cube of A */
    pdest = A1 = (TPSet*)UtAllocMem(sizeof(TPSet) * (A->nCount + 1));
    MForeach_Set(A, last, p)
    	*pdest++ = p;                   /* save the pointer */
    *pdest = NULL;                      /* Sentinel */
    return A1;
}

/* sf_unlist -- make a set family out of a list of pointers to sets */
TPSet_Family sf_unlist(
  TPCubeContext pCubeContext,
  TPSet *A1,
  TEspressoInt32 nTotCnt,
  TEspressoInt32 nSize)
{
    register TPSet pr, p, *pa;
    TPSet_Family R = sf_new(pCubeContext, nTotCnt, nSize);

    R->nCount = nTotCnt;

    for (pr = R->pData, pa = A1; (p = *pa++) != NULL; pr += R->nWSize)
    	MInlineSet_Copy(pr, p);

    if (A1)
    {
        UtFreeMem(A1);
        A1 = NULL;
    }

    return R;
}

/*
    sf_rev_contain -- perform containment on a set family (delete sets which
    contain some smaller set in the family).  No assumptions are made about
    A, and the result will be returned in increasing order of set size
*/

TPSet_Family sf_rev_contain(
  TPCubeContext pCubeContext,
  TPSet_Family A)            /* disposes of A */
{
    TEspressoInt32 cnt;
    TPSet *A1;
    TPSet_Family R;

    A1 = sf_sort(pCubeContext, A, ascend); /* sort into ascending order */
    cnt = rm_equal(A1, ascend);         /* remove duplicates */
    cnt = rm_rev_contain(A1);           /* remove containing sets */
    R = sf_unlist(pCubeContext, A1, cnt, A->nSF_size); /* recreate the set family */

    sf_free(pCubeContext, A);

    return R;
}

/*
    sf_union -- form the contained union of two set families (delete
    sets which are contained by some larger set in the family).  A and
    B are assumed already sorted in decreasing order of set size (and
    the SIZE field is assumed to contain the set size), and the result
    will be returned sorted likewise.
*/
TPSet_Family sf_union(
  TPCubeContext pCubeContext,
  TPSet_Family A,         /* disposes of A */
  TPSet_Family B)         /* disposes of B */
{
    TEspressoInt32 cnt;
    TPSet_Family R;
    TPSet *A1 = sf_list(A), *B1 = sf_list(B), *E1;

    E1 = (TPSet*)UtAllocMem(sizeof(TPSet) * (MEspressoMax(A->nCount, B->nCount) + 1));
    cnt = rm2_equal(A1, B1, E1, descend);
    cnt += rm2_contain(A1, B1) + rm2_contain(B1, A1);
    R = sf_merge(pCubeContext, A1, B1, E1, cnt, A->nSF_size);

    sf_free(pCubeContext, A);
    sf_free(pCubeContext, B);

    return R;
}

/* sf_merge -- merge three sorted lists of set pointers */
TPSet_Family sf_merge(
  TPCubeContext pCubeContext,
  TPSet *A1,               /* will be disposed of */
  TPSet *B1,               /* will be disposed of */
  TPSet *E1,               /* will be disposed of */
  TEspressoInt32 nTotCnt,
  TEspressoInt32 nSize)
{
    register TPSet pr, ps, *pmin, *pmid, *pmax;
    TPSet_Family R;
    TPSet *temp[3], *swap;
    TEspressoInt32 i, j, n;

    /* Allocate the result set_family */
    R = sf_new(pCubeContext, nTotCnt, nSize);
    R->nCount = nTotCnt;
    pr = R->pData;

    /* Quick bubble sort to order the top member of the three arrays */
    n = 3;  temp[0] = A1;  temp[1] = B1;  temp[2] = E1;
    for(i = 0; i < n-1; i++)
    {
	    for(j = i+1; j < n; j++)
        {
	        if (desc1(*temp[i], *temp[j]) > 0)
            {
	    	    swap = temp[j];
    		    temp[j] = temp[i];
		        temp[i] = swap;
	        }
        }
    }

    pmin = temp[0];  pmid = temp[1];  pmax = temp[2];

    /* Save the minimum element, then update pmin, pmid, pmax */
    while (*pmin != NULL)
    {
	    ps = *pmin++;
	    MInlineSet_Copy(pr, ps);
	    pr += R->nWSize;
	    if (desc1(*pmin, *pmax) > 0) 
        {
	        swap = pmax; pmax = pmin; pmin = pmid; pmid = swap;
	    }
        else if (desc1(*pmin, *pmid) > 0)
        {
	        swap = pmin; pmin = pmid; pmid = swap;
	    }
    }

    if (A1)
    {
        UtFreeMem(A1);
        A1 = NULL;
    }

    if (B1)
    {
        UtFreeMem(B1);
        B1 = NULL;
    }

    if (E1)
    {
        UtFreeMem(E1);
        E1 = NULL;
    }

    return R;
}

/* sf_dupl -- delete duplicate sets in a set family */
TPSet_Family sf_dupl(
  TPCubeContext pCubeContext,
  TPSet_Family A)            /* disposes of A */
{
    register TEspressoInt32 cnt;
    register TPSet *A1;
    TPSet_Family R;

    A1 = sf_sort(pCubeContext, A, descend);           /* sort the set family */
    cnt = rm_equal(A1, descend);        /* remove duplicates */
    R = sf_unlist(pCubeContext, A1, cnt, A->nSF_size); /* recreate the set family */

    sf_free(pCubeContext, A);

    return R;
}

/*
    dist_merge -- consider all sets to be "or"-ed with "mask" and then
    delete duplicates from the set family.
*/
TPSet_Family dist_merge(
  TPCubeContext pCubeContext,
  TPSet_Family A,            /* disposes of A */
  const TPSet mask)                   /* defines variables to mask out */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPSet *A1;
    TEspressoInt32 cnt;
    TPSet_Family R;

    set_copy(pCubeSettings->ppTemp[0], mask);

    A1 = sf_sort(pCubeContext, A, d1_order);
    cnt = d1_rm_equal(A1, d1_order);
    R = sf_unlist(pCubeContext, A1, cnt, A->nSF_size);

    sf_free(pCubeContext, A);

    return R;
}

/*
    d1merge -- perform an efficient distance-1 merge of cubes of A
*/
TPSet_Family d1merge(
  TPCubeContext pCubeContext,
  TPSet_Family A,            /* disposes of A */
  TEspressoInt32 nVar)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

    return dist_merge(pCubeContext, A, pCubeSettings->ppVar_Mask[nVar]);
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
