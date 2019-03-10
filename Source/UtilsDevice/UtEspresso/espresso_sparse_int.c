/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sparse.h"
#include "espresso_sparse_int.h"

/*
 *  remove an element from a col vector (given a pointer to the element) 
 */
void sm_col_remove_element(
  register TPSM_Col pcol,
  register TPSM_Element p)
{
    MDLL_Unlink(p, pcol->pFirst_Row, pcol->pLast_Row, 
			    pNext_Row, pPrev_Row, pcol->nLength);

    if (p)
    {
        UtFreeMem(p);

        p = NULL;
    }
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
