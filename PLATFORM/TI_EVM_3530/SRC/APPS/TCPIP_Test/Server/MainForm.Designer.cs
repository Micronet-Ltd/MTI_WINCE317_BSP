namespace TCPIPServer
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.buttonTurnOn = new System.Windows.Forms.Button();
            this.buttonClearLog = new System.Windows.Forms.Button();
            this.listMessages = new System.Windows.Forms.ListView();
            this.columnTimestamp = new System.Windows.Forms.ColumnHeader();
            this.columnMessage = new System.Windows.Forms.ColumnHeader();
            this.contextMenuLog = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuLogStop = new System.Windows.Forms.ToolStripMenuItem();
            this.imageListMessages = new System.Windows.Forms.ImageList(this.components);
            this.notifyTrayIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.menuTray = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuRestore = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuTrayExit = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonExit = new System.Windows.Forms.Button();
            this.textPort = new System.Windows.Forms.TextBox();
            this.labelPort = new System.Windows.Forms.Label();
            this.errorProvider = new System.Windows.Forms.ErrorProvider(this.components);
            this.checkAlwaysOnTop = new System.Windows.Forms.CheckBox();
            this.timerStartServer = new System.Windows.Forms.Timer(this.components);
            this.timerCheckThreads = new System.Windows.Forms.Timer(this.components);
            this.contextMenuLog.SuspendLayout();
            this.menuTray.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).BeginInit();
            this.SuspendLayout();
            // 
            // buttonTurnOn
            // 
            this.buttonTurnOn.Location = new System.Drawing.Point(12, 12);
            this.buttonTurnOn.Name = "buttonTurnOn";
            this.buttonTurnOn.Size = new System.Drawing.Size(75, 23);
            this.buttonTurnOn.TabIndex = 0;
            this.buttonTurnOn.Text = "Start";
            this.buttonTurnOn.UseVisualStyleBackColor = true;
            this.buttonTurnOn.Click += new System.EventHandler(this.buttonTurnOn_Click);
            // 
            // buttonClearLog
            // 
            this.buttonClearLog.Location = new System.Drawing.Point(93, 12);
            this.buttonClearLog.Name = "buttonClearLog";
            this.buttonClearLog.Size = new System.Drawing.Size(75, 23);
            this.buttonClearLog.TabIndex = 2;
            this.buttonClearLog.Text = "Clear log";
            this.buttonClearLog.UseVisualStyleBackColor = true;
            this.buttonClearLog.Click += new System.EventHandler(this.buttonClearLog_Click);
            // 
            // listMessages
            // 
            this.listMessages.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listMessages.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnTimestamp,
            this.columnMessage});
            this.listMessages.ContextMenuStrip = this.contextMenuLog;
            this.listMessages.FullRowSelect = true;
            this.listMessages.GridLines = true;
            this.listMessages.Location = new System.Drawing.Point(12, 42);
            this.listMessages.Name = "listMessages";
            this.listMessages.Size = new System.Drawing.Size(499, 223);
            this.listMessages.SmallImageList = this.imageListMessages;
            this.listMessages.TabIndex = 3;
            this.listMessages.UseCompatibleStateImageBehavior = false;
            this.listMessages.View = System.Windows.Forms.View.Details;
            // 
            // columnTimestamp
            // 
            this.columnTimestamp.Text = "Time";
            // 
            // columnMessage
            // 
            this.columnMessage.Text = "Message";
            this.columnMessage.Width = 220;
            // 
            // contextMenuLog
            // 
            this.contextMenuLog.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuLogStop});
            this.contextMenuLog.Name = "contextMenuLog";
            this.contextMenuLog.Size = new System.Drawing.Size(136, 26);
            this.contextMenuLog.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuLog_Opening);
            // 
            // toolStripMenuLogStop
            // 
            this.toolStripMenuLogStop.Name = "toolStripMenuLogStop";
            this.toolStripMenuLogStop.Size = new System.Drawing.Size(135, 22);
            this.toolStripMenuLogStop.Text = "Stop thread";
            this.toolStripMenuLogStop.Click += new System.EventHandler(this.toolStripMenuLogStop_Click);
            // 
            // imageListMessages
            // 
            this.imageListMessages.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageListMessages.ImageStream")));
            this.imageListMessages.TransparentColor = System.Drawing.Color.Transparent;
            this.imageListMessages.Images.SetKeyName(0, "binary.ico");
            this.imageListMessages.Images.SetKeyName(1, "3floppy-mount.ico");
            this.imageListMessages.Images.SetKeyName(2, "info.ico");
            this.imageListMessages.Images.SetKeyName(3, "txt.ico");
            this.imageListMessages.Images.SetKeyName(4, "warning.ico");
            // 
            // notifyTrayIcon
            // 
            this.notifyTrayIcon.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.notifyTrayIcon.ContextMenuStrip = this.menuTray;
            this.notifyTrayIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyTrayIcon.Icon")));
            this.notifyTrayIcon.Text = "TCP/IP Server";
            this.notifyTrayIcon.Visible = true;
            this.notifyTrayIcon.BalloonTipClicked += new System.EventHandler(this.notifyTrayIcon_BalloonTipClicked);
            this.notifyTrayIcon.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.notifyTrayIcon_MouseDoubleClick);
            // 
            // menuTray
            // 
            this.menuTray.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuRestore,
            this.toolStripSeparator1,
            this.toolStripMenuTrayExit});
            this.menuTray.Name = "menuTray";
            this.menuTray.Size = new System.Drawing.Size(117, 54);
            // 
            // toolStripMenuRestore
            // 
            this.toolStripMenuRestore.Font = new System.Drawing.Font("Gisha", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.toolStripMenuRestore.Name = "toolStripMenuRestore";
            this.toolStripMenuRestore.Size = new System.Drawing.Size(116, 22);
            this.toolStripMenuRestore.Text = "Restore";
            this.toolStripMenuRestore.Click += new System.EventHandler(this.toolStripMenuRestore_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(113, 6);
            // 
            // toolStripMenuTrayExit
            // 
            this.toolStripMenuTrayExit.Image = global::TCPIPServer.Resources.Symbols_Error_icon;
            this.toolStripMenuTrayExit.Name = "toolStripMenuTrayExit";
            this.toolStripMenuTrayExit.Size = new System.Drawing.Size(116, 22);
            this.toolStripMenuTrayExit.Text = "&Exit";
            this.toolStripMenuTrayExit.Click += new System.EventHandler(this.toolStripMenuTrayExit_Click);
            // 
            // buttonExit
            // 
            this.buttonExit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonExit.Location = new System.Drawing.Point(436, 12);
            this.buttonExit.Name = "buttonExit";
            this.buttonExit.Size = new System.Drawing.Size(75, 23);
            this.buttonExit.TabIndex = 4;
            this.buttonExit.Text = "Exit";
            this.buttonExit.UseVisualStyleBackColor = true;
            this.buttonExit.Click += new System.EventHandler(this.buttonExit_Click);
            // 
            // textPort
            // 
            this.textPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.textPort.Location = new System.Drawing.Point(47, 271);
            this.textPort.Name = "textPort";
            this.textPort.Size = new System.Drawing.Size(71, 20);
            this.textPort.TabIndex = 8;
            this.textPort.Text = "5000";
            // 
            // labelPort
            // 
            this.labelPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(12, 274);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(29, 13);
            this.labelPort.TabIndex = 7;
            this.labelPort.Text = "Port:";
            // 
            // errorProvider
            // 
            this.errorProvider.ContainerControl = this;
            this.errorProvider.Icon = ((System.Drawing.Icon)(resources.GetObject("errorProvider.Icon")));
            // 
            // checkAlwaysOnTop
            // 
            this.checkAlwaysOnTop.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.checkAlwaysOnTop.AutoSize = true;
            this.checkAlwaysOnTop.Location = new System.Drawing.Point(419, 271);
            this.checkAlwaysOnTop.Name = "checkAlwaysOnTop";
            this.checkAlwaysOnTop.Size = new System.Drawing.Size(92, 17);
            this.checkAlwaysOnTop.TabIndex = 9;
            this.checkAlwaysOnTop.Text = "Always on top";
            this.checkAlwaysOnTop.UseVisualStyleBackColor = true;
            this.checkAlwaysOnTop.CheckedChanged += new System.EventHandler(this.checkAlwaysOnTop_CheckedChanged);
            // 
            // timerStartServer
            // 
            this.timerStartServer.Enabled = true;
            this.timerStartServer.Interval = 10;
            this.timerStartServer.Tick += new System.EventHandler(this.timerStartServer_Tick);
            // 
            // timerCheckThreads
            // 
            this.timerCheckThreads.Enabled = true;
            this.timerCheckThreads.Interval = 5000;
            this.timerCheckThreads.Tick += new System.EventHandler(this.timerCheckThreads_Tick);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(523, 300);
            this.Controls.Add(this.checkAlwaysOnTop);
            this.Controls.Add(this.listMessages);
            this.Controls.Add(this.textPort);
            this.Controls.Add(this.buttonExit);
            this.Controls.Add(this.buttonClearLog);
            this.Controls.Add(this.buttonTurnOn);
            this.Controls.Add(this.labelPort);
            this.DoubleBuffered = true;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(422, 338);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "TCIP/IP Server";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.Resize += new System.EventHandler(this.MainForm_Resize);
            this.contextMenuLog.ResumeLayout(false);
            this.menuTray.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonTurnOn;
        private System.Windows.Forms.Button buttonClearLog;
        private System.Windows.Forms.ListView listMessages;
        private System.Windows.Forms.ColumnHeader columnTimestamp;
        private System.Windows.Forms.ColumnHeader columnMessage;
        private System.Windows.Forms.ImageList imageListMessages;
        private System.Windows.Forms.NotifyIcon notifyTrayIcon;
        private System.Windows.Forms.Button buttonExit;
        private System.Windows.Forms.ContextMenuStrip menuTray;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuTrayExit;
        private System.Windows.Forms.TextBox textPort;
        private System.Windows.Forms.Label labelPort;
        private System.Windows.Forms.ErrorProvider errorProvider;
        private System.Windows.Forms.CheckBox checkAlwaysOnTop;
        private System.Windows.Forms.Timer timerStartServer;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuRestore;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ContextMenuStrip contextMenuLog;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuLogStop;
        private System.Windows.Forms.Timer timerCheckThreads;
    }
}

