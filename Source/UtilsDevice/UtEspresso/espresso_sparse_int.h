/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_sparse_int_h)
#define espresso_sparse_int_h

/*
 *  sorted, double-linked list insertion
 *
 *  type: object type
 *
 *  first, last: fields (in header) to head and tail of the list
 *  count: field (in header) of length of the list
 *
 *  next, prev: fields (in object) to link next and previous objects
 *  value: field (in object) which controls the order
 *
 *  newval: value field for new object
 *  e: an object to use if insertion needed (set to actual value used)
 */

#define MSorted_Insert(type, first, last, count, next, prev, value, newval, e) \
    if (last == 0) \
    { \
	    e->value = newval; \
	    first = e; \
	    last = e; \
	    e->next = 0; \
	    e->prev = 0; \
	    count++; \
    } \
    else if (last->value < newval) \
    { \
	    e->value = newval; \
	    last->next = e; \
	    e->prev = last; \
	    last = e; \
	    e->next = 0; \
	    count++; \
    } \
    else if (first->value > newval) \
    { \
	    e->value = newval; \
	    first->prev = e; \
	    e->next = first; \
	    first = e; \
	    e->prev = 0; \
	    count++; \
    } \
    else \
    { \
	    type *p; \
	    for(p = first; p->value < newval; p = p->next) \
	        ; \
	    if (p->value > newval) \
        { \
	        e->value = newval; \
	        p = p->prev; \
	        p->next->prev = e; \
	        e->next = p->next; \
	        p->next = e; \
	        e->prev = p; \
	        count++; \
	    } \
        else \
        { \
	        e = p; \
	    } \
    }

/*
 *  double linked-list deletion
 */
#define MDLL_Unlink(p, first, last, next, prev, count) \
    { \
        if (p->prev == 0) \
        { \
	        first = p->next; \
        } \
        else \
        { \
	        p->prev->next = p->next; \
        } \
        if (p->next == 0) \
        { \
	        last = p->prev; \
        } \
        else \
        { \
	        p->next->prev = p->prev; \
        } \
        count--; \
    }

void sm_col_remove_element(register TPSM_Col pcol, register TPSM_Element p);

#endif /* end of espresso_sparse_int_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
