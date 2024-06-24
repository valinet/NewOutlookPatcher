namespace gui
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            label1 = new Label();
            txtPath = new TextBox();
            label2 = new Label();
            chkDisableFirstMailAd = new CheckBox();
            chkDisableOneDriveBanner = new CheckBox();
            chkDisableWordIcon = new CheckBox();
            label3 = new Label();
            chkDisableExcelIcon = new CheckBox();
            chkDisablePowerPointIcon = new CheckBox();
            chkDisableOneDriveIcon = new CheckBox();
            chkDisableMoreAppsIcon = new CheckBox();
            btnApplyRestart = new Button();
            btnAbout = new Button();
            timerCheckOutlook = new System.Windows.Forms.Timer(components);
            chkDisableAll = new CheckBox();
            label4 = new Label();
            chkF12 = new CheckBox();
            label5 = new Label();
            chkDisableToDoIcon = new CheckBox();
            btnUninstall = new Button();
            SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(7, 8);
            label1.Name = "label1";
            label1.Size = new Size(163, 15);
            label1.TabIndex = 0;
            label1.Text = "New Outlook Install Location:";
            // 
            // txtPath
            // 
            txtPath.Enabled = false;
            txtPath.Location = new Point(12, 26);
            txtPath.Multiline = true;
            txtPath.Name = "txtPath";
            txtPath.Size = new Size(386, 58);
            txtPath.TabIndex = 1;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(7, 116);
            label2.Name = "label2";
            label2.Size = new Size(83, 15);
            label2.TabIndex = 2;
            label2.Text = "Advertisments";
            // 
            // chkDisableFirstMailAd
            // 
            chkDisableFirstMailAd.Checked = true;
            chkDisableFirstMailAd.CheckState = CheckState.Checked;
            chkDisableFirstMailAd.Location = new Point(12, 134);
            chkDisableFirstMailAd.Name = "chkDisableFirstMailAd";
            chkDisableFirstMailAd.Size = new Size(386, 53);
            chkDisableFirstMailAd.TabIndex = 3;
            chkDisableFirstMailAd.Text = "Disable ad as first item in e-mails list\r\nThe first item in the mailbox is always an ad, unless you pay for Microsoft 365. Use this to only show real e-mails in the list.";
            chkDisableFirstMailAd.UseVisualStyleBackColor = true;
            chkDisableFirstMailAd.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // chkDisableOneDriveBanner
            // 
            chkDisableOneDriveBanner.Checked = true;
            chkDisableOneDriveBanner.CheckState = CheckState.Checked;
            chkDisableOneDriveBanner.Location = new Point(12, 193);
            chkDisableOneDriveBanner.Name = "chkDisableOneDriveBanner";
            chkDisableOneDriveBanner.Size = new Size(386, 55);
            chkDisableOneDriveBanner.TabIndex = 4;
            chkDisableOneDriveBanner.Text = "Disable OneDrive banner\r\nIn the lower left corner, a OneDrive ad is displayed, unless you pay for Microsoft 365. Use this to hide that advertisment banner.";
            chkDisableOneDriveBanner.UseVisualStyleBackColor = true;
            chkDisableOneDriveBanner.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // chkDisableWordIcon
            // 
            chkDisableWordIcon.AutoSize = true;
            chkDisableWordIcon.Checked = true;
            chkDisableWordIcon.CheckState = CheckState.Checked;
            chkDisableWordIcon.Location = new Point(12, 271);
            chkDisableWordIcon.Name = "chkDisableWordIcon";
            chkDisableWordIcon.Size = new Size(122, 19);
            chkDisableWordIcon.TabIndex = 5;
            chkDisableWordIcon.Text = "Disable Word icon";
            chkDisableWordIcon.UseVisualStyleBackColor = true;
            chkDisableWordIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new Point(7, 251);
            label3.Name = "label3";
            label3.Size = new Size(108, 15);
            label3.TabIndex = 6;
            label3.Text = "Product placement";
            // 
            // chkDisableExcelIcon
            // 
            chkDisableExcelIcon.AutoSize = true;
            chkDisableExcelIcon.Checked = true;
            chkDisableExcelIcon.CheckState = CheckState.Checked;
            chkDisableExcelIcon.Location = new Point(12, 296);
            chkDisableExcelIcon.Name = "chkDisableExcelIcon";
            chkDisableExcelIcon.Size = new Size(120, 19);
            chkDisableExcelIcon.TabIndex = 7;
            chkDisableExcelIcon.Text = "Disable Excel icon";
            chkDisableExcelIcon.UseVisualStyleBackColor = true;
            chkDisableExcelIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // chkDisablePowerPointIcon
            // 
            chkDisablePowerPointIcon.AutoSize = true;
            chkDisablePowerPointIcon.Checked = true;
            chkDisablePowerPointIcon.CheckState = CheckState.Checked;
            chkDisablePowerPointIcon.Location = new Point(12, 321);
            chkDisablePowerPointIcon.Name = "chkDisablePowerPointIcon";
            chkDisablePowerPointIcon.Size = new Size(154, 19);
            chkDisablePowerPointIcon.TabIndex = 8;
            chkDisablePowerPointIcon.Text = "Disable PowerPoint icon";
            chkDisablePowerPointIcon.UseVisualStyleBackColor = true;
            chkDisablePowerPointIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // chkDisableOneDriveIcon
            // 
            chkDisableOneDriveIcon.AutoSize = true;
            chkDisableOneDriveIcon.Checked = true;
            chkDisableOneDriveIcon.CheckState = CheckState.Checked;
            chkDisableOneDriveIcon.Location = new Point(213, 296);
            chkDisableOneDriveIcon.Name = "chkDisableOneDriveIcon";
            chkDisableOneDriveIcon.Size = new Size(142, 19);
            chkDisableOneDriveIcon.TabIndex = 9;
            chkDisableOneDriveIcon.Text = "Disable OneDrive icon";
            chkDisableOneDriveIcon.UseVisualStyleBackColor = true;
            chkDisableOneDriveIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // chkDisableMoreAppsIcon
            // 
            chkDisableMoreAppsIcon.AutoSize = true;
            chkDisableMoreAppsIcon.Checked = true;
            chkDisableMoreAppsIcon.CheckState = CheckState.Checked;
            chkDisableMoreAppsIcon.Location = new Point(213, 321);
            chkDisableMoreAppsIcon.Name = "chkDisableMoreAppsIcon";
            chkDisableMoreAppsIcon.Size = new Size(149, 19);
            chkDisableMoreAppsIcon.TabIndex = 10;
            chkDisableMoreAppsIcon.Text = "Disable More apps icon";
            chkDisableMoreAppsIcon.UseVisualStyleBackColor = true;
            chkDisableMoreAppsIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // btnApplyRestart
            // 
            btnApplyRestart.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            btnApplyRestart.Enabled = false;
            btnApplyRestart.Location = new Point(300, 428);
            btnApplyRestart.Name = "btnApplyRestart";
            btnApplyRestart.Size = new Size(98, 29);
            btnApplyRestart.TabIndex = 11;
            btnApplyRestart.Text = "&Install";
            btnApplyRestart.UseVisualStyleBackColor = true;
            btnApplyRestart.Click += btnApplyRestart_Click;
            // 
            // btnAbout
            // 
            btnAbout.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            btnAbout.Location = new Point(12, 428);
            btnAbout.Name = "btnAbout";
            btnAbout.Size = new Size(98, 29);
            btnAbout.TabIndex = 12;
            btnAbout.Text = "&About";
            btnAbout.UseVisualStyleBackColor = true;
            btnAbout.Click += btnAbout_Click;
            // 
            // timerCheckOutlook
            // 
            timerCheckOutlook.Enabled = true;
            timerCheckOutlook.Interval = 1000;
            timerCheckOutlook.Tick += timerCheckOutlook_Tick;
            // 
            // chkDisableAll
            // 
            chkDisableAll.AutoSize = true;
            chkDisableAll.Checked = true;
            chkDisableAll.CheckState = CheckState.Checked;
            chkDisableAll.Location = new Point(12, 90);
            chkDisableAll.Name = "chkDisableAll";
            chkDisableAll.Size = new Size(120, 19);
            chkDisableAll.TabIndex = 13;
            chkDisableAll.Text = "Toggle everything";
            chkDisableAll.UseVisualStyleBackColor = true;
            chkDisableAll.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Location = new Point(7, 343);
            label4.Name = "label4";
            label4.Size = new Size(37, 15);
            label4.TabIndex = 14;
            label4.Text = "Other";
            // 
            // chkF12
            // 
            chkF12.AutoSize = true;
            chkF12.Checked = true;
            chkF12.CheckState = CheckState.Checked;
            chkF12.Location = new Point(12, 361);
            chkF12.Name = "chkF12";
            chkF12.Size = new Size(165, 19);
            chkF12.TabIndex = 15;
            chkF12.Text = "F12 opens Developer Tools";
            chkF12.UseVisualStyleBackColor = true;
            chkF12.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // label5
            // 
            label5.Location = new Point(7, 392);
            label5.Name = "label5";
            label5.Size = new Size(391, 33);
            label5.TabIndex = 16;
            label5.Text = "Pressing \"Install\" will close Outlook (olk.exe), apply your settings and restart Outlook (olk.exe) for you.";
            // 
            // chkDisableToDoIcon
            // 
            chkDisableToDoIcon.AutoSize = true;
            chkDisableToDoIcon.Checked = true;
            chkDisableToDoIcon.CheckState = CheckState.Checked;
            chkDisableToDoIcon.Location = new Point(213, 271);
            chkDisableToDoIcon.Name = "chkDisableToDoIcon";
            chkDisableToDoIcon.Size = new Size(123, 19);
            chkDisableToDoIcon.TabIndex = 17;
            chkDisableToDoIcon.Text = "Disable To Do icon";
            chkDisableToDoIcon.UseVisualStyleBackColor = true;
            chkDisableToDoIcon.CheckedChanged += chkDisableAll_CheckedChanged;
            // 
            // btnUninstall
            // 
            btnUninstall.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            btnUninstall.Enabled = false;
            btnUninstall.Location = new Point(196, 428);
            btnUninstall.Name = "btnUninstall";
            btnUninstall.Size = new Size(98, 29);
            btnUninstall.TabIndex = 18;
            btnUninstall.Text = "&Uninstall";
            btnUninstall.UseVisualStyleBackColor = true;
            btnUninstall.Click += btnApplyRestart_Click;
            // 
            // Form1
            // 
            AcceptButton = btnApplyRestart;
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(410, 469);
            Controls.Add(btnUninstall);
            Controls.Add(chkDisableToDoIcon);
            Controls.Add(label5);
            Controls.Add(chkF12);
            Controls.Add(label4);
            Controls.Add(chkDisableAll);
            Controls.Add(btnAbout);
            Controls.Add(btnApplyRestart);
            Controls.Add(chkDisableMoreAppsIcon);
            Controls.Add(chkDisableOneDriveIcon);
            Controls.Add(chkDisablePowerPointIcon);
            Controls.Add(chkDisableExcelIcon);
            Controls.Add(label3);
            Controls.Add(chkDisableWordIcon);
            Controls.Add(chkDisableOneDriveBanner);
            Controls.Add(chkDisableFirstMailAd);
            Controls.Add(label2);
            Controls.Add(txtPath);
            Controls.Add(label1);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            KeyPreview = true;
            Margin = new Padding(3, 2, 3, 2);
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "Form1";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "NewOutlookPatcher";
            TopMost = true;
            Load += Form1_Load;
            KeyDown += Form1_KeyDown;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label1;
        private TextBox txtPath;
        private Label label2;
        private CheckBox chkDisableFirstMailAd;
        private CheckBox chkDisableOneDriveBanner;
        private CheckBox chkDisableWordIcon;
        private Label label3;
        private CheckBox chkDisableExcelIcon;
        private CheckBox chkDisablePowerPointIcon;
        private CheckBox chkDisableOneDriveIcon;
        private CheckBox chkDisableMoreAppsIcon;
        private Button btnApplyRestart;
        private Button btnAbout;
        private System.Windows.Forms.Timer timerCheckOutlook;
        private CheckBox chkDisableAll;
        private Label label4;
        private CheckBox chkF12;
        private Label label5;
        private CheckBox chkDisableToDoIcon;
        private Button btnUninstall;
    }
}
