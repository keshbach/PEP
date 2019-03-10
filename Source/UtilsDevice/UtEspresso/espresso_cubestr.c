/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cubestr.h"
#include "espresso_set.h"

/*
    cube_setup -- assume that the fields "num_vars", "num_binary_vars", and
    part_size[num_binary_vars .. num_vars-1] are setup, and initialize the
    rest of cube and cdata.

    If a part_size is < 0, then the field size is abs(part_size) and the
    field read from the input is symbolic.
*/
void cube_setup(
  TPCubeContext pCubeContext)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TEspressoInt32 i, var;
    register TPCube p;

    if (pCubeSettings->nNum_Binary_Vars < 0 || pCubeSettings->nNum_Vars < pCubeSettings->nNum_Binary_Vars)
    {
	    //fatal("cube size is silly, error in .i/.o or .mv");
        assert(0);
    }

    pCubeSettings->nNum_MV_Vars = pCubeSettings->nNum_Vars - pCubeSettings->nNum_Binary_Vars;
    pCubeSettings->nOutput = pCubeSettings->nNum_MV_Vars > 0 ? pCubeSettings->nNum_Vars - 1 : -1;

    pCubeSettings->nSize = 0;
    pCubeSettings->pnFirst_Part = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnLast_Part = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnFirst_Word = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeSettings->pnLast_Word = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
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

    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    p = pCubeSettings->ppVar_Mask[var] = MNew_Cube(pCubeSettings);

	    for(i = pCubeSettings->pnFirst_Part[var]; i <= pCubeSettings->pnLast_Part[var]; i++)
        {
	        MSet_Insert(p, i);
        }

	    if (var < pCubeSettings->nNum_Binary_Vars) 
        {
	        MInlineSet_Or(pCubeSettings->pBinary_Mask, pCubeSettings->pBinary_Mask, p);
	        pCubeSettings->pnSparse[var] = 0;
	    }
        else
        {
	        MInlineSet_Or(pCubeSettings->pMV_Mask, pCubeSettings->pMV_Mask, p);
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

    for(i = 0; i < CCube_Temp; i++)
    {
	    pCubeSettings->ppTemp[i] = MNew_Cube(pCubeSettings);
    }

    pCubeSettings->pFullSet = set_fill(MNew_Cube(pCubeSettings), pCubeSettings->nSize);
    pCubeSettings->pEmptySet = MNew_Cube(pCubeSettings);

    pCubeData->pnPart_Zeros = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nSize);
    pCubeData->pnVar_Zeros = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeData->pnParts_Active = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
    pCubeData->pIs_Unate = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nNum_Vars);
}

/*
    setdown_cube -- free memory allocated for the cube/cdata structs
    (free's all but the part_size array)

    (I wanted to call this cube_setdown, but that violates the 8-character
    external routine limit on the IBM !)
*/
void setdown_cube(
  TPCubeContext pCubeContext)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TEspressoInt32 i, var;

    if (pCubeSettings->pnFirst_Part)
    {
        UtFreeMem(pCubeSettings->pnFirst_Part);
        pCubeSettings->pnFirst_Part = 0;
    }

    if (pCubeSettings->pnLast_Part)
    {
        UtFreeMem(pCubeSettings->pnLast_Part);
        pCubeSettings->pnLast_Part = 0;
    }

    if (pCubeSettings->pnFirst_Word)
    {
        UtFreeMem(pCubeSettings->pnFirst_Word);
        pCubeSettings->pnFirst_Word = 0;
    }

    if (pCubeSettings->pnLast_Word)
    {
        UtFreeMem(pCubeSettings->pnLast_Word);
        pCubeSettings->pnLast_Word = 0;
    }

    if (pCubeSettings->pnSparse)
    {
        UtFreeMem(pCubeSettings->pnSparse);
        pCubeSettings->pnSparse = 0;
    }
    
    MFree_Cube(pCubeSettings->pBinary_Mask);
    MFree_Cube(pCubeSettings->pMV_Mask);
    MFree_Cube(pCubeSettings->pFullSet);
    MFree_Cube(pCubeSettings->pEmptySet);

    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
    	MFree_Cube(pCubeSettings->ppVar_Mask[var]);
    }

    if (pCubeSettings->ppVar_Mask)
    {
        UtFreeMem(pCubeSettings->ppVar_Mask);
        pCubeSettings->ppVar_Mask = 0;
    }

    for(i = 0; i < CCube_Temp; i++)
    {
	    MFree_Cube(pCubeSettings->ppTemp[i]);
    }

    if (pCubeSettings->ppTemp)
    {
        UtFreeMem(pCubeSettings->ppTemp);
        pCubeSettings->ppTemp = 0;
    }

    if (pCubeData->pnPart_Zeros)
    {
        UtFreeMem(pCubeData->pnPart_Zeros);
        pCubeData->pnPart_Zeros = 0;
    }

    if (pCubeData->pnVar_Zeros)
    {
        UtFreeMem(pCubeData->pnVar_Zeros);
        pCubeData->pnVar_Zeros = 0;
    }

    if (pCubeData->pnParts_Active)
    {
        UtFreeMem(pCubeData->pnParts_Active);
        pCubeData->pnParts_Active = 0;
    }

    if (pCubeData->pIs_Unate)
    {
        UtFreeMem(pCubeData->pIs_Unate);
        pCubeData->pIs_Unate = 0;
    }

    pCubeSettings->pnFirst_Part = pCubeSettings->pnLast_Part = NULL;
    pCubeSettings->pnFirst_Word = pCubeSettings->pnLast_Word = NULL;
    pCubeSettings->pnSparse = NULL;
    pCubeSettings->pBinary_Mask = pCubeSettings->pMV_Mask = NULL;
    pCubeSettings->pFullSet = pCubeSettings->pEmptySet = NULL;
    pCubeSettings->ppVar_Mask = pCubeSettings->ppTemp = NULL;

    pCubeData->pnPart_Zeros = pCubeData->pnVar_Zeros = pCubeData->pnParts_Active = NULL;
    pCubeData->pIs_Unate = NULL;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
