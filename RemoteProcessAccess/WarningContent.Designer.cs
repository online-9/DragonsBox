namespace DragonsBox
{
    partial class WarningContent
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(WarningContent));
            this.ghostTheme1 = new GhostTheme.GhostTheme();
            this.hexBox1 = new Be.Windows.Forms.HexBox();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.dumpToHarddiskToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ghostControlBox1 = new GhostTheme.GhostControlBox();
            this.ghostTheme1.SuspendLayout();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // ghostTheme1
            // 
            this.ghostTheme1.BorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.ghostTheme1.Colors = new GhostTheme.Bloom[0];
            this.ghostTheme1.Controls.Add(this.hexBox1);
            this.ghostTheme1.Controls.Add(this.ghostControlBox1);
            this.ghostTheme1.Customization = "";
            this.ghostTheme1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ghostTheme1.Font = new System.Drawing.Font("Verdana", 8F);
            this.ghostTheme1.Image = null;
            this.ghostTheme1.Location = new System.Drawing.Point(0, 0);
            this.ghostTheme1.Movable = true;
            this.ghostTheme1.Name = "ghostTheme1";
            this.ghostTheme1.NoRounding = true;
            this.ghostTheme1.ShowIcon = true;
            this.ghostTheme1.Sizable = true;
            this.ghostTheme1.Size = new System.Drawing.Size(664, 368);
            this.ghostTheme1.SmartBounds = true;
            this.ghostTheme1.StartPosition = System.Windows.Forms.FormStartPosition.WindowsDefaultLocation;
            this.ghostTheme1.TabIndex = 1;
            this.ghostTheme1.Text = "DragonBox - Content viewer";
            this.ghostTheme1.TransparencyKey = System.Drawing.Color.Fuchsia;
            this.ghostTheme1.Transparent = false;
            // 
            // hexBox1
            // 
            this.hexBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.hexBox1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(18)))), ((int)(((byte)(18)))), ((int)(((byte)(18)))));
            this.hexBox1.ContextMenuStrip = this.contextMenuStrip1;
            this.hexBox1.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.hexBox1.ForeColor = System.Drawing.Color.White;
            this.hexBox1.LineInfoVisible = true;
            this.hexBox1.Location = new System.Drawing.Point(12, 28);
            this.hexBox1.Name = "hexBox1";
            this.hexBox1.ReadOnly = true;
            this.hexBox1.ShadowSelectionColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(60)))), ((int)(((byte)(188)))), ((int)(((byte)(255)))));
            this.hexBox1.Size = new System.Drawing.Size(640, 328);
            this.hexBox1.StringViewVisible = true;
            this.hexBox1.TabIndex = 1;
            this.hexBox1.UseFixedBytesPerLine = true;
            this.hexBox1.VScrollBarVisible = true;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.dumpToHarddiskToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(189, 48);
            // 
            // dumpToHarddiskToolStripMenuItem
            // 
            this.dumpToHarddiskToolStripMenuItem.Name = "dumpToHarddiskToolStripMenuItem";
            this.dumpToHarddiskToolStripMenuItem.Size = new System.Drawing.Size(188, 22);
            this.dumpToHarddiskToolStripMenuItem.Text = "Dump to Harddisk";
            this.dumpToHarddiskToolStripMenuItem.Click += new System.EventHandler(this.dumpToHarddiskToolStripMenuItem_Click);
            // 
            // ghostControlBox1
            // 
            this.ghostControlBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ghostControlBox1.Customization = "QEBA/wAAAP9aWlr/";
            this.ghostControlBox1.Font = new System.Drawing.Font("Verdana", 8F);
            this.ghostControlBox1.Image = null;
            this.ghostControlBox1.Location = new System.Drawing.Point(590, 3);
            this.ghostControlBox1.Name = "ghostControlBox1";
            this.ghostControlBox1.NoRounding = false;
            this.ghostControlBox1.Size = new System.Drawing.Size(71, 19);
            this.ghostControlBox1.TabIndex = 0;
            this.ghostControlBox1.Text = "ghostControlBox1";
            this.ghostControlBox1.Transparent = false;
            // 
            // WarningContent
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(664, 368);
            this.Controls.Add(this.ghostTheme1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "WarningContent";
            this.Text = "WarningContent";
            this.TransparencyKey = System.Drawing.Color.Fuchsia;
            this.Resize += new System.EventHandler(this.WarningContent_Resize);
            this.ghostTheme1.ResumeLayout(false);
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private GhostTheme.GhostTheme ghostTheme1;
        private GhostTheme.GhostControlBox ghostControlBox1;
        private Be.Windows.Forms.HexBox hexBox1;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem dumpToHarddiskToolStripMenuItem;
    }
}