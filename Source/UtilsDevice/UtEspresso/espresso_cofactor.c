/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cofactor.h"
#include "espresso_set.h"
#include "espresso_setc.h"

/* cofactor -- compute the cofactor of a cover with respect to a cube */
TPCube *cofactor(
  TPCubeContext pCubeContext,
  TPCube* T,
  register const TPCube c)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCube temp = pCubeSettings->ppTemp[0], *Tc_save, *Tc, *T1;
    register TPCube p;
    TEspressoInt32 listlen;

    listlen = MCubeListSize(T) + 5;

    /* Allocate a new list of cube pointers (max size is previous size) */
    Tc_save = Tc = (TPCube*)UtAllocMem(sizeof(TPCube) * listlen);

    /* pass on which variables have been cofactored against */
    *Tc++ = set_or(MNew_Cube(pCubeSettings), T[0], set_diff(temp, pCubeSettings->pFullSet, c));
    Tc++;

    /* Loop for each cube in the list, determine suitability, and save */
    for(T1 = T+2; (p = *T1++) != NULL; ) 
    {
	    if (p != c)
        {
	        if (! cdist0(pCubeContext, p, c))
                goto false;

	        *Tc++ = p;
	        false: ;
	    }
    }

    *Tc++ = NULL;                       /* sentinel */
    Tc_save[1] = (TPCube) Tc;                    /* save pointer to last */
    return Tc_save;
}

/*
    scofactor -- compute the cofactor of a cover with respect to a cube,
    where the cube is "active" in only a single variable.

    This routine has been optimized for speed.
*/

TPCube *scofactor(
  TPCubeContext pCubeContext,
  TPCube *T,
  const TPCube c,
  TEspressoInt32 var)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCube *Tc, *Tc_save;
    register TPCube p, mask = pCubeSettings->ppTemp[1], *T1;
    register TEspressoInt32 first = pCubeSettings->pnFirst_Word[var], last = pCubeSettings->pnLast_Word[var];
    TEspressoInt32 listlen;

    listlen = MCubeListSize(T) + 5;

    /* Allocate a new list of cube pointers (max size is previous size) */
    Tc_save = Tc = (TPCube*)UtAllocMem(sizeof(TPCube) * listlen);

    /* pass on which variables have been cofactored against */
    *Tc++ = set_or(MNew_Cube(pCubeSettings), T[0], set_diff(mask, pCubeSettings->pFullSet, c));
    Tc++;

    /* Setup for the quick distance check */
    set_and(mask, pCubeSettings->ppVar_Mask[var], c);

    /* Loop for each cube in the list, determine suitability, and save */
    for(T1 = T+2; (p = *T1++) != NULL; )
	if (p != c) 
    {
	    register TEspressoInt32 i = first;
	    do
		if (p[i] & mask[i]) {
		    *Tc++ = p;
		    break;
		}
	    while (++i <= last);
	}

    *Tc++ = NULL;                       /* sentinel */
    Tc_save[1] = (TPCube) Tc;                    /* save pointer to last */
    return Tc_save;
}

void massive_count(
  TPCubeContext pCubeContext,
  TPCube *T)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    TEspressoInt32 *count = pCubeData->pnPart_Zeros;
    TPCube *T1;

    /* Clear the column counts (count of # zeros in each column) */
    {
    register TEspressoInt32 i;
    for(i = pCubeSettings->nSize - 1; i >= 0; i--)
	    count[i] = 0;
    }

    /* Count the number of zeros in each column */
 {  register TEspressoInt32 i, *cnt;
    register TEspressoUInt32 val;
    register TPCube p, cof = T[0], full = pCubeSettings->pFullSet;
    for(T1 = T+2; (p = *T1++) != NULL; )
	for(i = MLoop(p); i > 0; i--)
    {
	    val = full[i] & ~ (p[i] | cof[i]);

	    if (val)
        {
		    cnt = count + ((i-1) << CLogBitsPerInteger);
#if CBitsPerInteger == 32
	        if (val & 0xFF000000)
            {
		        if (val & 0x80000000)
                    cnt[31]++;
		        if (val & 0x40000000)
                    cnt[30]++;
		        if (val & 0x20000000)
                    cnt[29]++;
		        if (val & 0x10000000)
                    cnt[28]++;
		        if (val & 0x08000000)
                    cnt[27]++;
		        if (val & 0x04000000)
                    cnt[26]++;
		        if (val & 0x02000000)
                    cnt[25]++;
		        if (val & 0x01000000)
                    cnt[24]++;
    	    }
	        if (val & 0x00FF0000)
            {
		        if (val & 0x00800000)
                    cnt[23]++;
		        if (val & 0x00400000)
                    cnt[22]++;
		        if (val & 0x00200000)
                    cnt[21]++;
		        if (val & 0x00100000)
                    cnt[20]++;
		        if (val & 0x00080000)
                    cnt[19]++;
		        if (val & 0x00040000)
                    cnt[18]++;
		        if (val & 0x00020000)
                    cnt[17]++;
		        if (val & 0x00010000)
                    cnt[16]++;
    	    }
#elif CBitsPerInteger == 16
#else
#error Undefined bits per integer
#endif
	        if (val & 0xFF00)
            {
		        if (val & 0x8000)
                    cnt[15]++;
		        if (val & 0x4000)
                    cnt[14]++;
		        if (val & 0x2000)
                    cnt[13]++;
		        if (val & 0x1000)
                    cnt[12]++;
		        if (val & 0x0800)
                    cnt[11]++;
		        if (val & 0x0400)
                    cnt[10]++;
		        if (val & 0x0200)
                    cnt[ 9]++;
		        if (val & 0x0100)
                    cnt[ 8]++;
	        }
	        if (val & 0x00FF)
            {
		        if (val & 0x0080)
                    cnt[ 7]++;
		        if (val & 0x0040)
                    cnt[ 6]++;
		        if (val & 0x0020)
                    cnt[ 5]++;
		        if (val & 0x0010)
                    cnt[ 4]++;
		        if (val & 0x0008)
                    cnt[ 3]++;
		        if (val & 0x0004)
                    cnt[ 2]++;
		        if (val & 0x0002)
                    cnt[ 1]++;
		        if (val & 0x0001)
                    cnt[ 0]++;
	        }
    	}
    }
 }

    /*
     * Perform counts for each variable:
     *    cdata.var_zeros[var] = number of zeros in the variable
     *    cdata.parts_active[var] = number of active parts for each variable
     *    cdata.vars_active = number of variables which are active
     *    cdata.vars_unate = number of variables which are active and unate
     *
     *    best -- the variable which is best for splitting based on:
     *    mostactive -- most # active parts in any variable
     *    mostzero -- most # zeros in any variable
     *    mostbalanced -- minimum over the maximum # zeros / part / variable
     */

 {  register TEspressoInt32 var, i, lastbit, active, maxactive;
    TEspressoInt32 best = -1, mostactive = 0, mostzero = 0, mostbalanced = 32000;
    pCubeData->nVars_Unate = pCubeData->nVars_Active = 0;

    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	if (var < pCubeSettings->nNum_Binary_Vars) /* special hack for binary vars */
    {
	    i = count[var*2];
	    lastbit = count[var*2 + 1];
	    active = (i > 0) + (lastbit > 0);
	    pCubeData->pnVar_Zeros[var] = i + lastbit;
	    maxactive = MEspressoMax(i, lastbit);
	}
    else
    {
	    maxactive = active = pCubeData->pnVar_Zeros[var] = 0;
	    lastbit = pCubeSettings->pnLast_Part[var];
	    for(i = pCubeSettings->pnFirst_Part[var]; i <= lastbit; i++)
        {
		    pCubeData->pnVar_Zeros[var] += count[i];
		    active += (count[i] > 0);
		    if (active > maxactive)
            {
                maxactive = active;
            }
	    }
	}

	/* first priority is to maximize the number of active parts */
	/* for binary case, this will usually select the output first */
	if (active > mostactive)
    {
	    best = var, mostactive = active, mostzero = pCubeData->pnVar_Zeros[best],
	    mostbalanced = maxactive;
    }
	else if (active == mostactive)
    {
	    /* secondary condition is to maximize the number zeros */
	    /* for binary variables, this is the same as minimum # of 2's */
	    if (pCubeData->pnVar_Zeros[var] > mostzero)
        {
	  	    best = var, mostzero = pCubeData->pnVar_Zeros[best],
		    mostbalanced = maxactive;
        }
	    else if (pCubeData->pnVar_Zeros[var] == mostzero)
        {
		    /* third condition is to pick a balanced variable */
		    /* for binary vars, this means roughly equal # 0's and 1's */
		    if (maxactive < mostbalanced)
            {
		        best = var, mostbalanced = maxactive;
            }
        }
    }

	pCubeData->pnParts_Active[var] = active;
	pCubeData->pIs_Unate[var] = (active == 1);
	pCubeData->nVars_Active += (active > 0);
	pCubeData->nVars_Unate += (active == 1);
    }
    pCubeData->nBest = best;
 }
}

TPCover cubeunlist(
  TPCubeContext pCubeContext,
  TPCube *A1)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 i;
    register TPCube p, pdest, cof = A1[0];
    register TPCover A;

    A = MNew_Cover(pCubeContext, MCubeListSize(A1), pCubeSettings);

    for(i = 2; (p = A1[i]) != NULL; i++)
    {
	    pdest = MGetSet(A, i-2);
	    MInlineSet_Or(pdest, p, cof);
    }

    A->nCount = MCubeListSize(A1);

    return A;
}

TPCube *cube2list(
  TPCubeContext pCubeContext,
  TPCover A,
  TPCover B)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p, *plist, *list;

    list = plist = (TPCube*)UtAllocMem(sizeof(TPCube) * (A->nCount + B->nCount + 3));
    *plist++ = MNew_Cube(pCubeSettings);
    plist++;
    MForeach_Set(A, last, p)
    {
	    *plist++ = p;
    }
    MForeach_Set(B, last, p) 
    {
    	*plist++ = p;
    }
    *plist++ = NULL;
    list[1] = (TPCube) plist;
    return list;
}

TPCube *cube3list(
  TPCubeContext pCubeContext,
  TPCover A,
  TPCover B,
  TPCover C)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p, *plist, *list;

    plist = (TPCube*)UtAllocMem(sizeof(TPCube) * (A->nCount + B->nCount + C->nCount + 3));
    list = plist;
    *plist++ = MNew_Cube(pCubeSettings);
    plist++;
    MForeach_Set(A, last, p)
    {
    	*plist++ = p;
    }
    MForeach_Set(B, last, p)
    {
    	*plist++ = p;
    }
    MForeach_Set(C, last, p)
    {
	    *plist++ = p;
    }
    *plist++ = NULL;
    list[1] = (TPCube) plist;
    return list;
}

TEspressoInt32 binate_split_select(
  TPCubeContext pCubeContext,
  /*IN*/ TPCube *T,
  /*IN*/ register TPCube cleft,
  /*IN*/ register TPCube cright)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    TEspressoInt32 best = pCubeData->nBest;
    register TEspressoInt32 i, lastbit = pCubeSettings->pnLast_Part[best], halfbit = 0;
    register TPCube cof=T[0];

    /* Create the cubes to cofactor against */
    set_diff(cleft, pCubeSettings->pFullSet, pCubeSettings->ppVar_Mask[best]);
    set_diff(cright, pCubeSettings->pFullSet, pCubeSettings->ppVar_Mask[best]);

    for(i = pCubeSettings->pnFirst_Part[best]; i <= lastbit; i++)
    {
	    if (! MIs_In_Set(cof,i))
        {
	        halfbit++;
        }
    }

    for(i = pCubeSettings->pnFirst_Part[best], halfbit = halfbit/2; halfbit > 0; i++)
    {
	    if (! MIs_In_Set(cof,i))
        {
	        halfbit--, MSet_Insert(cleft, i);
        }
    }

    for(; i <= lastbit; i++)
    {
	    if (! MIs_In_Set(cof,i))
        {
	        MSet_Insert(cright, i);
        }
    }

    return best;
}

TPCube* cube1list(
  TPCubeContext pCubeContext,
  TPCover A)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p, *plist, *list;

    list = plist = (TPCube*)UtAllocMem(sizeof(TPCube) * (A->nCount + 3));
    *plist++ = MNew_Cube(pCubeSettings);
    plist++;
    MForeach_Set(A, last, p)
    {
	    *plist++ = p;
    }
    *plist++ = NULL;                    /* sentinel */
    list[1] = (TPCube) plist;
    return list;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
