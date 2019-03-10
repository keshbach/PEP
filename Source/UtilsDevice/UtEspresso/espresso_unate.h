/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_unate_h)
#define espresso_unate_h

TPSet_Family unate_complement(TPCubeContext pCubeContext, TPSet_Family A);

TPSet_Family unate_compl(TPCubeContext pCubeContext, TPSet_Family A);

TPCover map_cover_to_unate(TPCubeContext pCubeContext,
                           TPCube *T);

TPCover map_unate_to_cover(TPCubeContext pCubeContext,
                           TPSet_Family A);

#endif /* end of espresso_unate_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
