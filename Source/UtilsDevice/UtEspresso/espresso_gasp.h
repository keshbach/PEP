/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_gasp_h)
#define espresso_gasp_h

TPCover expand_gasp(TPCubeContext pCubeContext, TPCover F, const TPCover D, const TPCover R, const TPCover Foriginal);

void expand1_gasp(TPCubeContext pCubeContext, TPCover F, TPCover D, TPCover R, TPCover Foriginal, TEspressoInt32 c1index, TPCover *G);

TPCover irred_gasp(TPCubeContext pCubeContext, TPCover F, TPCover D, TPCover G);

TPCover last_gasp(TPCubeContext pCubeContext, TPCover F, TPCover D, TPCover R, TPCost cost);

#endif /*end of espresso_gasp_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
