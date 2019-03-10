/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_pair.h"
#include "espresso_cvrout.h"
#include "espresso_cubestr.h"
#include "espresso_set.h"

/*
 *  util_strsav -- save a copy of a string
 */
static LPTSTR util_strsav(
  LPCTSTR s)
{
    INT nNewLen = lstrlen(s) + 1;
    LPTSTR pszNew = (LPTSTR)UtAllocMem(sizeof(TCHAR) * nNewLen);

    StringCchCopy(pszNew, nNewLen, s);

    return pszNew;
}

void set_pair(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    set_pair1(pCubeContext, PLA, CEspressoTrue);
}

void set_pair1(
  TPCubeContext pCubeContext,
  TPPLA PLA,
  TEspressoBool adjust_labels)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 i, var, *paired, newvar;
    TEspressoInt32 old_num_vars, old_num_binary_vars, old_size, old_mv_start;
    TEspressoInt32 *new_part_size, new_num_vars, new_num_binary_vars, new_mv_start;
    TPPair pair = PLA->pPair;
    TCHAR scratch[200], **oldlabel, *var1, *var1bar, *var2, *var2bar;

    if (adjust_labels)
    {
	    makeup_labels(pCubeContext, PLA);
    }

    /* Check the pair structure for valid entries and see which binary
       variables are left unpaired
    */

    paired = (TEspressoBool*)UtAllocMem(sizeof(TEspressoBool) * pCubeSettings->nNum_Binary_Vars);

    for(var = 0; var < pCubeSettings->nNum_Binary_Vars; var++)
    {
	    paired[var] = CEspressoFalse;
    }

    for(i = 0; i < pair->nCnt; i++)
    {
	    if ((pair->pnVar1[i] > 0 && pair->pnVar1[i] <= pCubeSettings->nNum_Binary_Vars) &&
	        (pair->pnVar2[i] > 0 && pair->pnVar2[i] <= pCubeSettings->nNum_Binary_Vars))
        {
	        paired[pair->pnVar1[i]-1] = CEspressoTrue;
	        paired[pair->pnVar2[i]-1] = CEspressoTrue;
	    } 
        else
        {
            assert(0);
	        //fatal("can only pair binary-valued variables");
        }
    }

    PLA->pF = delvar(pCubeContext, pairvar(pCubeContext, PLA->pF, pair), paired);
    PLA->pR = delvar(pCubeContext, pairvar(pCubeContext, PLA->pR, pair), paired);
    PLA->pD = delvar(pCubeContext, pairvar(pCubeContext, PLA->pD, pair), paired);

    /* Now painfully adjust the cube size */
    old_size = pCubeSettings->nSize;
    old_num_vars = pCubeSettings->nNum_Vars;
    old_num_binary_vars = pCubeSettings->nNum_Binary_Vars;
    old_mv_start = pCubeSettings->pnFirst_Part[pCubeSettings->nNum_Binary_Vars];
    /* Create the new cube.part_size vector and setup the cube structure */
    new_num_binary_vars = 0;
    for(var = 0; var < old_num_binary_vars; var++)
    {
	    new_num_binary_vars += (paired[var] == CEspressoFalse);
    }
    
    new_num_vars = new_num_binary_vars + pair->nCnt;
    new_num_vars += old_num_vars - old_num_binary_vars;
    new_part_size = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * new_num_vars);
    for(var = 0; var < pair->nCnt; var++)
    {
	    new_part_size[new_num_binary_vars + var] = 4;
    }
    for(var = 0; var < old_num_vars - old_num_binary_vars; var++)
    {
	    new_part_size[new_num_binary_vars + pair->nCnt + var] =
	        pCubeSettings->pnPart_Size[old_num_binary_vars + var];
    }
    setdown_cube(pCubeContext);

    if (pCubeSettings->pnPart_Size)
    {
        UtFreeMem(pCubeSettings->pnPart_Size);
        pCubeSettings->pnPart_Size = 0;
    }

    pCubeSettings->nNum_Vars = new_num_vars;
    pCubeSettings->nNum_Binary_Vars = new_num_binary_vars;
    pCubeSettings->pnPart_Size = new_part_size;
    cube_setup(pCubeContext);

    /* hack with the labels to get them correct */
    if (adjust_labels)
    {
	    oldlabel = PLA->ppszLabel;
	    PLA->ppszLabel = (LPTSTR*)UtAllocMem(sizeof(LPTSTR) * pCubeSettings->nSize);
	    for (var = 0; var < pair->nCnt; var++) 
        {
	        newvar = pCubeSettings->nNum_Binary_Vars*2 + var*4;
	        var1 = oldlabel[ (pair->pnVar1[var]-1) * 2 + 1];
	        var2 = oldlabel[ (pair->pnVar2[var]-1) * 2 + 1];
	        var1bar = oldlabel[ (pair->pnVar1[var]-1) * 2];
	        var2bar = oldlabel[ (pair->pnVar2[var]-1) * 2];

            StringCchPrintf(scratch, sizeof(scratch) / sizeof(scratch[0]),
                            TEXT("%s+%s"), var1bar, var2bar);
	        PLA->ppszLabel[newvar] = util_strsav(scratch);

	        StringCchPrintf(scratch, sizeof(scratch) / sizeof(scratch[0]),
                            TEXT("%s+%s"), var1bar, var2);
	        PLA->ppszLabel[newvar+1] = util_strsav(scratch);

	        StringCchPrintf(scratch, sizeof(scratch) / sizeof(scratch[0]),
                            TEXT("%s+%s"), var1, var2bar);
	        PLA->ppszLabel[newvar+2] = util_strsav(scratch);

	        StringCchPrintf(scratch, sizeof(scratch) / sizeof(scratch[0]),
                            TEXT("%s+%s"), var1, var2);
	        PLA->ppszLabel[newvar+3] = util_strsav(scratch);
	    }
	    /* Copy the old labels for the unpaired binary vars */
	    i = 0;
	    for(var = 0; var < old_num_binary_vars; var++)
        {
	        if (paired[var] == CEspressoFalse) 
            {
		        PLA->ppszLabel[2*i] = oldlabel[2*var];
		        PLA->ppszLabel[2*i+1] = oldlabel[2*var+1];
		        oldlabel[2*var] = oldlabel[2*var+1] = NULL;
		        i++;
	        }
	    }
	    /* Copy the old labels for the remaining unpaired vars */
	    new_mv_start = pCubeSettings->nNum_Binary_Vars*2 + pair->nCnt*4;
	    for(i = old_mv_start; i < old_size; i++)
        {
	        PLA->ppszLabel[new_mv_start + i - old_mv_start] = oldlabel[i];
	        oldlabel[i] = NULL;
	    }
	    /* free remaining entries in oldlabel */
	    for(i = 0; i < old_size; i++)
        {
	        if (oldlabel[i] != NULL)
            {
                if (oldlabel[i])
                {
                    UtFreeMem(oldlabel[i]);
                    oldlabel[i] = 0;
                }
            }
        }

        if (oldlabel)
        {
            UtFreeMem(oldlabel);
            oldlabel = 0;
        }
    }

    /* the paired variables should not be sparse (cf. mv_reduce/raise_in)*/
    for(var = 0; var < pair->nCnt; var++)
    {
	   pCubeSettings->pnSparse[pCubeSettings->nNum_Binary_Vars + var] = 0;
    }

    if (paired)
    {
        UtFreeMem(paired);
        paired = 0;
    }
}

TPCover pairvar(
  TPCubeContext pCubeContext,
  TPCover A,
  TPPair pair)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p;
    register TEspressoInt32 val, p1, p2, b1, b0;
    TEspressoInt32 insert_col, pairnum;

    insert_col = pCubeSettings->pnFirst_Part[pCubeSettings->nNum_Vars - 1];

    /* stretch the cover matrix to make room for the paired variables */
    A = sf_delcol(pCubeContext, A, insert_col, -4*pair->nCnt);

    /* compute the paired values */
    MForeach_Set(A, last, p)
    {
	    for(pairnum = 0; pairnum < pair->nCnt; pairnum++)
        {
	        p1 = pCubeSettings->pnFirst_Part[pair->pnVar1[pairnum] - 1];
	        p2 = pCubeSettings->pnFirst_Part[pair->pnVar2[pairnum] - 1];
	        b1 = MIs_In_Set(p, p2+1);
	        b0 = MIs_In_Set(p, p2);
	        val = insert_col + pairnum * 4;

	        if (/* a0 */ MIs_In_Set(p, p1))
            {
		        if (b0)
                {
		            MSet_Insert(p, val + 3);
                }

		        if (b1)
                {
		            MSet_Insert(p, val + 2);
                }
	        }

	        if (/* a1 */ MIs_In_Set(p, p1+1))
            {
		        if (b0)
                {
		            MSet_Insert(p, val + 1);
                }

		        if (b1)
                {
		            MSet_Insert(p, val);
                }
	        }
	    }
    }
    return A;
}

/* delvar -- delete variables from A, minimize the number of column shifts */
TPCover delvar(
  TPCubeContext pCubeContext,
  TPCover A,
  TEspressoBool paired[])
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoBool run;
    TEspressoInt32 first_run, run_length, var, offset = 0;

    first_run = 0;

    run = CEspressoFalse; run_length = 0;
    for(var = 0; var < pCubeSettings->nNum_Binary_Vars; var++)
    {
	    if (paired[var])
        {
	        if (run)
            {
		        run_length += pCubeSettings->pnPart_Size[var];
            }
	        else 
            {
		        run = CEspressoTrue;
		        first_run = pCubeSettings->pnFirst_Part[var];
		        run_length = pCubeSettings->pnPart_Size[var];
	        }
        }
	    else
        {
	        if (run)
            {
		        A = sf_delcol(pCubeContext, A, first_run-offset, run_length);
		        run = CEspressoFalse;
		        offset += run_length;
	        }
        }
    }

    if (run)
    {
    	A = sf_delcol(pCubeContext, A, first_run-offset, run_length);
    }

    return A;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
