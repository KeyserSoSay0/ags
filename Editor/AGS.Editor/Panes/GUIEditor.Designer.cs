namespace AGS.Editor
{
    partial class GUIEditor
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ctrlPanel = new System.Windows.Forms.Panel();
            this.lblTransparency = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.sldTransparency = new System.Windows.Forms.TrackBar();
            this.lblZoomInfo = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.sldZoomLevel = new System.Windows.Forms.TrackBar();
            this.bgPanel = new AGS.Editor.BufferedPanel();
            this.ctrlPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.sldTransparency)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.sldZoomLevel)).BeginInit();
            this.SuspendLayout();
            // 
            // ctrlPanel
            // 
            this.ctrlPanel.Controls.Add(this.lblTransparency);
            this.ctrlPanel.Controls.Add(this.label2);
            this.ctrlPanel.Controls.Add(this.sldTransparency);
            this.ctrlPanel.Controls.Add(this.lblZoomInfo);
            this.ctrlPanel.Controls.Add(this.label3);
            this.ctrlPanel.Controls.Add(this.sldZoomLevel);
            this.ctrlPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.ctrlPanel.Location = new System.Drawing.Point(0, 0);
            this.ctrlPanel.Name = "ctrlPanel";
            this.ctrlPanel.Size = new System.Drawing.Size(702, 51);
            this.ctrlPanel.TabIndex = 0;
            // 
            // lblTransparency
            // 
            this.lblTransparency.AutoSize = true;
            this.lblTransparency.Location = new System.Drawing.Point(560, 15);
            this.lblTransparency.Name = "lblTransparency";
            this.lblTransparency.Size = new System.Drawing.Size(33, 13);
            this.lblTransparency.TabIndex = 5;
            this.lblTransparency.Text = "100%";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(282, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(112, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Controls transparency:";
            // 
            // sldTransparency
            // 
            this.sldTransparency.LargeChange = 20;
            this.sldTransparency.Location = new System.Drawing.Point(400, 6);
            this.sldTransparency.Maximum = 100;
            this.sldTransparency.Name = "sldTransparency";
            this.sldTransparency.Size = new System.Drawing.Size(154, 42);
            this.sldTransparency.SmallChange = 5;
            this.sldTransparency.TabIndex = 4;
            this.sldTransparency.TickFrequency = 20;
            this.sldTransparency.Scroll += new System.EventHandler(this.sldTransparency_Scroll);
            // 
            // lblZoomInfo
            // 
            this.lblZoomInfo.AutoSize = true;
            this.lblZoomInfo.Location = new System.Drawing.Point(206, 15);
            this.lblZoomInfo.Name = "lblZoomInfo";
            this.lblZoomInfo.Size = new System.Drawing.Size(33, 13);
            this.lblZoomInfo.TabIndex = 2;
            this.lblZoomInfo.Text = "100%";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 15);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(37, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Zoom:";
            // 
            // sldZoomLevel
            // 
            this.sldZoomLevel.LargeChange = 1;
            this.sldZoomLevel.Location = new System.Drawing.Point(46, 6);
            this.sldZoomLevel.Minimum = 1;
            this.sldZoomLevel.Name = "sldZoomLevel";
            this.sldZoomLevel.Size = new System.Drawing.Size(154, 42);
            this.sldZoomLevel.TabIndex = 1;
            this.sldZoomLevel.Value = 1;
            this.sldZoomLevel.Scroll += new System.EventHandler(this.sldZoomLevel_Scroll);
            // 
            // bgPanel
            // 
            this.bgPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.bgPanel.AutoScroll = true;
            this.bgPanel.BackColor = System.Drawing.Color.Transparent;
            this.bgPanel.Location = new System.Drawing.Point(0, 57);
            this.bgPanel.Name = "bgPanel";
            this.bgPanel.Size = new System.Drawing.Size(702, 402);
            this.bgPanel.TabIndex = 1;
            this.bgPanel.TabStop = true;
            this.bgPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.bgPanel_Paint);
            this.bgPanel.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.bgPanel_MouseDoubleClick);
            this.bgPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.bgPanel_MouseDown);
            this.bgPanel.MouseEnter += new System.EventHandler(this.bgPanel_MouseEnter);
            this.bgPanel.MouseLeave += new System.EventHandler(this.bgPanel_MouseLeave);
            this.bgPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.bgPanel_MouseMove);
            this.bgPanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.bgPanel_MouseUp);
            // 
            // GUIEditor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.ctrlPanel);
            this.Controls.Add(this.bgPanel);
            this.Name = "GUIEditor";
            this.Size = new System.Drawing.Size(702, 459);
            this.Load += new System.EventHandler(this.GUIEditor_Load);
            this.ctrlPanel.ResumeLayout(false);
            this.ctrlPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.sldTransparency)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.sldZoomLevel)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private BufferedPanel bgPanel;
        private System.Windows.Forms.Panel ctrlPanel;
        private System.Windows.Forms.Label lblZoomInfo;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TrackBar sldZoomLevel;
        private System.Windows.Forms.Label lblTransparency;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TrackBar sldTransparency;
    }
}
