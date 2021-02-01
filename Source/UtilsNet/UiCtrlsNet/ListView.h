/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ListView.
        /// </summary>

        public ref class ListView : System::Windows::Forms::ListView,
                                    Common::Forms::ITextBoxKeyPress
        {
        // List View Events
        public:
            delegate void BeforeComboBoxEditHandler(System::Object^ sender, Common::Forms::ListViewComboBoxEditEventArgs^ e);
            [System::ComponentModel::Description("Occurs when the text of an item is about to be edited by the user with a combo box."),
                System::ComponentModel::Category("Behavior")]
            event BeforeComboBoxEditHandler^ BeforeComboBoxEdit;

            delegate void AfterComboBoxEditHandler(System::Object^ sender, Common::Forms::ListViewComboBoxEditEventArgs^ e);
            [System::ComponentModel::Description("Occurs when the text of an item hs been edited by the user with a combo box."),
                System::ComponentModel::Category("Behavior")]
            event AfterComboBoxEditHandler^ AfterComboBoxEdit;

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
                Group,
                Sequential,
                GroupSequential,
                UserDefined
            };

            enum class ELabelEditor
            {
                None,
                Label,
                ComboBox
            };

            enum class ESortArrow
            {
                None,
                Up,
                Down
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
                ESortOrder get() new
                {
                    return m_SortOrder;
                }

                void set(ESortOrder value)
                {
                    System::Windows::Forms::ListView^ pListView = dynamic_cast<System::Windows::Forms::ListView^>(this);

                    m_SortOrder = value;

                    pListView->ListViewItemSorter = nullptr;
                    pListView->Sorting = System::Windows::Forms::SortOrder::None;

                    switch (value)
                    {
                        case ESortOrder::None:
                        case ESortOrder::UserDefined:
                            break;
                        case ESortOrder::Ascending:
                            pListView->Sorting = System::Windows::Forms::SortOrder::Ascending;
                            break;
                        case ESortOrder::Descending:
                            pListView->Sorting = System::Windows::Forms::SortOrder::Descending;
                            break;
                        case ESortOrder::Group:
                            pListView->ListViewItemSorter = gcnew Common::Forms::ListViewItemGroupSorter();
                            break;
                        case ESortOrder::Sequential:
                            pListView->ListViewItemSorter = gcnew Common::Forms::ListViewItemSequentialSorter();
                            break;
                        case ESortOrder::GroupSequential:
                            pListView->ListViewItemSorter = gcnew Common::Forms::ListViewItemGroupSequentialSorter();
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

            property System::Collections::IComparer^ ListViewItemSorter
            {
                System::Collections::IComparer^ get() new
                {
                    return dynamic_cast<System::Windows::Forms::ListView^>(this)->ListViewItemSorter;
                }

                void set(System::Collections::IComparer^ value) new
                {
                    m_SortOrder = ESortOrder::UserDefined;

                    dynamic_cast<System::Windows::Forms::ListView^>(this)->ListViewItemSorter = value;
                }
            }

            /// <summary>
            /// Indicates the label editor the control uses.
            /// </summary>

            [System::ComponentModel::Description("Indicates the label editor the control uses."),
                System::ComponentModel::Category("Behavior")]
            property ELabelEditor LabelEditor
            {
                ELabelEditor get()
                {
                    return m_LabelEditor;
                }

                void set(ELabelEditor value)
                {
                    System::Windows::Forms::ListView^ ListView;

                    ListView = dynamic_cast<System::Windows::Forms::ListView^>(this);

                    switch (value)
                    {
                        case Common::Forms::ListView::ELabelEditor::Label:
                            ListView->LabelEdit = true;
                            break;
                        case Common::Forms::ListView::ELabelEditor::ComboBox:
                            ListView->LabelEdit = false;
                            break;
                        case Common::Forms::ListView::ELabelEditor::None:
                            ListView->LabelEdit = false;
                            break;
                        default:
                            ListView->LabelEdit = false;
                            System::Diagnostics::Debug::Assert(false, "Unknown label editor.");
                            break;
                    }

                    m_LabelEditor = value;
                }
            }

            /// <summary>
            /// Indicates the items the combo box editor control uses.
            /// </summary>

            [System::ComponentModel::Description("The items in the combo box editor."),
                System::ComponentModel::Category("Behavior")]
            property System::Collections::Specialized::StringCollection^ ComboBoxItems
            {
                System::Collections::Specialized::StringCollection^ get()
                {
                    return m_ComboBoxItems;
                }

                void set(System::Collections::Specialized::StringCollection^ value)
                {
                    m_ComboBoxItems = value;
                }
            }

            /// <summary>
            /// Indicates the sort arrow direction the control uses.
            /// </summary>

            [System::ComponentModel::Description("Indicates the sort arrow direction the control uses."),
                System::ComponentModel::Category("Behavior")]
            property ESortArrow SortArrow
            {
                ESortArrow get()
                {
                    return m_SortArrow;
                }

                void set(ESortArrow value)
                {
                    m_SortArrow = value;

                    if (IsHandleCreated)
                    {
                        RefreshSortArrow();
                    }
                }
            }

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Never)]
            property System::Boolean LabelEdit
            {
                System::Boolean get() new
                {
                    return dynamic_cast<System::Windows::Forms::ListView^>(this)->LabelEdit;
                }

                void set(System::Boolean value) new
                {
                    if (value)
                    {
                        m_LabelEditor = Common::Forms::ListView::ELabelEditor::Label;
                    }

                    dynamic_cast<System::Windows::Forms::ListView^>(this)->LabelEdit = value;
                }
            }

            /// <summary>
            /// Highlights an item by it's index that is the target of a drag and drop operation
            /// but does not ensure it is visible.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::Int32 DropHighlightedIndice
            {
                void set(System::Int32 value)
                {
                    HWND hWnd = (HWND)Handle.ToPointer();
                    INT nIndex = value;

                    ListView_SetItemState(hWnd, -1, 0, LVIS_DROPHILITED);

                    if (value >= 0)
                    {
                        ListView_SetItemState(hWnd, nIndex, LVIS_DROPHILITED,
                                              LVIS_DROPHILITED);
                    }
                }
            }

            /// <summary>
            /// Highlights an item that it is the target of a drag and drop operation
            /// but does not ensure it is visible.
            /// </summary>

            [System::ComponentModel::Browsable(false),
                System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
            property System::Windows::Forms::ListViewItem^ DropHighlightedItem
            {
                void set(System::Windows::Forms::ListViewItem^ value)
                {
                    System::Int32 nIndex = -1;

                    if (value != nullptr)
                    {
                        nIndex = value->Index;
                    }

                    DropHighlightedIndice = nIndex;
                }
            }

            /// <summary>
            /// Indicates if the list view can be scrolled up.
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
            /// Indicates if the list view can be scrolled down.
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
            ListView();

            System::Windows::Forms::ListViewHitTestInfo^ HitTest(System::Drawing::Point point) new;
            System::Windows::Forms::ListViewHitTestInfo^ HitTest(System::Int32 x, System::Int32 y) new;

            /// <summary>
            /// Re-sizes the width of all of the columns making sure all text is completely visible.
            /// </summary>

            void AutosizeColumns(void);

            /// <summary>
            /// Re-sizes the width of the given column making sure all text is completely visible.
            /// <param name="nColumn">
            /// Index of the column to re-size.  (Indexes start at zero.)
            /// </param>
            /// </summary>

            void AutosizeColumn(System::Int32 nColumn);

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
            /// Scroll all of the items in the list view up.
            /// </summary>

            void ScrollUp(void);

            /// <summary>
            /// Scroll all of the items in the list view down.
            /// </summary>

            void ScrollDown(void);

            /// <summary>
            /// Indicates where a point is during a drag and drop operation.
            /// <param name="point">
            /// Point in the client area.
            /// </param>
            /// </summary>

            EDragDropScrollHitTest DragDropScrollHitTest(System::Drawing::Point point);

        // Common::Forms::ITextBoxKeyPress
        public:
            virtual System::Boolean OnTextBoxKeyPress(wchar_t KeyChar);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ListView();

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

            void HandleCreate(System::Windows::Forms::Message% Message);
            void HandleLButtonDblClk(System::Windows::Forms::Message% Message);
            void HandleLButtonDown(System::Windows::Forms::Message% Message);
            void HandleRButtonDown(System::Windows::Forms::Message% Message);
            void HandleNotify(System::Windows::Forms::Message% Message);
            void HandleParentNotify(System::Windows::Forms::Message% Message);

            void BeginListViewComboBoxEdit(System::Int32 nRow, System::Int32 nColumn);
            void EndListViewComboBoxEdit(void);

            void RefreshSortArrow(void);

            // Event handlers
            void ListViewComboBox_SetFocus(System::Object^ sender, System::EventArgs^ e);
            void ListViewComboBox_KillFocus(System::Object^ sender, System::EventArgs^ e);
            void ListViewComboBox_Keydown(System::Object^ sender, Common::Forms::ListViewComboBox::ListViewComboBoxKeydownEventArgs^ e);

        protected:
            void OnBeforeComboBoxEdit(Common::Forms::ListViewComboBoxEditEventArgs^ e);
            void OnAfterComboBoxEdit(Common::Forms::ListViewComboBoxEditEventArgs^ e);
            void OnKeyPressLabelEdit(System::Windows::Forms::KeyPressEventArgs^ e);

        private:
            ESortOrder m_SortOrder;
            ELabelEditor m_LabelEditor;
            System::Collections::Specialized::StringCollection^ m_ComboBoxItems;
            ESortArrow m_SortArrow;
            Common::Forms::ListViewComboBox^ m_ListViewComboBox;
            System::Int32 m_nRow;
            System::Int32 m_nColumn;
            System::Boolean m_bApplyListViewComboBoxChanges;
            System::Boolean m_bIgnoreEndListViewComboBoxEdit;
            NativeEdit^ m_NativeEdit;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
