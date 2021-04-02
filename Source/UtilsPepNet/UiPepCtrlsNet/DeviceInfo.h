/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Forms
	{
		/// <summary>
		/// Summary for DeviceInfo
		/// </summary>
		[System::Drawing::ToolboxBitmapAttribute(Pep::Forms::DeviceInfo::typeid, "IDB_DEVICEINFO")]
		public ref class DeviceInfo sealed : public System::Windows::Forms::Control
		{
		public:
			/// <summary>
			/// The name of the device.
			/// </summary>

			[System::ComponentModel::Description("The name of the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceName
			{
				System::String^ get()
				{
					return GetNameValue();
				}

				void set(System::String^ sValue)
				{
					SetNameValue(sValue);
				}
			}

			/// <summary>
			/// The adapter required for the device.
			/// </summary>

			[System::ComponentModel::Description("The adapter required for the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceAdapter
			{
				System::String^ get()
				{
					return GetAdapterValue();
				}

				void set(System::String^ sValue)
				{
					SetAdapterValue(sValue);
				}
			}

			/// <summary>
			/// The adapter required for the device.
			/// </summary>

			[System::ComponentModel::Description("The adapter required for the device."),
				System::ComponentModel::Category("Data")]
			property array<System::Boolean>^ DeviceDipSwitches
			{
				array<System::Boolean>^ get()
				{
					return GetDipSwitchesValue();
				}

				void set(array<System::Boolean>^ Value)
				{
					SetDipSwitchesValue(Value);
				}
			}

			/// <summary>
			/// The size of the device in bytes.
			/// </summary>

			[System::ComponentModel::Description("The size of the device in bytes."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceSize
			{
				System::String^ get()
				{
					return GetSizeValue();
				}

				void set(System::String^ sValue)
				{
					SetSizeValue(sValue);
				}
			}

			/// <summary>
			/// The programming voltage of the device.
			/// </summary>

			[System::ComponentModel::Description("The programming voltage of the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceVpp
			{
				System::String^ get()
				{
					return GetVppValue();
				}

				void set(System::String^ sValue)
				{
					SetVppValue(sValue);
				}
			}

			/// <summary>
			/// The total bits of the device.
			/// </summary>

			[System::ComponentModel::Description("The total bits of the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceBits
			{
				System::String^ get()
				{
					return GetBitsValue();
				}

				void set(System::String^ sValue)
				{
					SetBitsValue(sValue);
				}
			}

			/// <summary>
			/// The chip enable of the device.
			/// </summary>

			[System::ComponentModel::Description("The chip enable of the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceChipEnable
			{
				System::String^ get()
				{
					return GetChipEnableValue();
				}

				void set(System::String^ sValue)
				{
					SetChipEnableValue(sValue);
				}
			}

			/// <summary>
			/// The output enable of the device.
			/// </summary>

			[System::ComponentModel::Description("The output enable of the device."),
				System::ComponentModel::Category("Data")]
			property System::String^ DeviceOutputEnable
			{
				System::String^ get()
				{
					return GetOutputEnableValue();
				}

				void set(System::String^ sValue)
				{
					SetOutputEnableValue(sValue);
				}
			}

			/// <summary>
			/// The minimum rectangle required to display all device information.
			/// </summary>

			[System::ComponentModel::Browsable(false),
				System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Always)]
			property System::Drawing::Rectangle^ MinRect
			{
				System::Drawing::Rectangle^ get()
				{
					return GetMinRectValue();
				}
			}

		public:
			DeviceInfo();
			DeviceInfo(System::ComponentModel::IContainer ^container);

			void BeginUpdate(void);
			void EndUpdate(void);

			/// <summary>
			/// Triggers the WM_SYSCOLORCHANGE message to the Device Info control.
			/// </summary>
			void UpdateSystemColors(void);

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~DeviceInfo();

		private:
			/// <summary>
			/// Required designer variable.
			/// </summary>
			System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
			/// <summary>
			/// Required method for Designer support - do not modify
			/// the contents of this method with the code editor.
			/// </summary>
			void InitializeComponent(void)
			{
				this->SuspendLayout();
				// 
				// BufferViewer
				// 
				this->ResumeLayout(false);
			}
#pragma endregion

		protected:
			void OnHandleCreated(System::EventArgs^ e) override;
			void OnHandleDestroyed(System::EventArgs^ e) override;
			void OnEnabledChanged(System::EventArgs^ e) override;
			void OnVisibleChanged(System::EventArgs^ e) override;
			void OnFontChanged(System::EventArgs^ e) override;
			void OnGotFocus(System::EventArgs^ e) override;
			void OnPaint(System::Windows::Forms::PaintEventArgs^ e) override;
			void OnResize(System::EventArgs^ e) override;

		private:
			void SetNameValue(System::String^ sName);
			System::String^ GetNameValue();

			void SetAdapterValue(System::String^ sAdapter);
			System::String^ GetAdapterValue();

			void SetDipSwitchesValue(array<System::Boolean>^ DipSwitches);
			array<System::Boolean>^ GetDipSwitchesValue();

			void SetSizeValue(System::String^ sSize);
			System::String^ GetSizeValue();

			void SetVppValue(System::String^ sVpp);
			System::String^ GetVppValue();

			void SetBitsValue(System::String^ sBits);
			System::String^ GetBitsValue();

			void SetChipEnableValue(System::String^ sChipEnable);
			System::String^ GetChipEnableValue();

			void SetOutputEnableValue(System::String^ sOutputEnable);
			System::String^ GetOutputEnableValue();

			System::Drawing::Rectangle^ GetMinRectValue();

			void UpdateFont(System::Boolean bRedraw);

		private:
			HWND m_hDeviceInfoCtrl;

			System::String^ m_sDeviceName;
			System::String^ m_sDeviceAdapter;
			array<System::Boolean>^ m_DeviceDipSwitches;
			System::String^ m_sDeviceSize;
			System::String^ m_sDeviceVpp;
			System::String^ m_sDeviceBits;
			System::String^ m_sDeviceChipEnable;
			System::String^ m_sDeviceOutputEnable;
		};
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
