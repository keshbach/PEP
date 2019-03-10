/***************************************************************************/
/*  Copyright (C) 2018-2018 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtTemplates_H)
#define UtTemplates_H

template <class T, class U>
System::Boolean IsInstance(U u)
{
    return dynamic_cast<T>(u) != nullptr;
}

#endif /* end of UtTemplates_H */

/***************************************************************************/
/*  Copyright (C) 2018-2018 Kevin Eshbach                                  */
/***************************************************************************/
