/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for TreeView.
        /// </summary>

        public ref class TreeView : System::Windows::Forms::TreeView
        {
        // List View Events
        public:
            delegate void KeyPressLabelEditHandler(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e);
            [System::ComponentModel::Description("Occurs when a key is pressed while focus is on the label edit control."),
                System::ComponentModel::Category("Label Edit Key")]
            event KeyPressLabelEditHandler^ KeyPressLabelEdit;

        public:
            enum class ESortOrder
            {
                None,
                Ascending,
                Descending,
                UserDefined
            };

            enum class EDragDropScrollHitTest
            {
                None,
                UpArea,
                DownArea
            };

        public:
            /// <summary>
            /// Indicates the sort order the control uses.
            /// </summary>

            [System::ComponentModel::Description("Indicates the sort order the control uses."),
                System::ComponentModel::Category("Behavior")]
            property ESortOrder Sorting
            {
                ESortOrder get()
                {
                    return m_SortOrder;
                }

                void set(ESortOrder value)
                {
                    System::Windows::Forms::TreeView^ TreeView = dynamic_cast<System::Windows::Forms::TreeView^>(this);

                    m_SortOrder = value;

                    switch (value)
                    {
                        case ESortOrder::None:
                        case ESortOrder::UserDefined:
                            break;
                        case ESortOrder::Ascending:
                            TreeView->TreeViewNodeSorter = gcnew TreeNodeAscendingSorter();
                            break;
                        case ESortOrder::Descending:
                            TreeView->TreeViewNodeSorter = gcnew TreeNodeDescendingSorter();
                            break;
                        default:
                            System::Diagnostics::Debug::Assert(false, "Unknown sort order.");
                            break;
                    }
                }
            }

            /// <summary>
            /// Indicates the sorter the control uses.
            /// </summary>

            property System::Collections::IComparer^ TreeViewNodeSorter
            {
                System::Collections::IComparer^ get() new
                {
                    return dynamic_cast<System::Windows::Forms::TreeView^>(this)->TreeViewNodeSorter;
                }

                void set(System::Collections::IComparer^ value) new
                {
                    m_SortOrder = ESortOrder::UserDefined;

                    dynamic_cast<System::Windows::Forms::TreeView^>(this)->TreeViewNodeSorter = value;
                }
            }

            /// <summary>
            /// Highlights the node that is the target of a drag and drop operation
            /// but does not ensure it is visible.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::Windows::Forms::TreeNode^ DropHighlightedNode
            {
                void set(System::Windows::Forms::TreeNode^ value)
                {
                    HWND hWnd = (HWND)Handle.ToPointer();
                    HTREEITEM hTreeItem = NULL;

                    if (value != nullptr)
                    {
                        hTreeItem = (HTREEITEM)value->Handle.ToPointer();
                    }

                    TreeView_SelectDropTarget(hWnd, hTreeItem);
                }
            }

            /// <summary>
            /// Indicates if the tree view can be scrolled up.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::Boolean CanScrollUp
            {
                System::Boolean get()
                {
                    HWND hWnd = (HWND)Handle.ToPointer();
                    SCROLLINFO ScrollInfo;

                    ScrollInfo.cbSize = sizeof(ScrollInfo);
                    ScrollInfo.fMask = SIF_POS | SIF_RANGE;

                    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

                    if (ScrollInfo.nPos == ScrollInfo.nMin)
                    {
                        return false;
                    }

                    return true;
                }
            }

            /// <summary>
            /// Indicates if the tree view can be scrolled down.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::Boolean CanScrollDown
            {
                System::Boolean get()
                {
                    HWND hWnd = (HWND)Handle.ToPointer();
                    SCROLLINFO ScrollInfo;

                    ScrollInfo.cbSize = sizeof(ScrollInfo);
                    ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;

                    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

                    if (ScrollInfo.nPos + (INT)ScrollInfo.nPage > ScrollInfo.nMax)
                    {
                        return false;
                    }

                    return true;
                }
            }

        public:
            TreeView();

            /// <summary>
            /// Sorts all of the items in the tree view with the current sorter.
            /// </summary>

            void Sort(void) new;

            /// <summary>
            /// Changes the label edit's text.
            /// <param name="sText">
            /// New text for the label edit.
            /// </param>
            /// </summary>

            void ChangeLabelEditText(System::String^ sText);

            /// <summary>
            /// Selects a range of characters in the label edit's text.
            /// <param name="nStart">
            /// Starting character position of the selection.  (Set to -1 to clear any selection.)
            /// </param>
            /// <param name="nEnd">
            /// Ending character position of the selection.
            /// </param>
            /// </summary>

            void ChangeLabelEditSelection(System::Int32 nStart, System::Int32 nEnd);

            /// <summary>
            /// Scroll all of the items in the tree view up.
            /// </summary>

            void ScrollUp(void);

            /// <summary>
            /// Scroll all of the items in the tree view down.
            /// </summary>

            void ScrollDown(void);

            /// <summary>
            /// Indicates where a point is during a drag and drop operation.
            /// <param name="point">
            /// Point in the client area.
            /// </param>
            /// </summary>

            EDragDropScrollHitTest DragDropScrollHitTest(System::Drawing::Point point);

            /// <summary>
            /// Finds the first tree node that contains the given text.
            /// <param name="NodeColl">
            /// Collection of tree nodes
            /// </param>
            /// <param name="sText">
            /// Text to search for.
            /// </param>
            /// <param name="bIgnoreCase">
            /// Perform a case insensitive comparison.
            /// </param>
            /// </summary>

            static System::Windows::Forms::TreeNode^ FindTreeNode(System::Windows::Forms::TreeNodeCollection^ NodeColl,
                                                                  System::String^ sText,
                                                                  System::Boolean bIgnoreCase);

            /// <summary>
            /// Determines if a node is a child of another node.
            /// <param name="RootNode">
            /// Root node to start the check from.
            /// </param>
            /// <param name="Node">
            /// Node to determine if it's a child of the root node.
            /// </param>
            /// </summary>

            static System::Boolean IsTreeNodeChild(System::Windows::Forms::TreeNode^ RootNode,
                                                   System::Windows::Forms::TreeNode^ Node);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~TreeView();

            virtual void WndProc(System::Windows::Forms::Message% Message) override;

        private:
            /// <summary>
            /// Required designer variable.
            /// </summary>
            System::ComponentModel::Container^ components;

            /// <summary>
            /// Required method for Designer support - do not modify
            /// the contents of this method with the code editor.
            /// </summary>		
            void InitializeComponent(void)
            {
            }

            void HandleRButtonDown(System::Windows::Forms::Message% Message);
            void HandleParentNotify(System::Windows::Forms::Message% Message);
            void HandleLabelEditChar(System::Windows::Forms::Message% Message);

        protected:
            void OnKeyPressLabelEdit(System::Windows::Forms::KeyPressEventArgs^ e);

        private:
            ESortOrder m_SortOrder;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
