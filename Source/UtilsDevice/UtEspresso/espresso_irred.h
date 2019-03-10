/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_irred_h)
#define espresso_irred_h

TEspressoBool tautology(TPCubeContext pCubeContext, TPCube *T);

TEspressoBool cube_is_covered(TPCubeContext pCubeContext, TPCube *T, TPCube c);

TPCover irredundant(TPCubeContext pCubeContext, TPCover F, TPCover D);

void mark_irredundant(TPCubeContext pCubeContext, TPCover F, TPCover D);

void irred_split_cover(TPCubeContext pCubeContext, TPCover F, TPCover D, TPCover *E, TPCover *Rt, TPCover *Rp);

TPSM_Matrix irred_derive_table(TPCubeContext pCubeContext, TPCover D, TPCover E, TPCover Rp);

#endif /* end of espresso_irred_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
