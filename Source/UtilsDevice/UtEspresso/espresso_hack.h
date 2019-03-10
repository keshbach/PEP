/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_hack_h)
#define espresso_hack_h

void find_inputs(TPCubeContext pCubeContext,
                 TPCover A,
                 TPPLA PLA,
                 TSymbolic_List *list,
                 TEspressoInt32 base,
                 TEspressoInt32 value,
                 TPCover *newF,
                 TPCover *newD);

void map_symbolic(TPCubeContext pCubeContext, TPPLA PLA);

TPCover map_symbolic_cover(TPCover T, TSymbolic_List *list, TEspressoInt32 base);

void form_bitvector(TPSet p, TEspressoInt32 base, TEspressoInt32 value, TSymbolic_List *list);

void symbolic_hack_labels(TPCubeContext pCubeContext,
                          TPPLA PLA,
                          TSymbolic *list,
                          TPSet compress,
                          TEspressoInt32 new_size,
                          TEspressoInt32 old_size,
                          TEspressoInt32 size_added);

void map_output_symbolic(TPCubeContext pCubeContext,
                         TPPLA PLA);

#endif /* end of espresso_hack_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
