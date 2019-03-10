/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace OpenZip.Forms
{
    internal class ListViewNameDescendingComparer : System.Collections.IComparer
    {
        public virtual System.Int32 Compare(
            System.Object x,
            System.Object y)
        {
            System.Windows.Forms.ListViewItem Item1 = (System.Windows.Forms.ListViewItem)x;
            System.Windows.Forms.ListViewItem Item2 = (System.Windows.Forms.ListViewItem)y;

            if ((Item1.Tag == null && Item2.Tag == null) ||
                (Item1.Tag != null && Item2.Tag != null))
            {
                return System.StringComparer.CurrentCulture.Compare(Item2.Text, Item1.Text);
            }
            else if (Item1.Tag == null)
            {
                return 1;
            }
            else if (Item2.Tag == null)
            {
                return -1;
            }

            return 0;
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2014-2014 Kevin Eshbach                                  */
/***************************************************************************/
