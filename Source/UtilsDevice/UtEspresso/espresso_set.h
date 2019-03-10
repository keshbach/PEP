/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_set_h)
#define espress_set_h

TEspressoInt32 bit_index(register TEspressoUInt32 a);

TPSet set_clear(register TPSet r, TEspressoInt32 size);

TPSet set_fill(register TPSet r, register TEspressoInt32 size);

TPSet set_copy(register TPSet r, register TPSet a);

TPSet set_and(register TPSet r, register TPSet a, register TPSet b);

TEspressoInt32 set_ord(TPCubeContext pCubeContext, register TPSet a);

TEspressoInt32 set_dist(TPCubeContext pCubeContext, register TPSet a, register TPSet b);

TPSet set_merge(register TPSet r, register TPSet a, register TPSet b, register TPSet mask);

void set_adjcnt(register TPSet a, register TEspressoInt32 *count, register TEspressoInt32 weight);

TPSet_Family sf_new(TPCubeContext pCubeContext, TEspressoInt32 num, TEspressoInt32 size);

TPSet_Family sf_save(TPCubeContext pCubeContext, register TPSet_Family A);

TPSet_Family sf_addset(TPSet_Family A, TPSet s);

void sf_free(TPCubeContext pCubeContext, TPSet_Family A);

void sf_cleanup(TPCubeContext pCubeContext);

TPSet_Family sf_copy(TPSet_Family R, TPSet_Family A);

TPSet_Family sf_join(TPCubeContext pCubeContext, TPSet_Family A, TPSet_Family B);

TPSet_Family sf_active(TPSet_Family A);

TEspressoBool setp_equal(register TPSet a, register TPSet b);

TEspressoBool setp_disjoint(register TPSet a, register TPSet b);

TEspressoBool setp_implies(register TPSet a, register TPSet b);

TPSet set_or(register TPSet r, register TPSet a, register TPSet b);

TPSet set_diff(register TPSet r,register TPSet a, register TPSet b);

TPSet_Family sf_append(TPCubeContext pCubeContext, TPSet_Family A, TPSet_Family B);

TEspressoInt32 *sf_count(TPSet_Family A);

TEspressoInt32 *sf_count_restricted(TPCubeContext pCubeContext, TPSet_Family A, register TPSet r);

TPSet_Family sf_addcol(TPCubeContext pCubeContext, TPSet_Family A, TEspressoInt32 firstcol, TEspressoInt32 n);

TPSet_Family sf_delcol(TPCubeContext pCubeContext, TPSet_Family A, register TEspressoInt32 firstcol, register TEspressoInt32 n);

TPSet_Family sf_copy_col(TPSet_Family dst, TEspressoInt32 dstcol, TPSet_Family src, TEspressoInt32 srccol);

TPSet_Family sf_compress(TPCubeContext pCubeContext, TPSet_Family A, register TPSet c);

TPSet_Family sf_inactive(TPSet_Family A);

TEspressoBool setp_empty(register TPSet a);

#endif /* end of espress_set_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
