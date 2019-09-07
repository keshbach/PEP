/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Application
    {
        static class Startup
        {
            /// <summary>
            /// The main entry point for the application.
            /// </summary>
            [STAThread]
            static void Main()
            {
                Common.Process.Running AppRunning;
                Pep.Forms.MainForm AppForm;

                AppRunning = new Common.Process.Running(System.Windows.Forms.Application.ProductName);

                try
                {
                    System.Windows.Forms.Application.EnableVisualStyles();
                    System.Windows.Forms.Application.SetCompatibleTextRenderingDefault(false);

                    if (AppRunning.AlreadyRunning == false)
                    {
                        Pep.Controls.Config.Initialize();

                        AppForm = new Pep.Forms.MainForm();

                        System.Windows.Forms.Application.AddMessageFilter(AppForm);

                        Common.Forms.Application.Run(AppForm);

                        System.Windows.Forms.Application.RemoveMessageFilter(AppForm);

                        Pep.Controls.Config.Uninitialize();
                    }
                    else
                    {
                        System.Windows.Forms.MessageBox.Show("The application is already running.",
                            System.Windows.Forms.Application.ProductName,
                            System.Windows.Forms.MessageBoxButtons.OK,
                            System.Windows.Forms.MessageBoxIcon.Error);
                    }
                }
                catch (Exception e)
                {
                    System.String sMsg;

                    sMsg = "Unhandled exception caught.  (" + e.Message +
                        ")\n\nThe application will now automatically close to prevent data loss.";

                    System.Windows.Forms.MessageBox.Show(sMsg,
                        System.Windows.Forms.Application.ProductName,
                        System.Windows.Forms.MessageBoxButtons.OK,
                        System.Windows.Forms.MessageBoxIcon.Error);
                }

                AppRunning.Close();
            }
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/
