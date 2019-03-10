/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// Summary description for ToolStripProgressBar.
        /// </summary>

        public ref class ToolStripProgressBar : System::Windows::Forms::ToolStripProgressBar
        {
        public:
            /// <summary>
            /// The current value for the ProgressBar, in the range specified by the maximum and minimum properties.
            /// </summary>

            [System::ComponentModel::Description("The current value for the ProgressBar, in the range specified by the maximum and minimum properties."),
                System::ComponentModel::Category("Behavior")]
            property System::Int32 Value
            {
                System::Int32 get() new
                {
                    return dynamic_cast<System::Windows::Forms::ToolStripProgressBar^>(this)->Value;
                }

                void set(System::Int32 value) new
                {
                    System::Windows::Forms::ToolStripProgressBar^ pProgressBar;

                    pProgressBar = dynamic_cast<System::Windows::Forms::ToolStripProgressBar^>(this);

                    // Fix for Windows 7 Aero theme progress bar not drawing correctly.

                    if (value < pProgressBar->Maximum)
                    {
                        pProgressBar->Value = value + 1;
                        pProgressBar->Value = value;
                    }
                    else
                    {
                        ++pProgressBar->Maximum;

                        pProgressBar->Value = pProgressBar->Maximum;

                        --pProgressBar->Value;
                        --pProgressBar->Maximum;
                    }
                }
            }

        public:
            ToolStripProgressBar();
            ToolStripProgressBar(System::String^ sName);

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~ToolStripProgressBar();

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
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
