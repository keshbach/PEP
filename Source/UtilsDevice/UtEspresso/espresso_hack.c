/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_hack.h"
#include "espresso_set.h"
#include "espresso_cubestr.h"
#include "espresso_compl.h"
#include "espresso_cofactor.h"
#include "espresso_sharp.h"
#include "espresso_cvrm.h"
#include "espresso_contain.h"

void find_inputs(
  TPCubeContext pCubeContext,
  TPCover A,
  TPPLA PLA,
  TSymbolic_List *list,
  TEspressoInt32 base,
  TEspressoInt32 value,
  TPCover *newF,
  TPCover *newD)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCover S, S1;
    register TPSet last, p;

    /*
     *  A represents th 'input' values for which the outputs assume
     *  the integer value 'value
     */
    if (list == NULL) 
    {
	    /*
	     *  Simulate these inputs against the on-set; then, insert into the
	     *  new on-set a 1 in the proper position
	     */
	    S = cv_intersect(pCubeContext, A, PLA->pF);

	    MForeach_Set(S, last, p)
        {
	        MSet_Insert(p, base + value);
	    }

	    *newF = sf_append(pCubeContext, *newF, S);

	    /*
	     *  'simulate' these inputs against the don't-care set
	    S = cv_intersect(A, PLA->D);
	    *newD = sf_append(*newD, S);
	     */
    }
    else
    {
	    /* intersect and recur with the OFF-set */
	    S = cof_output(pCubeContext, PLA->pR, pCubeSettings->pnFirst_Part[pCubeSettings->nOutput] + list->nPos);

	    if (A != NULL)

        {
	        S1 = cv_intersect(pCubeContext, A, S);

	        MFree_Cover(pCubeContext, S);

	        S = S1;
	    }

	    find_inputs(pCubeContext, S, PLA, list->pNext, base, value*2, newF, newD);

	    MFree_Cover(pCubeContext, S);

	    /* intersect and recur with the ON-set */
	    S = cof_output(pCubeContext, PLA->pF, pCubeSettings->pnFirst_Part[pCubeSettings->nOutput] + list->nPos);

	    if (A != NULL)
        {
	        S1 = cv_intersect(pCubeContext, A, S);

	        MFree_Cover(pCubeContext, S);

	        S = S1;
	    }

	    find_inputs(pCubeContext, S, PLA, list->pNext, base, value*2 + 1, newF, newD);

	    MFree_Cover(pCubeContext, S);
    }
}

void map_symbolic(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TSymbolic *p1;
    TSymbolic_List *p2;
    TEspressoInt32 var, base, num_vars, num_binary_vars, *new_part_size;
    TEspressoInt32 new_size, size_added, num_deleted_vars, num_added_vars, newvar;
    TPSet compress;

    /* Verify legal values are in the symbolic lists */
    for(p1 = PLA->pSymbolic; p1 != NULL; p1 = p1->pNext) 
    {
	    for(p2=p1->pSymbolic_List; p2!=NULL; p2=p2->pNext)
        {
	        if (p2->nVariable  < 0 || p2->nVariable >= pCubeSettings->nNum_Binary_Vars)
            {
                assert(0);
    		    //fatal(".symbolic requires binary variables");
	        }
	    }
    }

    /*
     *  size_added = width added for all symbolic variables
     *  num_deleted_vars = # binary variables to be deleted
     *  num_added_vars = # new mv variables
     *  compress = a cube which will be used to compress the set families
     */
    size_added = 0;
    num_added_vars = 0;
    for(p1 = PLA->pSymbolic; p1 != NULL; p1 = p1->pNext)
    {
	    size_added += 1 << p1->nSymbolic_List_Length;
	    num_added_vars++;
    }
    compress = MSet_Full(PLA->pF->nSF_size + size_added);
    for(p1 = PLA->pSymbolic; p1 != NULL; p1 = p1->pNext)
    {
	    for(p2=p1->pSymbolic_List; p2 != NULL; p2=p2->pNext)
        {
	        MSet_Remove(compress, p2->nVariable*2);
	        MSet_Remove(compress, p2->nVariable*2+1);
	    }
    }
    num_deleted_vars = ((PLA->pF->nSF_size + size_added) - set_ord(pCubeContext, compress))/2;

    /* compute the new cube constants */
    num_vars = pCubeSettings->nNum_Vars - num_deleted_vars + num_added_vars;
    num_binary_vars = pCubeSettings->nNum_Binary_Vars - num_deleted_vars;
    new_size = pCubeSettings->nSize - num_deleted_vars*2 + size_added;
    new_part_size = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * num_vars);
    new_part_size[num_vars-1] = pCubeSettings->pnPart_Size[pCubeSettings->nNum_Vars-1];
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars-1; var++) 
    {
	    new_part_size[var-num_deleted_vars] = pCubeSettings->pnPart_Size[var];
    }

    /* re-size the covers, opening room for the new mv variables */
    base = pCubeSettings->pnFirst_Part[pCubeSettings->nOutput];
    PLA->pF = sf_addcol(pCubeContext, PLA->pF, base, size_added);
    PLA->pD = sf_addcol(pCubeContext, PLA->pD, base, size_added);
    PLA->pR = sf_addcol(pCubeContext, PLA->pR, base, size_added);

    /* compute the values for the new mv variables */
    newvar = (pCubeSettings->nNum_Vars - 1) - num_deleted_vars;
    for(p1 = PLA->pSymbolic; p1 != NULL; p1 = p1->pNext)
    {
	    PLA->pF = map_symbolic_cover(PLA->pF, p1->pSymbolic_List, base);
	    PLA->pD = map_symbolic_cover(PLA->pD, p1->pSymbolic_List, base);
	    PLA->pR = map_symbolic_cover(PLA->pR, p1->pSymbolic_List, base);
	    base += 1 << p1->nSymbolic_List_Length;
	    new_part_size[newvar++] = 1 << p1->nSymbolic_List_Length;
    }

    /* delete the binary variables which disappear */
    PLA->pF = sf_compress(pCubeContext, PLA->pF, compress);
    PLA->pD = sf_compress(pCubeContext, PLA->pD, compress);
    PLA->pR = sf_compress(pCubeContext, PLA->pR, compress);

    symbolic_hack_labels(pCubeContext, PLA, PLA->pSymbolic, compress,
		                 new_size, pCubeSettings->nSize, size_added);
    setdown_cube(pCubeContext);

    if (pCubeSettings->pnPart_Size)
    {
        UtFreeMem(pCubeSettings->pnPart_Size);
        pCubeSettings->pnPart_Size = 0;
    }

    pCubeSettings->nNum_Vars = num_vars;
    pCubeSettings->nNum_Binary_Vars = num_binary_vars;
    pCubeSettings->pnPart_Size = new_part_size;
    cube_setup(pCubeContext);
    MSet_Free(compress);
}

TPCover map_symbolic_cover(
  TPCover T,
  TSymbolic_List *list,
  TEspressoInt32 base)
{
    TPSet last, p;
    MForeach_Set(T, last, p)
    {
    	form_bitvector(p, base, 0, list);
    }
    return T;
}


void form_bitvector(
  TPSet p,			/* old cube, looking at binary variables */
  TEspressoInt32 base,		/* where in mv cube the new variable starts */
  TEspressoInt32 value,		/* current value for this recursion */
  TSymbolic_List *list)	/* current place in the symbolic list */
{
    if (list == NULL)
    {
	    MSet_Insert(p, base + value);
    }
    else
    {
	    switch(MGetInput(p, list->nVariable))
        {
	        case CEspressoZero:
		        form_bitvector(p, base, value*2, list->pNext);
		        break;
	        case CEspressoOne:
		        form_bitvector(p, base, value*2+1, list->pNext);
		        break;
	        case CEspressoTwo:
		        form_bitvector(p, base, value*2, list->pNext);
		        form_bitvector(p, base, value*2+1, list->pNext);
	    	    break;
	        default:
                assert(0);
    		    //fatal("bad cube in form_bitvector");
                break;
	    }
    }
}

void symbolic_hack_labels(
  TPCubeContext pCubeContext,
  TPPLA PLA,
  TSymbolic *list,
  TPSet compress,
  TEspressoInt32 new_size,
  TEspressoInt32 old_size,
  TEspressoInt32 size_added)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 i, base;
    LPTSTR* oldlabel;
    TSymbolic *p1;
    TSymbolic_Label *p3;
    TEspressoInt32 nLabelLen = 10;

    /* hack with the labels */
    if ((oldlabel = PLA->ppszLabel) == NULL)
    {
    	return;
    }

    PLA->ppszLabel = (LPTSTR*)UtAllocMem(sizeof(LPTSTR) * new_size);
    for(i = 0; i < new_size; i++)
    {
	    PLA->ppszLabel[i] = NULL;
    }

    /* copy the binary variable labels and unchanged mv variable labels */
    base = 0;
    for(i = 0; i < pCubeSettings->pnFirst_Part[pCubeSettings->nOutput]; i++)
    {
	    if (MIs_In_Set(compress, i))
        {
	        PLA->ppszLabel[base++] = oldlabel[i];
	    }
        else
        {
	        if (oldlabel[i] != NULL)
            {
                if (oldlabel[i])
                {
                    UtFreeMem(oldlabel[i]);
                    oldlabel[i] = NULL; 
                }
	        }
	    }
    }

    /* add the user-defined labels for the symbolic outputs */
    for(p1 = list; p1 != NULL; p1 = p1->pNext)
    {
	    p3 = p1->pSymbolic_Label;
	    for(i = 0; i < (1 << p1->nSymbolic_List_Length); i++)
        {
	        if (p3 == NULL)
            {
		        PLA->ppszLabel[base+i] = (LPTSTR)UtAllocMem(sizeof(TCHAR) * nLabelLen);
                StringCchPrintf(PLA->ppszLabel[base+i], nLabelLen, TEXT("X%d"), i);
	        }
            else
            {
	    	    PLA->ppszLabel[base+i] = p3->pszLabel;
		        p3 = p3->pNext;
	        }
	    }
	    base += 1 << p1->nSymbolic_List_Length;
    }

    /* copy the labels for the binary outputs which remain */
    for(i = pCubeSettings->pnFirst_Part[pCubeSettings->nOutput]; i < old_size; i++)
    {
	    if (MIs_In_Set(compress, i + size_added))
        {
	        PLA->ppszLabel[base++] = oldlabel[i];
	    }
        else
        {
	        if (oldlabel[i] != NULL)
            {
                if (oldlabel[i]) 
                {
                    UtFreeMem(oldlabel[i]);
                    oldlabel[i] = NULL;
                }
	        }
	    }
    }

    if (oldlabel)
    {
        UtFreeMem(oldlabel);
        oldlabel = NULL; 
    }
}

void map_output_symbolic(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPSet_Family newF, newD;
    TPSet compress;
    TSymbolic *p1;
    TSymbolic_List *p2;
    TEspressoInt32 i, bit, tot_size, base, old_size;

    newF = NULL;

    /* Remove the DC-set from the ON-set (is this necessary ??) */
    if (PLA->pD->nCount > 0)
    {
	    sf_free(pCubeContext, PLA->pF);
	    PLA->pF = complement(pCubeContext, cube2list(pCubeContext, PLA->pD, PLA->pR));
    }

    /* tot_size = width added for all symbolic variables */
    tot_size = 0;
    for(p1=PLA->pSymbolic_Output; p1!=NULL; p1=p1->pNext)
    {
	    for(p2=p1->pSymbolic_List; p2!=NULL; p2=p2->pNext)
        {
	        if (p2->nPos<0 || p2->nPos>=pCubeSettings->pnPart_Size[pCubeSettings->nOutput])
            {
                assert(0);
		        //fatal("symbolic-output index out of range");
	        }
	    }
	    tot_size += 1 << p1->nSymbolic_List_Length;
    }

    /* adjust the indices to skip over new outputs */
    for(p1=PLA->pSymbolic_Output; p1!=NULL; p1=p1->pNext)
    {
	    for(p2=p1->pSymbolic_List; p2!=NULL; p2=p2->pNext)
        {
	        p2->nPos += tot_size;
	    }
    }

    /* resize the cube structure -- add enough for the one-hot outputs */
    old_size = pCubeSettings->nSize;
    pCubeSettings->pnPart_Size[pCubeSettings->nOutput] += tot_size;
    setdown_cube(pCubeContext);
    cube_setup(pCubeContext);

    /* insert space in the output part for the one-hot output */
    base = pCubeSettings->pnFirst_Part[pCubeSettings->nOutput];
    PLA->pF = sf_addcol(pCubeContext, PLA->pF, base, tot_size);
    PLA->pD = sf_addcol(pCubeContext, PLA->pD, base, tot_size);
    PLA->pR = sf_addcol(pCubeContext, PLA->pR, base, tot_size);

    /* do the real work */
    for(p1=PLA->pSymbolic_Output; p1!=NULL; p1=p1->pNext)
    {
	    newF = MNew_Cover(pCubeContext, 100, pCubeSettings);
	    newD = MNew_Cover(pCubeContext, 100, pCubeSettings);
	    find_inputs(pCubeContext, NULL, PLA, p1->pSymbolic_List, base, 0,
			        &newF, &newD);
    /*
     *  Not sure what this means
	    find_dc_inputs(PLA, p1->symbolic_list,
			        base, 1 << p1->symbolic_list_length, &newF, &newD);
     */
	    MFree_Cover(pCubeContext, PLA->pF);
	    PLA->pF = newF;
    /*
     *  retain OLD DC-set -- but we've lost the don't-care arc information
     *  (it defaults to branch to the zero state)
	    free_cover(PLA->D);
	    PLA->D = newD;
     */
	    MFree_Cover(pCubeContext, newD);
	    base += 1 << p1->nSymbolic_List_Length;
    }

    /* delete the old outputs, and resize the cube */
    compress = MSet_Full(newF->nSF_size);
    for(p1=PLA->pSymbolic_Output; p1!=NULL; p1=p1->pNext)
    {
	    for(p2=p1->pSymbolic_List; p2!=NULL; p2=p2->pNext)
        {
	        bit = pCubeSettings->pnFirst_Part[pCubeSettings->nOutput] + p2->nPos;
	        MSet_Remove(compress, bit);
	    }
    }
    pCubeSettings->pnPart_Size[pCubeSettings->nOutput] -= newF->nSF_size - set_ord(pCubeContext, compress);
    setdown_cube(pCubeContext);
    cube_setup(pCubeContext);
    PLA->pF = sf_compress(pCubeContext, PLA->pF, compress);
    PLA->pD = sf_compress(pCubeContext, PLA->pD, compress);
    if (pCubeSettings->nSize != PLA->pF->nSF_size)
    {
        assert(0);
        //fatal("error");
    }

    /* Quick minimization */
    PLA->pF = sf_contain(pCubeContext, PLA->pF);
    PLA->pD = sf_contain(pCubeContext, PLA->pD);
    for(i = 0; i < pCubeSettings->nNum_Vars; i++)
    {
	    PLA->pF = d1merge(pCubeContext, PLA->pF, i);
	    PLA->pD = d1merge(pCubeContext, PLA->pD, i);
    }
    PLA->pF = sf_contain(pCubeContext, PLA->pF);
    PLA->pD = sf_contain(pCubeContext, PLA->pD);

    MFree_Cover(pCubeContext, PLA->pR);
    PLA->pR = MNew_Cover(pCubeContext, 0, pCubeSettings);

    symbolic_hack_labels(pCubeContext, PLA, PLA->pSymbolic_Output,
           			     compress, pCubeSettings->nSize, old_size, tot_size);
    MSet_Free(compress);
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
