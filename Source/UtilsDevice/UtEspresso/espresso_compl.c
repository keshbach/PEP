/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_compl.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_cofactor.h"
#include "espresso_unate.h"
#include "espresso_contain.h"
#include "espresso_cofactor.h"

#define CUse_Compl_Lift 0
#define CUse_Compl_Lift_Onset 1
#define CUse_Compl_Lift_Onset_Complex 2
#define CNo_Lifting 3

/*
 *  compl_d1merge -- distance-1 merge in the splitting variable
 */
void compl_d1merge(
  TPCubeContext pCubeContext,
  register TPCube *L1,
  register TPCube *R1)
{
    register TPCube pl, pr;

    /* Find equal cubes between the two cofactors */
    for (pl = *L1, pr = *R1; (pl != NULL) && (pr != NULL); )
    {
        switch (d1_order(pCubeContext, L1, R1))
        {
	        case 1:
    		    pr = *(++R1);
                break;            /* advance right pointer */
	        case -1:
	    	    pl = *(++L1);
                break;            /* advance left pointer */
	        case 0:
		        MReset(pr, CEspressoActive);
		        MInlineSet_Or(pl, pl, pr);
		        pr = *(++R1);
                break;
	    }
    }
}

/*
 *  compl_lift_simple -- expand in the splitting variable using single
 *  cube containment against the other recursion branch to check
 *  validity of the expansion, and expanding all (or none) of the
 *  splitting variable.
 */
void compl_lift(
  TPCubeContext pCubeContext,
  TPCube *A1,
  TPCube *B1,
  TPCube bcube,
  TEspressoInt32 var)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube a, b, *B2, lift=pCubeSettings->ppTemp[4], liftor=pCubeSettings->ppTemp[5];
    TPCube mask = pCubeSettings->ppVar_Mask[var];

    set_and(liftor, bcube, mask);

    /* for each cube in the first array ... */
    for(; (a = *A1++) != NULL; )
    {
	    if (MTestP(a, CEspressoActive)) 
        {
	        /* create a lift of this cube in the merging coord */
	        set_merge(lift, bcube, a, mask);

	        /* for each cube in the second array */
	        for(B2 = B1; (b = *B2++) != NULL; )
            {
		        MInlineSetP_Implies(lift, b, /* when_false => */ continue);
		        /* when_true => fall through to next statement */

		        /* cube of A1 was contained by some cube of B1, so raise */
		        MInlineSet_Or(a, a, liftor);
		        break;
	        }
	    }
    }
}

/*
 *  compl_lift_onset -- expand in the splitting variable using a
 *  distance-1 check against the original on-set; expand all (or
 *  none) of the splitting variable.  Each cube of A1 is expanded
 *  against the original on-set T.
 */
void compl_lift_onset(
  TPCubeContext pCubeContext,
  TPCube *A1,
  TPCover T,
  TPCube bcube,
  TEspressoInt32 var)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube a, last, p, lift=pCubeSettings->ppTemp[4], mask=pCubeSettings->ppVar_Mask[var];

    /* for each active cube from one branch of the complement */
    for(; (a = *A1++) != NULL; )
    {
	if (MTestP(a, CEspressoActive))
    {
	    /* create a lift of this cube in the merging coord */
	    MInlineSet_And(lift, bcube, mask);	/* isolate parts to raise */
	    MInlineSet_Or(lift, a, lift);	/* raise these parts in a */

	    /* for each cube in the ON-set, check for intersection */
	    MForeach_Set(T, last, p)
        {
		    if (cdist0(pCubeContext, p, lift))
            {
		        goto nolift;
		    }
	    }
	    MInlineSet_Copy(a, lift);		/* save the raising */
	    MSet(a, CEspressoActive);
nolift : ;
	}
    }
}

/*
 *  compl_lift_complex -- expand in the splitting variable, but expand all
 *  parts which can possibly expand.
 *  T is the original ON-set
 *  A1 is either the left or right cofactor
 */
void compl_lift_onset_complex(
  TPCubeContext pCubeContext,
  TPCube *A1,			/* array of pointers to new result */
  TPCover T,			/* original ON-set */
  TEspressoInt32 var)			/* which variable we split on */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 dist;
    register TPCube last, p, a, xlower;

    /* for each cube in the complement */
    xlower = MNew_Cube(pCubeSettings);
    for(; (a = *A1++) != NULL; ) 
    {
	    if (MTestP(a, CEspressoActive)) 
        {
	        /* Find which parts of the splitting variable are forced low */
	        MInlineSet_Clear(xlower, pCubeSettings->nSize);
	        MForeach_Set(T, last, p) 
            {
		        if ((dist = cdist01(pCubeContext, p, a)) < 2) 
                {
		            if (dist == 0)
                    {
                        assert(0);
			            /*fatal("compl: ON-set and OFF-set are not orthogonal");*/
		            }
                    else
                    {
			            force_lower(pCubeContext, xlower, p, a);
		            }
		        }
	        }

	        set_diff(xlower, pCubeSettings->ppVar_Mask[var], xlower);
	        set_or(a, a, xlower);
	        MFree_Cube(xlower);
	    }
    }
}

/* compl_cube -- return the complement of a single cube (De Morgan's law) */
TPCover compl_cube(
  TPCubeContext pCubeContext,
  register TPCube p)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube diff = pCubeSettings->ppTemp[7], pdest, mask, full=pCubeSettings->pFullSet;
    TEspressoInt32 var;
    TPCover R;

    /* Allocate worst-case size cover (to avoid checking overflow) */
    R = MNew_Cover(pCubeContext, pCubeSettings->nNum_Vars, pCubeSettings);

    /* Compute bit-wise complement of the cube */
    MInlineSet_Diff(diff, full, p);

    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var];
	    /* If the bit-wise complement is not empty in var ... */
	    if (! setp_disjoint(diff, mask))
        {
	        pdest = MGetSet(R, R->nCount++);
	        MInlineSet_Merge(pdest, diff, full, mask);
	    }
    }
    return R;
}

TEspressoBool compl_special_cases(
  TPCubeContext pCubeContext,
  TPCube *T,			/* will be disposed if answer is determined */
  TPCover *Tbar)			/* returned only if answer determined */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TPCube *T1, p, ceil, cof=T[0];
    TPCover A, ceil_compl;

    /* Check for no cubes in the cover */
    if (T[2] == NULL)
    {
	    *Tbar = sf_addset(MNew_Cover(pCubeContext, 1, pCubeSettings), pCubeSettings->pFullSet);
	    MFree_Cubelist(T);
	    return CEspressoTrue;
    }

    /* Check for only a single cube in the cover */
    if (T[3] == NULL) 
    {
	    *Tbar = compl_cube(pCubeContext, set_or(cof, cof, T[2]));
	    MFree_Cubelist(T);
	    return CEspressoTrue;
    }

    /* Check for a row of all 1's (implies complement is null) */
    for(T1 = T+2; (p = *T1++) != NULL; ) 
    {
	    if (full_row(pCubeContext, p, cof))
        {
	        *Tbar = MNew_Cover(pCubeContext, 0, pCubeSettings);
	        MFree_Cubelist(T);
	        return CEspressoTrue;
	    }
    }

    /* Check for a column of all 0's which can be factored out */
    ceil = MSet_Save(cof);
    for(T1 = T+2; (p = *T1++) != NULL; )
    {
    	MInlineSet_Or(ceil, ceil, p);
    }
    if (! setp_equal(ceil, pCubeSettings->pFullSet))
    {
	    ceil_compl = compl_cube(pCubeContext, ceil);
	    set_or(cof, cof, set_diff(ceil, pCubeSettings->pFullSet, ceil));
	    MSet_Free(ceil);
	    *Tbar = sf_append(pCubeContext, complement(pCubeContext, T), ceil_compl);
	    return CEspressoTrue;
    }
    MSet_Free(ceil);

    /* Collect column counts, determine unate variables, etc. */
    massive_count(pCubeContext, T);

    /* If single active variable not factored out above, then tautology ! */
    if (pCubeData->nVars_Active == 1)
    {
	    *Tbar = MNew_Cover(pCubeContext, 0, pCubeSettings);
	    MFree_Cubelist(T);
	    return CEspressoTrue;

    /* Check for unate cover */
    }
    else if (pCubeData->nVars_Unate == pCubeData->nVars_Active)
    {
	    A = map_cover_to_unate(pCubeContext, T);
	    MFree_Cubelist(T);
	    A = unate_compl(pCubeContext, A);
	    *Tbar = map_unate_to_cover(pCubeContext, A);
	    sf_free(pCubeContext, A);
	    return CEspressoTrue;

    /* Not much we can do about it */
    }
    else
    {
	    return CEspressoMaybe;
    }
}

/*
 *  compl_merge -- merge the two cofactors around the splitting
 *  variable
 *
 *  The merge operation involves intersecting each cube of the left
 *  cofactor with cl, and intersecting each cube of the right cofactor
 *  with cr.  The union of these two covers is the merged result.
 *
 *  In order to reduce the number of cubes, a distance-1 merge is
 *  performed (note that two cubes can only combine distance-1 in the
 *  splitting variable).  Also, a simple expand is performed in the
 *  splitting variable (simple implies the covering check for the
 *  expansion is not full containment, but single-cube containment).
 */

TPCover compl_merge(
  TPCubeContext pCubeContext,
  TPCube *T1,			/* Original ON-set */
  TPCover L,			/* Complement from each recursion branch */
  TPCover R,			/* Complement from each recursion branch */
  register TPCube cl,	/* cubes used for cofactoring */
  register TPCube cr,	/* cubes used for cofactoring */
  TEspressoInt32 var,			/* splitting variable */
  TEspressoInt32 lifting)			/* whether to perform lifting or not */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, last, pt;
    TPCover T, Tbar;
    TPCube *L1, *R1;
    
    /* Intersect each cube with the cofactored cube */
    MForeach_Set(L, last, p)
    {
	    MInlineSet_And(p, p, cl);
	    MSet(p, CEspressoActive);
    }
    MForeach_Set(R, last, p)
    {
	    MInlineSet_And(p, p, cr);
	    MSet(p, CEspressoActive);
    }

    /* Sort the arrays for a distance-1 merge */
    set_copy(pCubeSettings->ppTemp[0], pCubeSettings->ppVar_Mask[var]);
    qsort_s((L1 = sf_list(L)), L->nCount, sizeof(TPSet), d1_order, pCubeContext);
    qsort_s((R1 = sf_list(R)), R->nCount, sizeof(TPSet), d1_order, pCubeContext);

    /* Perform distance-1 merge */
    compl_d1merge(pCubeContext, L1, R1);

    /* Perform lifting */
    switch(lifting)
    {
	    case CUse_Compl_Lift_Onset:
	        T = cubeunlist(pCubeContext, T1);
	        compl_lift_onset(pCubeContext, L1, T, cr, var);
	        compl_lift_onset(pCubeContext, R1, T, cl, var);
	        MFree_Cover(pCubeContext, T);
	        break;
	    case CUse_Compl_Lift_Onset_Complex:
	        T = cubeunlist(pCubeContext, T1);
	        compl_lift_onset_complex(pCubeContext, L1, T, var);
	        compl_lift_onset_complex(pCubeContext, R1, T, var);
	        MFree_Cover(pCubeContext, T);
	        break;
	    case CUse_Compl_Lift:
	        compl_lift(pCubeContext, L1, R1, cr, var);
	        compl_lift(pCubeContext, R1, L1, cl, var);
	        break;
	    case CNo_Lifting:
	        break;
    }

    if (L1)
    {
        UtFreeMem(L1);
        L1 = NULL;
    }

    if (R1)
    {
        UtFreeMem(R1);
        R1 = NULL;
    }

    /* Re-create the merged cover */
    Tbar = MNew_Cover(pCubeContext, L->nCount + R->nCount, pCubeSettings);
    pt = Tbar->pData;
    MForeach_Set(L, last, p)
    {
    	MInlineSet_Copy(pt, p);
	    Tbar->nCount++;
	    pt += Tbar->nWSize;
    }
    MForeach_Active_Set(R, last, p)
    {
	    MInlineSet_Copy(pt, p);
	    Tbar->nCount++;
	    pt += Tbar->nWSize;
    }

    MFree_Cover(pCubeContext, L);
    MFree_Cover(pCubeContext, R);

    return Tbar;
}

/* complement -- compute the complement of T */
TPCover complement(
  TPCubeContext pCubeContext,
  TPCube *T)			/* T will be disposed of */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube cl, cr;
    register TEspressoInt32 best;
    TPCover Tbar, Tl, Tr;
    TEspressoInt32 lifting;

    if (compl_special_cases(pCubeContext, T, &Tbar) == CEspressoMaybe)
    {
	    /* Allocate space for the partition cubes */
	    cl = MNew_Cube(pCubeSettings);
	    cr = MNew_Cube(pCubeSettings);
	    best = binate_split_select(pCubeContext, T, cl, cr);

	    /* Complement the left and right halves */
	    Tl = complement(pCubeContext, scofactor(pCubeContext, T, cl, best));
	    Tr = complement(pCubeContext, scofactor(pCubeContext, T, cr, best));

	    if (Tr->nCount*Tl->nCount > (Tr->nCount+Tl->nCount)*MCubeListSize(T))
        {
	        lifting = CUse_Compl_Lift_Onset;
	    }
        else
        {
	        lifting = CUse_Compl_Lift;
	    }

	    Tbar = compl_merge(pCubeContext, T, Tl, Tr, cl, cr, best, lifting);

	    MFree_Cube(cl);
	    MFree_Cube(cr);
	    MFree_Cubelist(T);
    }

    return Tbar;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
