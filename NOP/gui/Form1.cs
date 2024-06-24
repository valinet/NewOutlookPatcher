using Microsoft.VisualBasic.ApplicationServices;
using Microsoft.Win32;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
using System.Windows.Forms.VisualStyles;
using System.Xml.Linq;

namespace gui
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool SetForegroundWindow(IntPtr hWnd);
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool IsIconic(IntPtr hWnd);
        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        [DllImport("user32")]
        public static extern UInt32 SendMessage(IntPtr hWnd, UInt32 msg, UInt32 wParam, UInt32 lParam);

        internal const int BCM_FIRST = 0x1600; //Normal button
        internal const int BCM_SETSHIELD = (BCM_FIRST + 0x000C); //Elevated button

        // https://stackoverflow.com/questions/33285468/display-a-uac-shield-icon-next-to-a-wpf-button
        static internal void AddShieldToButton(Button b)
        {
            b.FlatStyle = FlatStyle.System;
            SendMessage(b.Handle, BCM_SETSHIELD, 0, 0xFFFFFFFF);
        }

        // https://stackoverflow.com/questions/11660184/c-sharp-check-if-run-as-administrator
        public static bool IsAdministrator()
        {
            var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        // https://stackoverflow.com/questions/10702514/most-efficient-way-to-replace-one-sequence-of-the-bytes-with-some-other-sequence
        private static byte[] BytesReplace(byte[] input, byte[] pattern, byte[] replacement, ref int numMatches, bool keepSize = true)
        {
            if (pattern.Length == 0)
            {
                return input;
            }

            List<byte> result = new List<byte>();

            int i;

            for (i = 0; i <= input.Length - pattern.Length; i++)
            {
                bool foundMatch = true;
                for (int j = 0; j < pattern.Length; j++)
                {
                    if (input[i + j] != pattern[j])
                    {
                        foundMatch = false;
                        break;
                    }
                }

                if (foundMatch)
                {
                    numMatches++;
                    result.AddRange(replacement);
                    i += keepSize ? (replacement.Length - 1) : (pattern.Length - 1);
                }
                else
                {
                    result.Add(input[i]);
                }
            }

            for (; i < input.Length; i++)
            {
                result.Add(input[i]);
            }

            return result.ToArray();
        }

        public static bool IsPatcherInstalled()
        {
            bool isFileDropped = File.Exists(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "NewOutlookPatcher.dll"));
            bool isVerifierEnabled = false;
            bool isSetAsVerifierDll = false;

            RegistryKey localMachine = RegistryKey.OpenBaseKey(Microsoft.Win32.RegistryHive.LocalMachine, RegistryView.Registry64);
            var reg = localMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\olk.exe", false);
            if (reg != null)
            {
                var obj1 = reg.GetValue("GlobalFlag");
                if (obj1 != null)
                {
                    isVerifierEnabled = ((((int)obj1) & 0x100) == 0x100);
                }
                var obj2 = reg.GetValue("VerifierDlls");
                if (obj2 != null)
                {
                    string verifierDlls = (string)obj2;
                    isSetAsVerifierDll = verifierDlls.Contains("NewOutlookPatcher.dll");
                }
            }
            return isFileDropped && isVerifierEnabled && isSetAsVerifierDll;
        }

        public void FormatUI(bool patcherInstalled)
        {
            if (patcherInstalled)
            {
                btnApplyRestart.Text = "&Apply";
                btnUninstall.Enabled = true;
                label5.Text = label5.Text.Replace("Install", "Apply");
            }
            else
            {
                btnApplyRestart.Text = "&Install";
                btnUninstall.Enabled = false;
                label5.Text = label5.Text.Replace("Apply", "Install");
            }
        }

        public Form1()
        {
            InitializeComponent();
            if (!IsAdministrator()) AddShieldToButton(btnApplyRestart);
            if (!IsAdministrator()) AddShieldToButton(btnUninstall);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Update UI to reflect install status
            FormatUI(IsPatcherInstalled());

            // Load current settings
            if (IsPatcherInstalled())
            {
                byte[] buffer = File.ReadAllBytes(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "NewOutlookPatcher.dll"));
                int numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("#OwaContainer, #OwaContainerSlot1, "), Encoding.Unicode.GetBytes("                                   "), ref numMatches);
                chkDisableFirstMailAd.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes(".syTot, "), Encoding.Unicode.GetBytes("        "), ref numMatches);
                chkDisableOneDriveBanner.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='34318026-c018-414b-abb3-3e32dfb9cc4c'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                chkDisableWordIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='c5251a9b-a95d-4595-91ee-a39e6eed3db2'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                chkDisableExcelIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='48cb9ead-1c19-4e1f-8ed9-3d60a7e52b18'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                chkDisablePowerPointIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='59391057-d7d7-49fd-a041-d8e4080f05ec'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                chkDisableToDoIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='39109bd4-9389-4731-b8d6-7cc1a128d0b3'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                chkDisableOneDriveIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes(".___1fkhojs.f22iagw.f122n59.f1vx9l62.f1c21dwh.fqerorx.f1i5mqs4, "), Encoding.Unicode.GetBytes("                                                                "), ref numMatches);
                chkDisableMoreAppsIcon.Checked = (numMatches > 0);
                numMatches = 0;
                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("y_1A36CD25-E20F-4D0D-B1E6-3CC4307E1488"), Encoding.Unicode.GetBytes("n"), ref numMatches);
                chkF12.Checked = (numMatches > 0);
            }

            Process[] processes = Process.GetProcessesByName("olk");
            if (processes.Length > 0 && processes[0].MainModule != null)
            {
                if (IsIconic(processes[0].MainWindowHandle)) ShowWindow(processes[0].MainWindowHandle, 9);
                SetForegroundWindow(processes[0].MainWindowHandle);
            }
            else
            {
                ProcessStartInfo psi = new ProcessStartInfo();
                psi.FileName = "shell:AppsFolder\\Microsoft.OutlookForWindows_8wekyb3d8bbwe!Microsoft.OutlookforWindows";
                psi.UseShellExecute = true;
                System.Diagnostics.Process.Start(psi);
            }
            this.TopMost = true;
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape) this.Close();
        }

        private void btnAbout_Click(object sender, EventArgs e)
        {
            var ver = Assembly.GetExecutingAssembly().GetName().Version;
            if (ver != null)
            {
                var text = string.Format(
@"NewOutlookPatcher
Version {0:D4}.{1:D2}.{2:D2}.{3:D2}

Copyright 2024 VALINET Solutions SRL. All rights reserved.
Proudly engineered by Valentin-Gabriel Radu.",
                    ver.Major, ver.Minor, ver.Build, ver.Revision);
                MessageBox.Show(text, "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
            }
        }

        private void timerCheckOutlook_Tick(object sender, EventArgs e)
        {
            Process[] processes = Process.GetProcessesByName("olk");
            if (processes.Length > 0 && processes[0].MainModule != null)
            {
                txtPath.Text = Path.GetDirectoryName(processes[0].MainModule.FileName);
                btnApplyRestart.Enabled = true;
                timerCheckOutlook.Enabled = false;
            }
        }

        private void btnApplyRestart_Click(object sender, EventArgs e)
        {
            this.TopMost = false;

            bool uninstall = ((!chkDisableFirstMailAd.Checked && !chkDisableOneDriveBanner.Checked && !chkDisableWordIcon.Checked && !chkDisableExcelIcon.Checked && !chkDisablePowerPointIcon.Checked && !chkDisableToDoIcon.Checked && !chkDisableOneDriveIcon.Checked && !chkDisableMoreAppsIcon.Checked && !chkF12.Checked) || (sender == btnUninstall));

            string exeName = "Press Yes to apply new settings to olk.exe";
            if (!IsPatcherInstalled()) exeName = "Press Yes to install patcher in olk.exe";
            if (uninstall) exeName = "Press Yes to uninstall patcher from olk.exe";

            // Create scratch dir
            string tempFolderPath = Path.GetTempPath();
            string tempFolderName = Path.Combine(tempFolderPath, Guid.NewGuid().ToString());
            try
            {
                Directory.CreateDirectory(tempFolderName);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Unable to create a working directory.", "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.TopMost = true;
                return;
            }

            // Extract worker to scratch dir
            if (!uninstall) {
                Assembly assembly = Assembly.GetExecutingAssembly();
                string workerResourceName = "gui.dxgi.dll";
                string workerPath = Path.Combine(tempFolderName, "NewOutlookPatcher.dll");
                try
                {
                    using (Stream resourceStream = assembly.GetManifestResourceStream(workerResourceName))
                    {
                        if (resourceStream != null)
                        {
                            int numMatches = 0;
                            byte[] buffer = new byte[resourceStream.Length];
                            resourceStream.Read(buffer, 0, buffer.Length);
                            if (!chkDisableFirstMailAd.Checked)
                            {
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("#OwaContainer, #OwaContainerSlot1, "), Encoding.Unicode.GetBytes("                                   "), ref numMatches);
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes(".kk1xx._Bfyd.iIsOF.IjQyD, .kk1xx.lHRXq.iIsOF.IjQyD, "), Encoding.Unicode.GetBytes("                                                    "), ref numMatches);
                            }
                            if (!chkDisableOneDriveBanner.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes(".syTot, "), Encoding.Unicode.GetBytes("        "), ref numMatches);
                            if (!chkDisableWordIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='34318026-c018-414b-abb3-3e32dfb9cc4c'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                            if (!chkDisableExcelIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='c5251a9b-a95d-4595-91ee-a39e6eed3db2'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                            if (!chkDisablePowerPointIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='48cb9ead-1c19-4e1f-8ed9-3d60a7e52b18'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                            if (!chkDisableToDoIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='59391057-d7d7-49fd-a041-d8e4080f05ec'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                            if (!chkDisableOneDriveIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("[id='39109bd4-9389-4731-b8d6-7cc1a128d0b3'], "), Encoding.Unicode.GetBytes("                                             "), ref numMatches);
                            if (!chkDisableMoreAppsIcon.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes(".___1fkhojs.f22iagw.f122n59.f1vx9l62.f1c21dwh.fqerorx.f1i5mqs4, "), Encoding.Unicode.GetBytes("                                                                "), ref numMatches);
                            if (!chkF12.Checked)
                                buffer = BytesReplace(buffer, Encoding.Unicode.GetBytes("y_1A36CD25-E20F-4D0D-B1E6-3CC4307E1488"), Encoding.Unicode.GetBytes("n"), ref numMatches);

                            File.WriteAllBytes(workerPath, buffer);
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Unable to extract worker resource.", "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    this.TopMost = true;
                    return;
                }
            }

            // Customize UAC prompt
            try
            {
                File.Copy(Application.ExecutablePath, Path.Combine(tempFolderName, exeName), true);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Unable to extract driver resource.", "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.TopMost = true;
                return;
            }

            // Patch with selected options
            try
            {
                Process[] processes = Process.GetProcessesByName("olk");
                if (processes.Length > 0 && processes[0].MainModule != null)
                {
                    processes[0].CloseMainWindow();
                    processes[0].WaitForExit(5000);
                    processes[0].Kill();
                }
                ProcessStartInfo psi = new ProcessStartInfo();
                psi.FileName = Path.Combine(tempFolderName, exeName);
                psi.Arguments = uninstall ? "--uninstall" : ("--install \"" + tempFolderName + "\"");
                psi.UseShellExecute = true;
                psi.Verb = "runas";
                System.Diagnostics.Process.Start(psi).WaitForExit();
            }
            catch (Exception ex)
            {
                //MessageBox.Show("Unable to launch elevated patcher.", "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.TopMost = true;
                return;
            }

            // Restart olk
            try
            {
                ProcessStartInfo psi2 = new ProcessStartInfo();
                psi2.FileName = "shell:AppsFolder\\Microsoft.OutlookForWindows_8wekyb3d8bbwe!Microsoft.OutlookforWindows";
                psi2.UseShellExecute = true;
                System.Diagnostics.Process.Start(psi2);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Unable to restart Outlook (new).", "NewOutlookPatcher", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.TopMost = true;
                return;
            }

            // Delete scratch dir
            try
            {
                Directory.Delete(tempFolderName, true);
            }
            catch (Exception ex)
            {

            }

            // Update UI to reflect install status
            FormatUI(IsPatcherInstalled());

            this.TopMost = true;
        }

        bool disableChecks = false;
        private void chkDisableAll_CheckedChanged(object sender, EventArgs e)
        {
            if (!disableChecks)
            {
                disableChecks = true;
                if (sender == chkDisableAll)
                {
                    chkDisableFirstMailAd.Checked = chkDisableAll.Checked;
                    chkDisableOneDriveBanner.Checked = chkDisableAll.Checked;
                    chkDisableWordIcon.Checked = chkDisableAll.Checked;
                    chkDisableExcelIcon.Checked = chkDisableAll.Checked;
                    chkDisablePowerPointIcon.Checked = chkDisableAll.Checked;
                    chkDisableToDoIcon.Checked = chkDisableAll.Checked;
                    chkDisableOneDriveIcon.Checked = chkDisableAll.Checked;
                    chkDisableMoreAppsIcon.Checked = chkDisableAll.Checked;
                    chkF12.Checked = chkDisableAll.Checked;
                }
                else
                {
                    bool all = chkDisableFirstMailAd.Checked && chkDisableOneDriveBanner.Checked && chkDisableWordIcon.Checked && chkDisableExcelIcon.Checked && chkDisablePowerPointIcon.Checked && chkDisableToDoIcon.Checked && chkDisableOneDriveIcon.Checked && chkDisableMoreAppsIcon.Checked && chkF12.Checked;
                    if (all) chkDisableAll.Checked = true;
                    else chkDisableAll.Checked = false;
                }
                disableChecks = false;
            }
        }
    }
}
