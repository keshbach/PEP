/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cvrmisc.h"
#include "espresso_cofactor.h"

/* cost -- compute the cost of a cover */
void cover_cost(
  TPCubeContext pCubeContext,
  const TPCover F,
  TPCost cost)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TPCube p, last;
    TPCube *T;
    TEspressoInt32 var;

    /* use the routine used by cofactor to decide splitting variables */
    massive_count(pCubeContext, T = cube1list(pCubeContext, F));
    MFree_Cubelist(T);

    cost->nCubes = F->nCount;
    cost->nTotal = cost->nIn = cost->nOut = cost->nMv = cost->nPrimes = 0;

    /* Count transistors (zeros) for each binary variable (inputs) */
    for(var = 0; var < pCubeSettings->nNum_Binary_Vars; var++)
    {
	    cost->nIn += pCubeData->pnVar_Zeros[var];
    }

    /* Count transistors for each mv variable based on sparse/dense */
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars - 1; var++)
    {
	    if (pCubeSettings->pnSparse[var])
        {
	        cost->nMv += F->nCount * pCubeSettings->pnPart_Size[var] - pCubeData->pnVar_Zeros[var];
        }
	    else
        {
	        cost->nMv += pCubeData->pnVar_Zeros[var];
        }
    }

    /* Count the transistors (ones) for the output variable */
    if (pCubeSettings->nNum_Binary_Vars != pCubeSettings->nNum_Vars)
    {
	    var = pCubeSettings->nNum_Vars - 1;
	    cost->nOut = F->nCount * pCubeSettings->pnPart_Size[var] - pCubeData->pnVar_Zeros[var];
    }

    /* Count the number of nonprime cubes */
    MForeach_Set(F, last, p)
	    cost->nPrimes += MTestP(p, CEspressoPrime) != 0;

    /* Count the total number of literals */
    cost->nTotal = cost->nIn + cost->nOut + cost->nMv;
}

/* copy_cost -- copy a cost function from s to d */
void copy_cost(
  const TPCost s,
  TPCost d)
{
    d->nCubes = s->nCubes;
    d->nIn = s->nIn;
    d->nOut = s->nOut;
    d->nMv = s->nMv;
    d->nTotal = s->nTotal;
    d->nPrimes = s->nPrimes;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
