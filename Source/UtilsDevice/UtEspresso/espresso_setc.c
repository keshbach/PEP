/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_setc.h"
#include "espresso_set.h"

/*
    cactive -- return the index of the single active variable in
    the cube, or return -1 if there are none or more than 2.
*/

TEspressoInt32 cactive(
  TPCubeContext pCubeContext,
  register TPCube a)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 active = -1, dist = 0/*, bit_index()*/;

 {  /* Check binary variables */
    register TEspressoInt32 w, last;
    register TEspressoUInt32 x;
    if ((last = pCubeSettings->nInWord) != -1)
    {

	/* Check the partial word of binary variables */
	x = a[last];
	x = ~ (x & x >> 1) & pCubeSettings->nInMask;

	if (x)
    {
        if ((dist = MCount_Ones(x, pCubeContext->nBit_Count)) > 1)
        {
		    return -1;		/* more than 2 active variables */
        }
	    active = (last-1)*(CBitsPerInteger/2) + bit_index(x) / 2;
	}

	/* Check the full words of binary variables */
	for(w = 1; w < last; w++)
    {
	    x = a[w];
	    x = ~ (x & x >> 1) & CDisjoint;
	    if (x)
        {
		    if ((dist += MCount_Ones(x, pCubeContext->nBit_Count)) > 1)
            {
		        return -1;		/* more than 2 active variables */
            }
		    active = (w-1)*(CBitsPerInteger/2) + bit_index(x) / 2;
	    }
	}
    }
 }

 {  /* Check the multiple-valued variables */
    register TEspressoInt32 w, var, last;
    register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var];
	    last = pCubeSettings->pnLast_Word[var];
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        if (mask[w] & ~ a[w])
            {
		        if (++dist > 1)
                {
		            return -1;
                }
		        active = var;
		        break;
	        }
        }
    }
 }
 return active;
}

/*
    ccommon -- return TRUE if a and b are share "active" variables
    active variables include variables that are empty;
*/

TEspressoBool ccommon(
  TPCubeContext pCubeContext,
  register TPCube a,
  register TPCube b,
  register TPCube cof)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

 {  /* Check binary variables */
    TEspressoInt32 last;
    register TEspressoInt32 w;
    register TEspressoUInt32 x, y;
    if ((last = pCubeSettings->nInWord) != -1)
    {
	    /* Check the partial word of binary variables */
	    x = a[last] | cof[last];
	    y = b[last] | cof[last];
	    if (~(x & x>>1) & ~(y & y>>1) & pCubeSettings->nInMask)
        {
	        return CEspressoTrue;
        }

	    /* Check the full words of binary variables */
	    for(w = 1; w < last; w++)
        {
	        x = a[w] | cof[w];
	        y = b[w] | cof[w];
	        if (~(x & x>>1) & ~(y & y>>1) & CDisjoint)
            {
		        return CEspressoTrue;
            }
	    }
    }
 }

 {  /* Check the multiple-valued variables */
    TEspressoInt32 var;
    register TEspressoInt32 w, last;
    register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var]; last = pCubeSettings->pnLast_Word[var];
	    /* Check for some part missing from a */
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        if (mask[w] & ~a[w] & ~cof[w])
            {
		        /* If so, check for some part missing from b */
		        for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
                {
		            if (mask[w] & ~b[w] & ~cof[w])
                    {
    			        return CEspressoTrue;            /* both active */
                    }
                }
		        break;
	        }
    }
    }
 }
    return CEspressoFalse;
}

/*
    These routines compare two sets (cubes) for the qsort() routine and
    return:

	-1 if set a is to precede set b
	 0 if set a and set b are equal
	 1 if set a is to follow set b

    Usually the SIZE field of the set is assumed to contain the size
    of the set (which will save recomputing the set size during the
    sort).  For distance-1 merging, the global variable cube.temp[0] is
    a mask which mask's-out the merging variable.
*/

/* descend -- comparison for descending sort on set size */
int CEspressoQSortCallStyle descend(
  void* context, 
  const void* elem1,
  const void* elem2)
{
    TPSet *a = (TPSet*)elem1;
    TPSet *b = (TPSet*)elem2;
    register TPSet a1 = *a, b1 = *b;

    context;

    if (MSize(a1) > MSize(b1))
    {
        return -1;
    }
    else if (MSize(a1) < MSize(b1))
    {
        return 1;
    }
    else
    {
    	register TEspressoInt32 i = MLoop(a1);

	    do
        {
	        if (a1[i] > b1[i])
            {
                return -1;
            }
            else if (a1[i] < b1[i])
            {
                return 1;
            }
        }
	    while (--i > 0);
    }

    return 0;
}

/* ascend -- comparison for ascending sort on set size */
int CEspressoQSortCallStyle ascend(
  void* context, 
  const void* elem1,
  const void* elem2)
{
    TPSet* a = (TPSet*)elem1;
    TPSet* b = (TPSet*)elem2;
    register TPSet a1 = *a, b1 = *b;

    context;

    if (MSize(a1) > MSize(b1))
    {
        return 1;
    }
    else if (MSize(a1) < MSize(b1))
    {
        return -1;
    }
    else
    {
	    register TEspressoInt32 i = MLoop(a1);

	    do
        {
	        if (a1[i] > b1[i])
            {
                return 1;
            }
            else if (a1[i] < b1[i])
            {
                return -1;
            }
        } while (--i > 0);
    }

    return 0;
}

/* d1_order -- comparison for distance-1 merge routine */
int CEspressoQSortCallStyle d1_order(
  void* context,
  const void* elem1,
  const void* elem2)
{
    TPSet *a = (TPSet*)elem1;
    TPSet *b = (TPSet*)elem2;
    TPCubeContext pCubeContext = (TPCubeContext)context;
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPSet a1 = *a, b1 = *b, c1 = pCubeSettings->ppTemp[0];
    register TEspressoInt32 i = MLoop(a1);
    register TEspressoUInt32 x1, x2;

    do
    {
	    if ((x1 = a1[i] | c1[i]) > (x2 = b1[i] | c1[i]))
        {
            return -1;
        }
	    else if (x1 < x2)
        {
            return 1;
        }
    } while (--i > 0);

    return 0;
}

/* desc1 -- comparison (without indirection) for descending sort */
/* also has effect of handling NULL pointers,and a NULL pointer has smallest
order */
TEspressoInt32 desc1(
  register TPSet a,
  register TPSet b)
{
    if (a == NULL)
    {
	    return (b == NULL) ? 0 : 1;
    }
    else if (b == NULL)
    {
	    return -1;
    }

    if (MSize(a) > MSize(b))
    {
        return -1;
    }
    else if (MSize(a) < MSize(b))
    {
        return 1;
    }
    else
    {
	    register TEspressoInt32 i = MLoop(a);

	    do
        {
	        if (a[i] > b[i])
            {
                return -1;
            }
            else if (a[i] < b[i])
            {
                return 1;
            }
        } while (--i > 0);
    }
    return 0;
}

/*
    cdist0 -- return TRUE if a and b are distance 0 apart
*/

TEspressoBool cdist0(
  TPCubeContext pCubeContext,
  register TPCube a,
  register TPCube b)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

 {  /* Check binary variables */
    register TEspressoInt32 w, last; register TEspressoUInt32 x;
    if ((last = pCubeSettings->nInWord) != -1)
    {
	/* Check the partial word of binary variables */
	x = a[last] & b[last];
	if (~(x | x >> 1) & pCubeSettings->nInMask)
    {
	    return CEspressoFalse;               /* disjoint in some variable */
    }

	/* Check the full words of binary variables */
	for(w = 1; w < last; w++)
    {
	    x = a[w] & b[w];
	    if (~(x | x >> 1) & CDisjoint)
        {
		    return CEspressoFalse;           /* disjoint in some variable */
        }
	}
    }
 }

 {  /* Check the multiple-valued variables */
    register TEspressoInt32 w, var, last; register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var]; last = pCubeSettings->pnLast_Word[var];
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        if (a[w] & b[w] & mask[w])
            {
		       goto nextvar;
            }
        }
	    return CEspressoFalse;           /* disjoint in this variable */
        nextvar: ;
    }
 }
    return CEspressoTrue;
}

/*
    cdist01 -- return the "distance" between two cubes (defined as the
    number of null variables in their intersection).  If the distance
    exceeds 1, the value 2 is returned.
*/

TEspressoInt32 cdist01(
  TPCubeContext pCubeContext,
  register TPSet a,
  register TPSet b)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 dist = 0;

 {  /* Check binary variables */
    register TEspressoInt32 w, last; register TEspressoUInt32 x;
    if ((last = pCubeSettings->nInWord) != -1)
    {
	/* Check the partial word of binary variables */
	x = a[last] & b[last];
	x = ~ (x | x >> 1) & pCubeSettings->nInMask;

	if (x)
    {
	    if ((dist = MCount_Ones(x, pCubeContext->nBit_Count)) > 1)
        {
    		return 2;
        }
    }

	/* Check the full words of binary variables */
	for(w = 1; w < last; w++)
    {
	    x = a[w] & b[w];
	    x = ~ (x | x >> 1) & CDisjoint;
	    if (x)
        {
	    	if (dist == 1 || (dist += MCount_Ones(x, pCubeContext->nBit_Count)) > 1)
            {
		        return 2;
            }
        }
	}
    }
 }

 {  /* Check the multiple-valued variables */
    register TEspressoInt32 w, var, last; register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++) 
    {
	mask = pCubeSettings->ppVar_Mask[var]; last = pCubeSettings->pnLast_Word[var];
	for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
    {
	    if (a[w] & b[w] & mask[w])
        {
		    goto nextvar;
        }
    }

	if (++dist > 1)
    {
	    return 2;
    }
    nextvar: ;
    }
 }
    return dist;
}

/*
    cdist -- return the "distance" between two cubes (defined as the
    number of null variables in their intersection).
*/

TEspressoInt32 cdist(
  TPCubeContext pCubeContext,
  register TPSet a,
  register TPSet b)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 dist = 0;

 {  /* Check binary variables */
    register TEspressoInt32 w, last; register TEspressoUInt32 x;
    if ((last = pCubeSettings->nInWord) != -1)
    {
	    /* Check the partial word of binary variables */
	    x = a[last] & b[last];
	    x = ~ (x | x >> 1) & pCubeSettings->nInMask;
	    if (x)
        {
	        dist = MCount_Ones(x, pCubeContext->nBit_Count);
        }

	    /* Check the full words of binary variables */
	    for(w = 1; w < last; w++)
        {
	        x = a[w] & b[w];
	        x = ~ (x | x >> 1) & CDisjoint;
	        if (x)
            {
    		    dist += MCount_Ones(x, pCubeContext->nBit_Count);
            }
	    }
    }
 }

 {  /* Check the multiple-valued variables */
    register TEspressoInt32 w, var, last; register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var]; last = pCubeSettings->pnLast_Word[var];
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        if (a[w] & b[w] & mask[w])
            {
    		    goto nextvar;
            }
        }
	    dist++;
        nextvar: ;
    }
 }
    return dist;
}

/*
    force_lower -- Determine which variables of a do not intersect b.
*/

TPSet force_lower(
  TPCubeContext pCubeContext,
  /*INOUT*/ TPSet xlower,
  /*IN*/ register TPSet a,
  /*IN*/ register TPSet b)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

 {  /* Check binary variables (if any) */
    register TEspressoInt32 w, last; register TEspressoUInt32 x;
    if ((last = pCubeSettings->nInWord) != -1)
    {
	    /* Check the partial word of binary variables */
	    x = a[last] & b[last];
	    x = ~(x | x >> 1) & pCubeSettings->nInMask;
	    if (x)
        {
	        xlower[last] |= (x | (x << 1)) & a[last];
        }

	    /* Check the full words of binary variables */
	    for(w = 1; w < last; w++) 
        {
	        x = a[w] & b[w];
	        x = ~(x | x >> 1) & CDisjoint;
	        if (x)
            {
    		    xlower[w] |= (x | (x << 1)) & a[w];
            }
	    }
    }
 }

 {  /* Check the multiple-valued variables */
    register TEspressoInt32 w, var, last; register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++) 
    {
	    mask = pCubeSettings->ppVar_Mask[var]; last = pCubeSettings->pnLast_Word[var];
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        if (a[w] & b[w] & mask[w])
            {
	    	    goto nextvar;
            }
        }

	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        xlower[w] |= a[w] & mask[w];
        }
        nextvar: ;
    }
 }
    return xlower;
}

/*
    consensus -- multiple-valued consensus

    Although this looks very messy, the idea is to compute for r the
    "and" of the cubes a and b for each variable, unless the "and" is
    null in a variable, in which case the "or" of a and b is computed
    for this variable.

    Because we don't check how many variables are null in the
    intersection of a and b, the returned value for r really only
    represents the consensus when a and b are distance 1 apart.
*/

void consensus(
  TPCubeContext pCubeContext,
  /*INOUT*/ TPCube r,
  /*IN*/ register TPCube a,
  /*IN*/ register TPCube b)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;

    MInlineSet_Clear(r, pCubeSettings->nSize);

    {  /* Check binary variables (if any) */
    register TEspressoInt32 w, last;
    register TEspressoUInt32 x;

    if ((last = pCubeSettings->nInWord) != -1)
    {
	    /* Check the partial word of binary variables */
	    r[last] = x = a[last] & b[last];
	    x = ~(x | x >> 1) & pCubeSettings->nInMask;
	    if (x)
        {
	        r[last] |= (x | (x << 1)) & (a[last] | b[last]);
        }

	    /* Check the full words of binary variables */
	    for(w = 1; w < last; w++)
        {
	        r[w] = x = a[w] & b[w];
	        x = ~(x | x >> 1) & CDisjoint;
	        if (x)
            {
    		    r[w] |= (x | (x << 1)) & (a[w] | b[w]);
            }
	    }
    }
 }


 {  /* Check the multiple-valued variables */
    TEspressoBool empty;
    TEspressoInt32 var;
    TEspressoUInt32 x;
    register TEspressoInt32 w, last;
    register TPCube mask;
    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    mask = pCubeSettings->ppVar_Mask[var];
	    last = pCubeSettings->pnLast_Word[var];
	    empty = CEspressoTrue;
	    for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
        {
	        x = a[w] & b[w] & mask[w];
	        if (x)
            {
	    	    empty = CEspressoFalse, r[w] |= x;
            }
        }

	    if (empty)
        {
	        for(w = pCubeSettings->pnFirst_Word[var]; w <= last; w++)
            {
    		    r[w] |= mask[w] & (a[w] | b[w]);
            }
        }
    }
 }
}

/* see if the cube has a full row of 1's (with respect to cof) */
TEspressoBool full_row(
  TPCubeContext pCubeContext,
  /*IN*/ register TPCube p,
  /*IN*/ register TPCube cof)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 i = MLoop(p);

    do
    {
        if ((p[i] | cof[i]) != pCubeSettings->pFullSet[i])
        {
            return CEspressoFalse;
        }
    } while (--i > 0);

    return CEspressoTrue;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
