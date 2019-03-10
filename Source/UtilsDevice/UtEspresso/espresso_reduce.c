/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

/*
    module: reduce.c
    purpose: Perform the Espresso-II reduction step

    Reduction is a technique used to explore larger regions of the
    optimization space.  We replace each cube of F with a smaller
    cube while still maintaining a cover of the same logic function.
*/
#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_reduce.h"
#include "espresso_cvrm.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_cofactor.h"

/*
    reduce -- replace each cube in F with its reduction

    The reduction of a cube is the smallest cube contained in the cube
    which can replace the cube in the original cover without changing
    the cover.  This is equivalent to the super cube of all of the
    essential points in the cube.  This can be computed directly.

    The problem is that the order in which the cubes are reduced can
    greatly affect the final result.  We alternate between two ordering
    strategies:

	(1) Order the cubes in ascending order of distance from the
	largest cube breaking ties by ordering cubes of equal distance
	in descending order of size (sort_reduce)

	(2) Order the cubes in descending order of the inner-product of
	the cube and the column sums (mini_sort)

    The real workhorse of this section is the routine SCCC which is
    used to find the Smallest Cube Containing the Complement of a cover.
    Reduction as proposed by Espresso-II takes a cube and computes its
    maximal reduction as the intersection between the cube and the
    smallest cube containing the complement of (F u D - {c}) cofactored
    against c.

    As usual, the unate-recursive paradigm is used to compute SCCC.
    The SCCC of a unate cover is trivial to compute, and thus we perform
    Shannon Cofactor expansion attempting to drive the cover to be unate
    as fast as possible.
*/

TPCover reduce(
  TPCubeContext pCubeContext,
  TPCover F,
  const TPCover D)
{
    register TPCube last, p, cunder, *FD;

    /* Order the cubes */
    if (pCubeContext->Use_Random_Order)
    {
    	F = random_order(F);
    }
    else
    {
	    F = pCubeContext->nToggle ? sort_reduce(pCubeContext, F) : mini_sort(pCubeContext, F, descend);
	    pCubeContext->nToggle = ! pCubeContext->nToggle;
    }

    /* Try to reduce each cube */
    FD = cube2list(pCubeContext, F, D);
    MForeach_Set(F, last, p)
    {
	    cunder = reduce_cube(pCubeContext, FD, p);		/* reduce the cube */
	    if (setp_equal(cunder, p))            /* see if it actually did */
        {
	        MSet(p, CEspressoActive);	/* cube remains active */
	        MSet(p, CEspressoPrime);	/* cube remains prime ? */
	    }
        else
        {
	        set_copy(p, cunder);                /* save reduced version */
	        MReset(p, CEspressoPrime);                    /* cube is no longer prime */
	        if (setp_empty(cunder))
            {
		        MReset(p, CEspressoActive);               /* if null, kill the cube */
            }
	        else
            {
		        MSet(p, CEspressoActive);                 /* cube is active */
            }
	    }
	    MFree_Cube(cunder);
    }
    MFree_Cubelist(FD);

    /* Delete any cubes of F which reduced to the empty cube */
    return sf_inactive(F);
}

/* reduce_cube -- find the maximal reduction of a cube */
TPCube reduce_cube(
  TPCubeContext pCubeContext,
  TPCube *FD,
  const TPCube p)
{
    TPCube cunder;

    cunder = sccc(pCubeContext, cofactor(pCubeContext, FD, p));

    return set_and(cunder, cunder, p);
}

/* sccc -- find Smallest Cube Containing the Complement of a cover */
TPCube sccc(
  TPCubeContext pCubeContext,
  TPCube *T)         /* T will be disposed of */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCube r;
    register TPCube cl, cr;
    register TEspressoInt32 best;

    if (sccc_special_cases(pCubeContext, T, &r) == CEspressoMaybe)
    {
	    cl = MNew_Cube(pCubeSettings);
	    cr = MNew_Cube(pCubeSettings);
	    best = binate_split_select(pCubeContext, T, cl, cr);
	    r = sccc_merge(sccc(pCubeContext, scofactor(pCubeContext, T, cl, best)),
                       sccc(pCubeContext, scofactor(pCubeContext, T, cr, best)), cl, cr);

	    MFree_Cubelist(T);
    }

    return r;
}

TPCube sccc_merge(
  register TPCube left,       /* will be disposed of ... */
  register TPCube right,      /* will be disposed of ... */
  register TPCube cl,         /* will be disposed of ... */
  register TPCube cr)         /* will be disposed of ... */
{
    MInlineSet_And(left, left, cl);
    MInlineSet_And(right, right, cr);
    MInlineSet_Or(left, left, right);

    MFree_Cube(right);
    MFree_Cube(cl);
    MFree_Cube(cr);

    return left;
}

/*
    sccc_cube -- find the smallest cube containing the complement of a cube

    By DeMorgan's law and the fact that the smallest cube containing a
    cover is the "or" of the positional cubes, it is simple to see that
    the SCCC is the universe if the cube has more than two active
    variables.  If there is only a single active variable, then the
    SCCC is merely the bitwise complement of the cube in that
    variable.  A last special case is no active variables, in which
    case the SCCC is empty.

    This is "anded" with the incoming cube result.
*/
TPCube sccc_cube(
  TPCubeContext pCubeContext,
  register TPCube result,
  register TPCube p)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube temp=pCubeSettings->ppTemp[0], mask;
    TEspressoInt32 var;

    if ((var = cactive(pCubeContext, p)) >= 0)
    {
	    mask = pCubeSettings->ppVar_Mask[var];
	    MInlineSet_Xor(temp, p, mask);
	    MInlineSet_And(result, result, temp);
    }
    return result;
}

/*
 *   sccc_special_cases -- check the special cases for sccc
 */

TEspressoBool sccc_special_cases(
  TPCubeContext pCubeContext,
  TPCube *T,                 /* will be disposed if answer is determined */
  TPCube *result)              /* returned only if answer determined */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TPCube *T1, p, temp = pCubeSettings->ppTemp[1], ceil, cof = T[0];
    TPCube *A, *B;

    /* empty cover => complement is universe => SCCC is universe */
    if (T[2] == NULL)
    {
	    *result = MSet_Save(pCubeSettings->pFullSet);
	    MFree_Cubelist(T);
	    return CEspressoTrue;
    }

    /* row of 1's => complement is empty => SCCC is empty */
    for(T1 = T+2; (p = *T1++) != NULL; )
    {
	    if (full_row(pCubeContext, p, cof))
        {
	        *result = MNew_Cube(pCubeSettings);

	        MFree_Cubelist(T);

	        return CEspressoTrue;
	    }
    }

    /* Collect column counts, determine unate variables, etc. */
    massive_count(pCubeContext, T);

    /* If cover is unate (or single cube), apply simple rules to find SCCCU */
    if (pCubeData->nVars_Unate == pCubeData->nVars_Active || T[3] == NULL)
    {
	    *result = MSet_Save(pCubeSettings->pFullSet);

	    for(T1 = T+2; (p = *T1++) != NULL; )
        {
	        sccc_cube(pCubeContext, *result, set_or(temp, p, cof));
	    }

	    MFree_Cubelist(T);

	    return CEspressoTrue;
    }

    /* Check for column of 0's (which can be easily factored( */
    ceil = MSet_Save(cof);
    for(T1 = T+2; (p = *T1++) != NULL; ) 
    {
    	MInlineSet_Or(ceil, ceil, p);
    }

    if (! setp_equal(ceil, pCubeSettings->pFullSet))
    {
	    *result = sccc_cube(pCubeContext, MSet_Save(pCubeSettings->pFullSet), ceil);

	    if (setp_equal(*result, pCubeSettings->pFullSet))
        {
	        MFree_Cube(ceil);
	    }
        else
        {
	        *result = sccc_merge(sccc(pCubeContext, cofactor(pCubeContext, T,ceil)),
			                     MSet_Save(pCubeSettings->pFullSet), ceil, *result);
	    }

	    MFree_Cubelist(T);

	    return CEspressoTrue;
    }

    MFree_Cube(ceil);

    /* Single active column at this point => tautology => SCCC is empty */
    if (pCubeData->nVars_Active == 1)
    {
	    *result = MNew_Cube(pCubeSettings);

	    MFree_Cubelist(T);

	    return CEspressoTrue;
    }

    /* Check for components */
    if (pCubeData->pnVar_Zeros[pCubeData->nBest] < MCubeListSize(T)/2)
    {
	    if (cubelist_partition(pCubeContext, T, &A, &B) == 0)
        {
	        return CEspressoMaybe;
	    }
        else
        {
	        MFree_Cubelist(T);

	        *result = sccc(pCubeContext, A);
	        ceil = sccc(pCubeContext, B);

	        set_and(*result, *result, ceil);

	        MSet_Free(ceil);

	        return CEspressoTrue;
	    }
    }

    /* Not much we can do about it */
    return CEspressoMaybe;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
