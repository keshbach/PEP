/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	public ref class Process sealed
	{
	public:
		/// <summary>
		/// Interface to implement capturing the output from a process.
		/// </summary>

		interface class IRunProcess
		{
		public:
			virtual void RunProcessBegin();
			virtual void RunProcessEnd();
			virtual void RunProcessStdOutData(System::String^ sData);
			virtual void RunProcessStdErrData(System::String^ sData);
		};

	public:
		/// <summary>
		/// Class to determine if an existing process is already running.
		/// </summary>
		ref class Running sealed
		{
		public:
			property System::Boolean AlreadyRunning
			{
				System::Boolean get()
				{
					return m_bAlreadyRunning;
				}
			}

		public:
			Running(System::String^ sAppName);

			/// <summary>
			/// Releases the associated resources.
			/// </summary>

			void Close();

		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~Running();

		private:
			System::Boolean m_bAlreadyRunning;
			System::Threading::Mutex^ m_AppMutex;

		private:
			Running();
		};

	public:
		/// <summary>
		/// Launches and captures the output from a process.
		/// </summary>

		static System::Boolean LaunchAndCaptureProcessOutput(
			System::String^ sApplication,
			System::String^ sArguments,
			Common::Process::IRunProcess^ RunProcess,
			System::String^% sErrorMsg);

		/// <summary>
		/// Triggers a stop to the capture of a process's output.
		/// </summary>

		static System::Boolean StopCaptureAppOutput();
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
