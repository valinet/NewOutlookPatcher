using Microsoft.Win32;

namespace gui
{
    internal static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            string tempFolderName = "";
            foreach (var arg in args)
            {
                tempFolderName = arg;
            }
            if (tempFolderName != "")
            {
                if (tempFolderName == "--uninstall")
                {
                    try
                    {
                        RegistryKey localMachine = RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.LocalMachine, RegistryView.Registry64);
                        var reg = localMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\olk.exe", true);
                        if (reg != null)
                        {
                            bool stillIsVerified = false;
                            var obj2 = reg.GetValue("VerifierDlls");
                            if (obj2 != null)
                            {
                                string verifierDlls = (string)obj2;
                                verifierDlls = verifierDlls.Replace(" NewOutlookPatcher.dll", "");
                                verifierDlls = verifierDlls.Replace("NewOutlookPatcher.dll ", "");
                                verifierDlls = verifierDlls.Replace("NewOutlookPatcher.dll", "");
                                if (verifierDlls == "")
                                {
                                    reg.DeleteValue("VerifierDlls");
                                }
                                else
                                {
                                    reg.SetValue("VerifierDlls", verifierDlls);
                                    stillIsVerified = true;
                                }
                            }

                            var obj1 = reg.GetValue("GlobalFlag");
                            if (obj1 != null)
                            {
                                int val = (int)obj1;
                                if (!stillIsVerified)
                                {
                                    val = val & ~0x100;
                                    if (val == 0)
                                    {
                                        reg.DeleteValue("GlobalFlag");
                                    }
                                    else
                                    {
                                        reg.SetValue("GlobalFlag", val);
                                    }
                                }
                            }

                            if (!(reg.SubKeyCount > 0 || reg.ValueCount > 0))
                            {
                                reg.Close();
                                localMachine.DeleteSubKeyTree("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\olk.exe", false);
                            }
                        }
                        File.Delete(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "NewOutlookPatcher.dll"));
                    }
                    catch { }
                }
                else
                {
                    try
                    {
                        File.Copy(Path.Combine(tempFolderName, "NewOutlookPatcher.dll"), Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "NewOutlookPatcher.dll"), true);
                    }
                    catch { }
                    finally
                    {
                        try
                        {
                            RegistryKey localMachine = RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.LocalMachine, RegistryView.Registry64);
                            var reg = localMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\olk.exe", true);
                            if (reg == null)
                            {
                                reg = localMachine.CreateSubKey("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\olk.exe");
                            }

                            var obj1 = reg.GetValue("GlobalFlag");
                            if (obj1 != null)
                            {
                                int val = (int)obj1;
                                val = val | 0x100;
                                reg.SetValue("GlobalFlag", val);
                            }
                            else
                            {
                                reg.SetValue("GlobalFlag", 0x100);
                            }

                            var obj2 = reg.GetValue("VerifierDlls");
                            if (obj2 != null)
                            {
                                string verifierDlls = (string)obj2;
                                if (!verifierDlls.Contains(" NewOutlookPatcher.dll") && !verifierDlls.Contains("NewOutlookPatcher.dll ") && verifierDlls != "NewOutlookPatcher.dll")
                                {
                                    verifierDlls += " NewOutlookPatcher.dll";
                                    reg.SetValue("VerifierDlls", verifierDlls);
                                }
                            }
                            else
                            {
                                reg.SetValue("VerifierDlls", "NewOutlookPatcher.dll");
                            }
                        }
                        catch { }
                    }
                }
                Environment.Exit(0);
            }
            // To customize application configuration such as set high DPI settings or default font,
            // see https://aka.ms/applicationconfiguration.
            ApplicationConfiguration.Initialize();
            Application.Run(new Form1());
        }
    }
}