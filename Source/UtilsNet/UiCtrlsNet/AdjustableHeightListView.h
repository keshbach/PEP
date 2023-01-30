/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for AdjustableHeightListView.
        /// </summary>

        public ref class AdjustableHeightListView : Common::Forms::ListView
        {
        public:
            /// <summary>
            /// Indicates the height of an item in the control.
            /// </summary>

            [System::ComponentModel::Description("Indicates the height of an item in the control."),
                System::ComponentModel::Category("Behavior")]
            property System::Int32 ItemHeight
            {
                System::Int32 get()
                {
                    return m_nItemHeight;
                }

                void set(System::Int32 value)
                {
                    m_nItemHeight = value;

                    if (IsHandleCreated)
                    {
                        Invalidate();
                    }
                }
            }

        protected:
            [System::ComponentModel::Browsable(false)]
            property System::Windows::Forms::CreateParams^ CreateParams
            {
                System::Windows::Forms::CreateParams^ get() override
                {
                    System::Windows::Forms::CreateParams^ Params;

                    Params = System::Windows::Forms::ListView::CreateParams;

                    if ((Params->Style & LVS_REPORT) == LVS_REPORT)
                    {
                        Params->Style |= LVS_OWNERDRAWFIXED;
                    }

                    return Params;
                }
            }

        public:
            AdjustableHeightListView();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~AdjustableHeightListView();

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

            void HandleNotify(System::Windows::Forms::Message% Message);
            void HandleMeasureItem(System::Windows::Forms::Message% Message);
            void HandleDrawItem(System::Windows::Forms::Message% Message);
            void HandleMouseMove(System::Windows::Forms::Message% Message);
            void HandleMove(System::Windows::Forms::Message% Message);
            void HandleSize(System::Windows::Forms::Message% Message);
            void HandleThemeChanged(System::Windows::Forms::Message% Message);
            void HandleCreate(System::Windows::Forms::Message% Message);
            void HandleDestroy(System::Windows::Forms::Message% Message);

            void ProcessMouseMove(LPPOINT pPoint);

            void ProcessTimerEvent(void);

            static void TimerEventProcessor(System::Object^ Object, System::EventArgs^ EventArgs);

        private:
            System::Int32 m_nItemHeight;

            System::Windows::Forms::Timer^ m_Timer;

            HTHEME m_hListViewTheme;

            INT m_nMouseMoveLastItem;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2022 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
