/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/

using System;

namespace Pep
{
    namespace Forms
    {
        public partial class AboutForm : System.Windows.Forms.Form
        {
            #region "Constructor"
            public AboutForm()
            {
                InitializeComponent();

                //  Initialize the AboutBox to display the product information from the assembly information.
                //  Change assembly information settings for your application through either:
                //  - Project->Properties->Application->Assembly Information
                //  - AssemblyInfo.cs
                this.Text = String.Format("About {0}", "PEP"/*AssemblyTitle*/);
                this.labelProductName.Text = "PEP"/*AssemblyProduct*/;
                this.labelProductVersion.Text = String.Format("v{0}", AssemblyVersion);
                this.labelProductCopyright.Text = AssemblyCopyright;
            }
            #endregion

            #region "Properties"
            public string AssemblyTitle
            {
                get
                {
                    // Get all Title attributes on this assembly
                    object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyTitleAttribute), false);
                    // If there is at least one Title attribute
                    if (attributes.Length > 0)
                    {
                        // Select the first one
                        System.Reflection.AssemblyTitleAttribute titleAttribute = (System.Reflection.AssemblyTitleAttribute)attributes[0];
                        // If it is not an empty string, return it
                        if (titleAttribute.Title != "")
                            return titleAttribute.Title;
                    }
                    // If there was no Title attribute, or if the Title attribute was the empty string, return the .exe name
                    return System.IO.Path.GetFileNameWithoutExtension(System.Reflection.Assembly.GetExecutingAssembly().CodeBase);
                }
            }

            public string AssemblyVersion
            {
                get
                {
                    return System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString();
                }
            }

            public string AssemblyDescription
            {
                get
                {
                    // Get all Description attributes on this assembly
                    object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyDescriptionAttribute), false);
                    // If there aren't any Description attributes, return an empty string
                    if (attributes.Length == 0)
                        return "";
                    // If there is a Description attribute, return its value
                    return ((System.Reflection.AssemblyDescriptionAttribute)attributes[0]).Description;
                }
            }

            public string AssemblyProduct
            {
                get
                {
                    // Get all Product attributes on this assembly
                    object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyProductAttribute), false);
                    // If there aren't any Product attributes, return an empty string
                    if (attributes.Length == 0)
                        return "";
                    // If there is a Product attribute, return its value
                    return ((System.Reflection.AssemblyProductAttribute)attributes[0]).Product;
                }
            }

            public string AssemblyCopyright
            {
                get
                {
                    // Get all Copyright attributes on this assembly
                    object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyCopyrightAttribute), false);
                    // If there aren't any Copyright attributes, return an empty string
                    if (attributes.Length == 0)
                        return "";
                    // If there is a Copyright attribute, return its value
                    return ((System.Reflection.AssemblyCopyrightAttribute)attributes[0]).Copyright;
                }
            }

            public string AssemblyCompany
            {
                get
                {
                    // Get all Company attributes on this assembly
                    object[] attributes = System.Reflection.Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(System.Reflection.AssemblyCompanyAttribute), false);
                    // If there aren't any Company attributes, return an empty string
                    if (attributes.Length == 0)
                        return "";
                    // If there is a Company attribute, return its value
                    return ((System.Reflection.AssemblyCompanyAttribute)attributes[0]).Company;
                }
            }
            #endregion

            #region "Event Handlers"
            private void AboutForm_Load(object sender, EventArgs e)
            {
                System.Collections.Generic.List<Pep.Programmer.Plugin> PluginsList = Pep.Programmer.Devices.PluginsList;
                System.Windows.Forms.ListViewItem Item;
                System.Text.StringBuilder sb;

                if (PluginsList != null)
                {
                    listViewPlugins.BeginUpdate();

                    foreach (Pep.Programmer.Plugin Plugin in PluginsList)
                    {
                        Item = listViewPlugins.Items.Add(Plugin.Name);

                        sb = new System.Text.StringBuilder();

                        sb.Append("v");
                        sb.Append(Plugin.ProductMajorVersion);
                        sb.Append(".");
                        sb.Append(Plugin.ProductMinorVersion);
                        sb.Append(".");
                        sb.Append(Plugin.ProductBuildVersion);
                        sb.Append(".");
                        sb.Append(Plugin.ProductPrivateVersion);

                        Item.SubItems.Add(sb.ToString());
                    }

                    listViewPlugins.AutosizeColumns();
                    listViewPlugins.EndUpdate();
                }

                textBoxOther.AppendText("Espresso Logic Minimizer\n");
                textBoxOther.AppendText("Copyright (c) 1988, 1989, Regents of the University of California.\n");
                textBoxOther.AppendText("All rights reserved.\n");

                textBoxEmail.Text = "keshbach@comcast.net";
            }
            #endregion
        }
    }
}

/***************************************************************************/
/*  Copyright (C) 2006-2014 Kevin Eshbach                                  */
/***************************************************************************/
