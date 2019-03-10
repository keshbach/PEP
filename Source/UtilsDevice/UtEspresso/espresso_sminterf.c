/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sminterf.h"
#include "espresso_sparse.h"
#include "espresso_mincov.h"
#include "espresso_set.h"

TPSet do_sm_minimum_cover(
  TPSet_Family A)
{
    TPSM_Matrix M;
    TPSM_Row sparse_cover;
    TPSM_Element pe;
    TPSet cover;
    register TEspressoInt32 i, base, rownum;
    register TEspressoUInt32 val;
    register TPSet last, p;

    M = sm_alloc();
    rownum = 0;
    MForeach_Set(A, last, p)
    {
	    MForeach_Set_Element(p, i, val, base)
        {
	        sm_insert(M, rownum, base);
	    }
	    rownum++;
    }

    sparse_cover = sm_minimum_cover(M, NULL, 1);
    sm_free(M);

    cover = MSet_New(A->nSF_size);
    MSM_Foreach_Row_Element(sparse_cover, pe)
    {
	    MSet_Insert(cover, pe->nCol_Num);
    }
    sm_row_free(sparse_cover);

    return cover;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
