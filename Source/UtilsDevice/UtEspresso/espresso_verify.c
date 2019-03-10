/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_verify.h"
#include "espresso_cofactor.h"
#include "espresso_sparse.h"
#include "espresso_irred.h"
#include "espresso_sharp.h"
#include "espresso_set.h"
#include "espresso_compl.h"

/*
 *  verify -- check that all minterms of F are contained in (Fold u Dold)
 *  and that all minterms of Fold are contained in (F u Dold).
 */
TEspressoBool verify(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover Fold,
  TPCover Dold)
{
    TPCube p, last, *FD;
    TEspressoBool verify_error = CEspressoFalse;

    /* Make sure the function didn't grow too large */
    FD = cube2list(pCubeContext, Fold, Dold);
    MForeach_Set(F, last, p)
	if (! cube_is_covered(pCubeContext, FD, p))
    {
	    //printf("some minterm in F is not covered by Fold u Dold\n");
	    verify_error = CEspressoTrue;
	    //if (verbose_debug)
        //    printf("%s\n", pc1(p));
        //else
            break;
	}
    MFree_Cubelist(FD);

    /* Make sure minimized function covers the original function */
    FD = cube2list(pCubeContext, F, Dold);
    MForeach_Set(Fold, last, p)
	if (! cube_is_covered(pCubeContext, FD, p))
    {
	    //printf("some minterm in Fold is not covered by F u Dold\n");
	    verify_error = CEspressoTrue;
	    //if (verbose_debug)
        //    printf("%s\n", pc1(p));
        //else
            break;
	}
    MFree_Cubelist(FD);

    return verify_error;
}

/*
 *  check_consistency -- test that the ON-set, OFF-set and DC-set form
 *  a partition of the boolean space.
 */
TEspressoBool check_consistency(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TEspressoBool verify_error = CEspressoFalse;
    TPCover T;

    T = cv_intersect(pCubeContext, PLA->pF, PLA->pD);
    if (T->nCount == 0)
    {
	    //printf("ON-SET and DC-SET are disjoint\n");
    }
    else
    {
	    //printf("Some minterm(s) belong to both the ON-SET and DC-SET !\n");
	    //if (verbose_debug)
	    //    cprint(T);
	    verify_error = CEspressoTrue;
    }

    MFree_Cover(pCubeContext, T);

    T = cv_intersect(pCubeContext, PLA->pF, PLA->pR);
    if (T->nCount == 0)
    {
	    //printf("ON-SET and OFF-SET are disjoint\n");
    }
    else
    {
	    //printf("Some minterm(s) belong to both the ON-SET and OFF-SET !\n");
	    //if (verbose_debug)
        //{
	    //    cprint(T);
        //}
	    verify_error = CEspressoTrue;
    }

    MFree_Cover(pCubeContext, T);

    T = cv_intersect(pCubeContext, PLA->pD, PLA->pR);
    if (T->nCount == 0)
    {
	    //printf("DC-SET and OFF-SET are disjoint\n");
    }
    else
    {
	    //printf("Some minterm(s) belong to both the OFF-SET and DC-SET !\n");
	    //if (verbose_debug)
        //{
	    //    cprint(T);
        //}
	    verify_error = CEspressoTrue;
    }

    MFree_Cover(pCubeContext, T);

    if (tautology(pCubeContext, cube3list(pCubeContext, PLA->pF, PLA->pD, PLA->pR)))
    {
    	//printf("Union of ON-SET, OFF-SET and DC-SET is the universe\n");
    }
    else
    {
	    T = complement(pCubeContext, cube3list(pCubeContext, PLA->pF, PLA->pD, PLA->pR));
	    //printf("There are minterms left unspecified !\n");
	    //if (verbose_debug)
        //{
	    //    cprint(T);
        //}
	    verify_error = CEspressoTrue;
	    MFree_Cover(pCubeContext, T);
    }

    return verify_error;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
