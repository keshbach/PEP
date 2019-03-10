/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include <UtilsDevice/UtEspresso.h>
#include <UtilsDevice/UtPALDefs.h>

#include "espresso_defs.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_contain.h"
#include "espresso_compl.h"
#include "espresso_cofactor.h"
#include "espresso_cvrin.h"
#include "espresso_opo.h"
#include "espresso_pair.h"
#include "espresso_cvrmisc.h"
#include "espresso_cvrm.h"
#include "espresso_expand.h"
#include "espresso_sparse.h"
#include "espresso_irred.h"
#include "espresso_essen.h"
#include "espresso_reduce.h"
#include "espresso_gasp.h"
#include "espresso_verify.h"
#include "espresso_cubestr.h"
#include "espresso_hack.h"

/* Table for efficient bit counting */
static TEspressoInt32 l_nDef_Bit_Count_Values[CBitCountSize] = {
  0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

static BOOL lCubeSetup(
  TPCubeContext pCubeContext,
  TEspressoInt32 nOutputBit)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TEspressoInt32 i, var;
    register TPCube p;

    if (pCubeSettings->nNum_Binary_Vars < 0 ||
        pCubeSettings->nNum_Vars < pCubeSettings->nNum_Binary_Vars)
    {
        return FALSE;
    }

    pCubeSettings->pnPart_Size[pCubeSettings->nNum_Vars - 1] = nOutputBit;

    pCubeSettings->nNum_MV_Vars = pCubeSettings->nNum_Vars - pCubeSettings->nNum_Binary_Vars;
    pCubeSettings->nOutput = pCubeSettings->nNum_MV_Vars > 0 ? pCubeSettings->nNum_Vars - 1 : -1;

    pCubeSettings->nSize = 0;
    pCubeSettings->pnFirst_Part = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnLast_Part = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnFirst_Word = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnLast_Word = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);

    for (var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    if (var < pCubeSettings->nNum_Binary_Vars)
        {
	        pCubeSettings->pnPart_Size[var] = 2;
        }

	    pCubeSettings->pnFirst_Part[var] = pCubeSettings->nSize;
	    pCubeSettings->pnFirst_Word[var] = MWhich_Word(pCubeSettings->nSize);
	    pCubeSettings->nSize += MEspressoAbs(pCubeSettings->pnPart_Size[var]);
	    pCubeSettings->pnLast_Part[var] = pCubeSettings->nSize - 1;
	    pCubeSettings->pnLast_Word[var] = MWhich_Word(pCubeSettings->nSize - 1);
    }

    pCubeSettings->ppVar_Mask = (TPSet*)UtAllocMem(sizeof(TPSet) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnSparse = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pBinary_Mask = MNew_Cube(pCubeSettings);
    pCubeSettings->pMV_Mask = MNew_Cube(pCubeSettings);

    for (var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
    	p = pCubeSettings->ppVar_Mask[var] = MNew_Cube(pCubeSettings);

	    for (i = pCubeSettings->pnFirst_Part[var]; i <= pCubeSettings->pnLast_Part[var]; i++)
        {
	        MSet_Insert(p, i);
        }

        if (var < pCubeSettings->nNum_Binary_Vars)
        {
	        MInlineSet_Or(pCubeSettings->pBinary_Mask,
                          pCubeSettings->pBinary_Mask, p);
	        pCubeSettings->pnSparse[var] = 0;
	    }
        else
        {
	        MInlineSet_Or(pCubeSettings->pMV_Mask,
                          pCubeSettings->pMV_Mask, p);
	        pCubeSettings->pnSparse[var] = 1;
	    }
    }

    if (pCubeSettings->nNum_Binary_Vars == 0)
    {
	    pCubeSettings->nInWord = -1;
    }
    else
    {
	    pCubeSettings->nInWord = pCubeSettings->pnLast_Word[pCubeSettings->nNum_Binary_Vars - 1];
	    pCubeSettings->nInMask = pCubeSettings->pBinary_Mask[pCubeSettings->nInWord] & CDisjoint;
    }

    pCubeSettings->ppTemp = (TPSet*)UtAllocMem(sizeof(TPSet) * CCube_Temp);

    for (i = 0; i < CCube_Temp; i++)
    {
	    pCubeSettings->ppTemp[i] = MNew_Cube(pCubeSettings);
    }

    pCubeSettings->pFullSet = set_fill(MNew_Cube(pCubeSettings), pCubeSettings->nSize);
    pCubeSettings->pEmptySet = MNew_Cube(pCubeSettings);

    pCubeData->pnPart_Zeros = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nSize);
    pCubeData->pnVar_Zeros = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeData->pnParts_Active = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeData->pIs_Unate = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);

    return TRUE;
}

static BOOL lCubeCleanup(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

    /* cleanup all used memory */
    free_PLA(pCubeContext, PLA);

    if (pCubeSettings->pnPart_Size)
    {
        UtFreeMem(pCubeSettings->pnPart_Size);
    }

    setdown_cube(pCubeContext); /* free the cube/cdata structure data */
    sf_cleanup(pCubeContext);   /* free unused set structures */
    sm_cleanup();               /* sparse matrix cleanup */

    return TRUE;
}

static void lReadCube(
  LPCWSTR pszTerm,
  TEspressoInt32 nOutputBit,
  TPCubeSettings pCubeStruct,
  TPPLA PLA)
{
    register TEspressoInt32 var, i;
    TPCube cf = pCubeStruct->ppTemp[0], cr = pCubeStruct->ppTemp[1], cd = pCubeStruct->ppTemp[2];
    TEspressoBool savef = CEspressoFalse;
    TEspressoBool saved = CEspressoFalse;
    TEspressoBool saver = CEspressoFalse;

    set_clear(cf, pCubeStruct->nSize);

    for (var = 0; var < pCubeStruct->nNum_Binary_Vars; var++)
    {
        switch (pszTerm[var])
        {
    	    case CPALTermLow:
	        	MSet_Insert(cf, var*2);
		        break;
	        case CPALTermHigh:
		        MSet_Insert(cf, var*2+1);
		        break;
            default:
                assert(0);
                break;
        }
    }

    /* Loop for last multiple-valued variable */
	set_copy(cr, cf);
    set_copy(cd, cf);

    for (i = pCubeStruct->pnFirst_Part[var]; i <= pCubeStruct->pnLast_Part[var]; i++)
    {
	    switch (nOutputBit)
        {
            case 4:
            case 1:
		        if (PLA->nPLA_Type & F_type)
                {
		            MSet_Insert(cf, i), savef = CEspressoTrue;
                }
		        break;
	        case 3:
            case 0:
		        if (PLA->nPLA_Type & R_type)
                {
		            MSet_Insert(cr, i), saver = CEspressoTrue;
                }
		        break;
	        case 2:
		        if (PLA->nPLA_Type & D_type)
                {
		            MSet_Insert(cd, i), saved = CEspressoTrue;
                }
                break;
	    }
    }

    if (savef == CEspressoTrue)
    {
        PLA->pF = sf_addset(PLA->pF, cf);
    }

    if (saved == CEspressoTrue)
    {
        PLA->pD = sf_addset(PLA->pD, cd);
    }

    if (saver == CEspressoTrue)
    {
        PLA->pR = sf_addset(PLA->pR, cr);
    }
}

static TPCover lEspresso(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D1,
  TPCover R)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCover E, D, Fsave;
    TPSet last, p;
    TCost cost, best_cost;

begin:
    Fsave = sf_save(pCubeContext, F);		/* save original function */
    D = sf_save(pCubeContext, D1);		/* make a scratch copy of D */

    /* Setup has always been a problem */
    cover_cost(pCubeContext, F, &cost);
    if ((pCubeSettings->pnPart_Size[pCubeSettings->nNum_Vars - 1] > 1) &&
           (cost.nOut != cost.nCubes*pCubeSettings->pnPart_Size[pCubeSettings->nNum_Vars-1]) &&
           (cost.nOut < 5000))
    {
	    F = sf_contain(pCubeContext, unravel(pCubeContext, F, pCubeSettings->nNum_Vars - 1));
    }

    /* Initial expand and irredundant */
    MForeach_Set(F, last, p)
    {
    	MReset(p, CEspressoPrime);
    }

    F = expand(pCubeContext, F, R, CEspressoFalse);
    cover_cost(pCubeContext, F, &cost);

    F = irredundant(pCubeContext, F, D);
    cover_cost(pCubeContext, F, &cost);

    if (!pCubeContext->nSingle_Expand)
    {
	    if (pCubeContext->nRemove_Essential) 
        {
	        E = essential(pCubeContext, &F, &D);
            cover_cost(pCubeContext, E, &cost);
	    }
        else
        {
	        E = MNew_Cover(pCubeContext, 0, pCubeSettings);
	    }

	    cover_cost(pCubeContext, F, &cost);
	    do
        {
	        /* Repeat inner loop until solution becomes "stable" */
	        do
            {
		        copy_cost(&cost, &best_cost);

		        F = reduce(pCubeContext, F, D);
                cover_cost(pCubeContext, F, &cost);

		        F = expand(pCubeContext, F, R, CEspressoFalse);
                cover_cost(pCubeContext, F, &cost);

		        F = irredundant(pCubeContext, F, D);
                cover_cost(pCubeContext, F, &cost);
	        } while (cost.nCubes < best_cost.nCubes);

	        /* Perturb solution to see if we can continue to iterate */
	        copy_cost(&cost, &best_cost);
	        if (pCubeContext->nUse_Super_Gasp)
            {
    		    /*F = super_gasp(F, D, R, &cost);
	    	    if (cost.cubes >= best_cost.cubes)
                {
		            break;
                }*/
	        }
            else
            {
    		    F = last_gasp(pCubeContext, F, D, R, &cost);
	        }
	    } while (cost.nCubes < best_cost.nCubes ||
     	        (cost.nCubes == best_cost.nCubes && cost.nTotal < best_cost.nTotal));

	    /* Append the essential cubes to F */
	    F = sf_append(pCubeContext, F, E);                /* disposes of E */
    }

    /* Free the D which we used */
    MFree_Cover(pCubeContext, D);

    /* Attempt to make the PLA matrix sparse */
    if (! pCubeContext->nSkip_Make_Sparse)
    {
    	F = make_sparse(pCubeContext, F, D1, R);
    }

    /*
     *  Check to make sure function is actually smaller !!
     *  This can only happen because of the initial unravel.  If we fail,
     *  then run the whole thing again without the unravel.
     */
    if (Fsave->nCount < F->nCount)
    {
	    MFree_Cover(pCubeContext, F);
	    F = Fsave;
	    pCubeContext->nUnwrap_Onset = FALSE;
	    goto begin;
    }
    else
    {
    	MFree_Cover(pCubeContext, Fsave);
    }

    return F;
}

static LPWSTR lBuildTermFromCube(
  TPCubeSettings pCubeSettings,
  TPCube pCube)
{
    static WCHAR cOutput[] = {
        CPALTermUnknown,
        CPALTermLow,
        CPALTermHigh,
        CPALTermUnused};
    TEspressoInt32 var;
    LPWSTR pszTerm;

    pszTerm = (LPWSTR)UtAllocMem(sizeof(WCHAR) * (pCubeSettings->nNum_Binary_Vars + 1));

    for(var = 0; var < pCubeSettings->nNum_Binary_Vars; var++) 
    {
        pszTerm[var] = cOutput[MGetInput(pCube, var)];
    }

    pszTerm[var] = 0;

    return pszTerm;
}

LPWSTR* UTESPRESSOAPI Espresso_AllocMinimizedTerms(
  LPCWSTR* ppszTerms)
{
    TCubeContext CubeContext;
    TPPLA PLA;
    TEspressoInt32 nIndex, i;
    TPCover Fold;
    TCost cost;
    TEspressoBool error = CEspressoFalse;
    LPWSTR* ppszNewTerms = NULL;

    CopyMemory(CubeContext.nBit_Count, l_nDef_Bit_Count_Values, sizeof(l_nDef_Bit_Count_Values));

    CubeContext.CubeSettings.nNum_Binary_Vars = lstrlenW(ppszTerms[0]);
    CubeContext.CubeSettings.nNum_Vars = CubeContext.CubeSettings.nNum_Binary_Vars + 1;
    CubeContext.CubeSettings.pnPart_Size = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * CubeContext.CubeSettings.nNum_Vars);
    CubeContext.use_random_order = CEspressoFalse;
    CubeContext.nToggle = CEspressoTrue;
    CubeContext.pSet_Family_Garbage = NULL;
    CubeContext.nForce_Irredundant = CEspressoTrue;
    CubeContext.nSingle_Expand = 0;
    CubeContext.nRemove_Essential = 1;	/* default -e: */
    CubeContext.nUse_Super_Gasp = 0;
    CubeContext.nSkip_Make_Sparse = 0; // assume this is the default value
    CubeContext.nUnwrap_Onset = 1;
    CubeContext.Use_Random_Order = CEspressoFalse;

    lCubeSetup(&CubeContext, 1); /* must specify 1 as 0 will not work */

    /* Allocate and initialize the PLA structure */
    PLA = new_PLA();
    PLA->nPLA_Type = FD_type;

    PLA_labels(&CubeContext, PLA);

    if (PLA->pF == NULL) 
    {
        TPCubeSettings pCubeStruct = &CubeContext.CubeSettings;

		PLA->pF = MNew_Cover(&CubeContext, 10, pCubeStruct);
		PLA->pD = MNew_Cover(&CubeContext, 10, pCubeStruct);
		PLA->pR = MNew_Cover(&CubeContext, 10, pCubeStruct);
    }

    for (nIndex = 0; ppszTerms[nIndex] != NULL; ++nIndex)
    {
        lReadCube(ppszTerms[nIndex], 1, &CubeContext.CubeSettings, PLA);
    }

    /* This hack merges the next-state field with the outputs */
    for (i = 0; i < CubeContext.CubeSettings.nNum_Vars; i++) 
    {
    	CubeContext.CubeSettings.pnPart_Size[i] = MEspressoAbs(CubeContext.CubeSettings.pnPart_Size[i]);
    }

    /* Decide how to break PLA into ON-set, OFF-set and DC-set */

    if ((PLA->nPLA_Type==F_type || PLA->nPLA_Type==FD_type))
    {
	    MFree_Cover(&CubeContext, PLA->pR);

	    PLA->pR = complement(&CubeContext, cube2list(&CubeContext, PLA->pF, PLA->pD));
    } 
    else
    {
        assert(0);
    }

    /* Check for phase rearrangement of the functions */
    if (PLA->pPhase != NULL)
    {
	    set_phase(&CubeContext, PLA);
    }

    /* Setup minimization for two-bit decoders */
    if (PLA->pPair != NULL)
    {
	    set_pair(&CubeContext, PLA);
    }

    if (PLA->pSymbolic != NULL) 
    {
	    map_symbolic(&CubeContext, PLA);
    }

    if (PLA->pSymbolic_Output != NULL)
    {
	    map_output_symbolic(&CubeContext, PLA);
	    //if (needs_offset)
        //{
	    //    free_cover(PLA->R);
        //    EXECUTE(PLA->R=complement(cube2list(PLA->F,PLA->D)), COMPL_TIME, PLA->R, cost);
	    //}
    }

	Fold = sf_save(&CubeContext, PLA->pF);
	PLA->pF = lEspresso(&CubeContext, PLA->pF, PLA->pD, PLA->pR);

	error = verify(&CubeContext, PLA->pF,Fold,PLA->pD);
    cover_cost(&CubeContext, PLA->pF, &cost);

	if (error)
    {
	    //print_solution = FALSE;
	    PLA->pF = Fold;
	    check_consistency(&CubeContext, PLA);
	}
    else
    {
	    MFree_Cover(&CubeContext, Fold);
	}

    /* Output the solution */
    {
        register TPCube last, p;
        TEspressoInt32 nTermCount = 0;
        TEspressoInt32 nTermIndex = 0;

	    MForeach_Set(PLA->pF, last, p) 
        {
		    ++nTermCount;
	    }

        ppszNewTerms = (LPWSTR*)UtAllocMem(sizeof(LPWSTR) * (nTermCount + 1 ));

	    MForeach_Set(PLA->pF, last, p) 
        {
            ppszNewTerms[nTermIndex] = lBuildTermFromCube(&CubeContext.CubeSettings, p);

            ++nTermIndex;
	    }

        ppszNewTerms[nTermIndex] = NULL;

        cover_cost(&CubeContext, PLA->pF, &cost);
    }

    /* Crash and burn if there was a verify error */
    if (error)
    {
        assert(0);
    	//fatal("cover verification failed");
    }

    lCubeCleanup(&CubeContext, PLA);

    return ppszNewTerms;
}

VOID UTESPRESSOAPI Espresso_FreeMinimizedTerms(
  LPWSTR* ppszTerms)
{
    TEspressoInt32 nIndex = 0;

    while (ppszTerms[nIndex] != NULL)
    {
        UtFreeMem(ppszTerms[nIndex]);

        ++nIndex;
    }

    UtFreeMem(ppszTerms);
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
