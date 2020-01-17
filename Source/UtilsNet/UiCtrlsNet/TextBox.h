/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
    namespace Forms
    {
        /// <summary>
        /// TextBox with a timer delay after a key has been pressed.
        /// </summary>

        public ref class TextBox : System::Windows::Forms::TextBox
        {
        public:
            delegate void KeyPressTimerExpiredHandler(System::Object^ sender, System::EventArgs^ e);
            [System::ComponentModel::Description("Occurs when the time delay between key presses has expired."),
                System::ComponentModel::Category("Behavior")]
            event KeyPressTimerExpiredHandler^ KeyPressTimerExpired;

        public:
            /// <summary>
            /// Key press timer delay in seconds.
            /// </summary>

            [System::ComponentModel::Description("Key press timer delay in seconds."),
                System::ComponentModel::Category("Behavior")]
            property System::Double KeyPressTimerDelay
            {
                System::Double get()
                {
                    return m_dTimerDelay;
                }

                void set(System::Double value)
                {
                    if (value > 0.0)
                    {
                        m_dTimerDelay = value;

                        m_Timer->Stop();

                        m_Timer->Interval = (System::Int32)(value * 1000);

                        if (m_bTimerStarted)
                        {
                            m_Timer->Start();
                        }
                    }
                }
            }

        public:
            TextBox();

		public:
			void CancelKeyPressTimer();

        protected:
            /// <summary>
            /// Clean up any resources being used.
            /// </summary>
            ~TextBox();

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

            void HandleChar(System::Windows::Forms::Message% Message);
            void HandleKeyDown(System::Windows::Forms::Message% Message);
            static void TimerEventProcessor(System::Object^ Object, System::EventArgs^ EventArgs);

        protected:
            void OnKeyPressTimerExpired(System::EventArgs^ e);

        private:
            System::Double m_dTimerDelay;
            System::Boolean m_bTimerStarted;
            Common::Forms::FormTimer^ m_Timer;
        };
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
