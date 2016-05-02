namespace GenericHid
{
    partial class TestFrm
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
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.gyrYawLabel = new System.Windows.Forms.Label();
            this.gyrPitchLabel = new System.Windows.Forms.Label();
            this.gyrRollLabel = new System.Windows.Forms.Label();
            this.accZLabel = new System.Windows.Forms.Label();
            this.accYLabel = new System.Windows.Forms.Label();
            this.accXLabel = new System.Windows.Forms.Label();
            this.analogLabel = new System.Windows.Forms.Label();
            this.buttonLabel = new System.Windows.Forms.Label();
            this.tmrContinuousDataCollect = new System.Windows.Forms.Timer(this.components);
            this.squareCheck = new System.Windows.Forms.CheckBox();
            this.crossCheck = new System.Windows.Forms.CheckBox();
            this.triangleCheck = new System.Windows.Forms.CheckBox();
            this.circleCheck = new System.Windows.Forms.CheckBox();
            this.moveCheck = new System.Windows.Forms.CheckBox();
            this.tCheck = new System.Windows.Forms.CheckBox();
            this.startCheck = new System.Windows.Forms.CheckBox();
            this.selectCheck = new System.Windows.Forms.CheckBox();
            this.magXLabel = new System.Windows.Forms.Label();
            this.magYLabel = new System.Windows.Forms.Label();
            this.magZLabel = new System.Windows.Forms.Label();
            this.batLabel = new System.Windows.Forms.Label();
            this.tempLabel = new System.Windows.Forms.Label();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.gainBar = new System.Windows.Forms.TrackBar();
            this.label4 = new System.Windows.Forms.Label();
            this.exposureBar = new System.Windows.Forms.TrackBar();
            this.label5 = new System.Windows.Forms.Label();
            this.wbredBar = new System.Windows.Forms.TrackBar();
            this.label6 = new System.Windows.Forms.Label();
            this.wbgreenBar = new System.Windows.Forms.TrackBar();
            this.label7 = new System.Windows.Forms.Label();
            this.wbblueBar = new System.Windows.Forms.TrackBar();
            this.label8 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.gainBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.exposureBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbredBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbgreenBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbblueBar)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(12, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(480, 320);
            this.pictureBox1.TabIndex = 16;
            this.pictureBox1.TabStop = false;
            // 
            // gyrYawLabel
            // 
            this.gyrYawLabel.AutoSize = true;
            this.gyrYawLabel.Location = new System.Drawing.Point(510, 103);
            this.gyrYawLabel.Name = "gyrYawLabel";
            this.gyrYawLabel.Size = new System.Drawing.Size(33, 13);
            this.gyrYawLabel.TabIndex = 19;
            this.gyrYawLabel.Text = "Gyr Y";
            // 
            // gyrPitchLabel
            // 
            this.gyrPitchLabel.AutoSize = true;
            this.gyrPitchLabel.Location = new System.Drawing.Point(510, 90);
            this.gyrPitchLabel.Name = "gyrPitchLabel";
            this.gyrPitchLabel.Size = new System.Drawing.Size(33, 13);
            this.gyrPitchLabel.TabIndex = 20;
            this.gyrPitchLabel.Text = "Gyr P";
            // 
            // gyrRollLabel
            // 
            this.gyrRollLabel.AutoSize = true;
            this.gyrRollLabel.Location = new System.Drawing.Point(510, 77);
            this.gyrRollLabel.Name = "gyrRollLabel";
            this.gyrRollLabel.Size = new System.Drawing.Size(34, 13);
            this.gyrRollLabel.TabIndex = 17;
            this.gyrRollLabel.Text = "Gyr R";
            // 
            // accZLabel
            // 
            this.accZLabel.AutoSize = true;
            this.accZLabel.Location = new System.Drawing.Point(510, 64);
            this.accZLabel.Name = "accZLabel";
            this.accZLabel.Size = new System.Drawing.Size(36, 13);
            this.accZLabel.TabIndex = 18;
            this.accZLabel.Text = "Acc Z";
            // 
            // accYLabel
            // 
            this.accYLabel.AutoSize = true;
            this.accYLabel.Location = new System.Drawing.Point(510, 51);
            this.accYLabel.Name = "accYLabel";
            this.accYLabel.Size = new System.Drawing.Size(36, 13);
            this.accYLabel.TabIndex = 21;
            this.accYLabel.Text = "Acc Y";
            // 
            // accXLabel
            // 
            this.accXLabel.AutoSize = true;
            this.accXLabel.Location = new System.Drawing.Point(510, 38);
            this.accXLabel.Name = "accXLabel";
            this.accXLabel.Size = new System.Drawing.Size(36, 13);
            this.accXLabel.TabIndex = 24;
            this.accXLabel.Text = "Acc X";
            // 
            // analogLabel
            // 
            this.analogLabel.AutoSize = true;
            this.analogLabel.Location = new System.Drawing.Point(510, 25);
            this.analogLabel.Name = "analogLabel";
            this.analogLabel.Size = new System.Drawing.Size(40, 13);
            this.analogLabel.TabIndex = 23;
            this.analogLabel.Text = "Analog";
            // 
            // buttonLabel
            // 
            this.buttonLabel.AutoSize = true;
            this.buttonLabel.Location = new System.Drawing.Point(510, 12);
            this.buttonLabel.Name = "buttonLabel";
            this.buttonLabel.Size = new System.Drawing.Size(43, 13);
            this.buttonLabel.TabIndex = 22;
            this.buttonLabel.Text = "Buttons";
            // 
            // tmrContinuousDataCollect
            // 
            this.tmrContinuousDataCollect.Interval = 1;
            this.tmrContinuousDataCollect.Tick += new System.EventHandler(this.tmrContinuousDataCollect_Tick_1);
            // 
            // squareCheck
            // 
            this.squareCheck.AutoSize = true;
            this.squareCheck.Location = new System.Drawing.Point(629, 14);
            this.squareCheck.Name = "squareCheck";
            this.squareCheck.Size = new System.Drawing.Size(94, 17);
            this.squareCheck.TabIndex = 35;
            this.squareCheck.Text = "Square Button";
            this.squareCheck.UseVisualStyleBackColor = true;
            // 
            // crossCheck
            // 
            this.crossCheck.AutoSize = true;
            this.crossCheck.Location = new System.Drawing.Point(629, 37);
            this.crossCheck.Name = "crossCheck";
            this.crossCheck.Size = new System.Drawing.Size(86, 17);
            this.crossCheck.TabIndex = 35;
            this.crossCheck.Text = "Cross Button";
            this.crossCheck.UseVisualStyleBackColor = true;
            // 
            // triangleCheck
            // 
            this.triangleCheck.AutoSize = true;
            this.triangleCheck.Location = new System.Drawing.Point(629, 60);
            this.triangleCheck.Name = "triangleCheck";
            this.triangleCheck.Size = new System.Drawing.Size(98, 17);
            this.triangleCheck.TabIndex = 35;
            this.triangleCheck.Text = "Triangle Button";
            this.triangleCheck.UseVisualStyleBackColor = true;
            // 
            // circleCheck
            // 
            this.circleCheck.AutoSize = true;
            this.circleCheck.Location = new System.Drawing.Point(629, 83);
            this.circleCheck.Name = "circleCheck";
            this.circleCheck.Size = new System.Drawing.Size(86, 17);
            this.circleCheck.TabIndex = 35;
            this.circleCheck.Text = "Circle Button";
            this.circleCheck.UseVisualStyleBackColor = true;
            // 
            // moveCheck
            // 
            this.moveCheck.AutoSize = true;
            this.moveCheck.Location = new System.Drawing.Point(629, 106);
            this.moveCheck.Name = "moveCheck";
            this.moveCheck.Size = new System.Drawing.Size(87, 17);
            this.moveCheck.TabIndex = 35;
            this.moveCheck.Text = "Move Button";
            this.moveCheck.UseVisualStyleBackColor = true;
            // 
            // tCheck
            // 
            this.tCheck.AutoSize = true;
            this.tCheck.Location = new System.Drawing.Point(629, 129);
            this.tCheck.Name = "tCheck";
            this.tCheck.Size = new System.Drawing.Size(67, 17);
            this.tCheck.TabIndex = 35;
            this.tCheck.Text = "T Button";
            this.tCheck.UseVisualStyleBackColor = true;
            // 
            // startCheck
            // 
            this.startCheck.AutoSize = true;
            this.startCheck.Location = new System.Drawing.Point(629, 152);
            this.startCheck.Name = "startCheck";
            this.startCheck.Size = new System.Drawing.Size(82, 17);
            this.startCheck.TabIndex = 35;
            this.startCheck.Text = "Start Button";
            this.startCheck.UseVisualStyleBackColor = true;
            // 
            // selectCheck
            // 
            this.selectCheck.AutoSize = true;
            this.selectCheck.Location = new System.Drawing.Point(629, 175);
            this.selectCheck.Name = "selectCheck";
            this.selectCheck.Size = new System.Drawing.Size(90, 17);
            this.selectCheck.TabIndex = 35;
            this.selectCheck.Text = "Select Button";
            this.selectCheck.UseVisualStyleBackColor = true;
            // 
            // magXLabel
            // 
            this.magXLabel.AutoSize = true;
            this.magXLabel.Location = new System.Drawing.Point(511, 116);
            this.magXLabel.Name = "magXLabel";
            this.magXLabel.Size = new System.Drawing.Size(33, 13);
            this.magXLabel.TabIndex = 19;
            this.magXLabel.Text = "Gyr Y";
            // 
            // magYLabel
            // 
            this.magYLabel.AutoSize = true;
            this.magYLabel.Location = new System.Drawing.Point(510, 129);
            this.magYLabel.Name = "magYLabel";
            this.magYLabel.Size = new System.Drawing.Size(33, 13);
            this.magYLabel.TabIndex = 19;
            this.magYLabel.Text = "Gyr Y";
            // 
            // magZLabel
            // 
            this.magZLabel.AutoSize = true;
            this.magZLabel.Location = new System.Drawing.Point(510, 142);
            this.magZLabel.Name = "magZLabel";
            this.magZLabel.Size = new System.Drawing.Size(33, 13);
            this.magZLabel.TabIndex = 19;
            this.magZLabel.Text = "Gyr Y";
            // 
            // batLabel
            // 
            this.batLabel.AutoSize = true;
            this.batLabel.Location = new System.Drawing.Point(510, 155);
            this.batLabel.Name = "batLabel";
            this.batLabel.Size = new System.Drawing.Size(33, 13);
            this.batLabel.TabIndex = 19;
            this.batLabel.Text = "Gyr Y";
            // 
            // tempLabel
            // 
            this.tempLabel.AutoSize = true;
            this.tempLabel.Location = new System.Drawing.Point(510, 168);
            this.tempLabel.Name = "tempLabel";
            this.tempLabel.Size = new System.Drawing.Size(33, 13);
            this.tempLabel.TabIndex = 19;
            this.tempLabel.Text = "Gyr Y";
            // 
            // pictureBox2
            // 
            this.pictureBox2.Location = new System.Drawing.Point(12, 338);
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.Size = new System.Drawing.Size(480, 320);
            this.pictureBox2.TabIndex = 16;
            this.pictureBox2.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(501, 220);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 13);
            this.label1.TabIndex = 36;
            this.label1.Text = "Near";
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(539, 218);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(223, 22);
            this.progressBar1.TabIndex = 37;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(768, 220);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(22, 13);
            this.label2.TabIndex = 36;
            this.label2.Text = "Far";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(512, 202);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(35, 13);
            this.label3.TabIndex = 38;
            this.label3.Text = "label3";
            // 
            // gainBar
            // 
            this.gainBar.Location = new System.Drawing.Point(559, 283);
            this.gainBar.Maximum = 79;
            this.gainBar.Name = "gainBar";
            this.gainBar.Size = new System.Drawing.Size(222, 45);
            this.gainBar.TabIndex = 39;
            this.gainBar.TickFrequency = 10;
            this.gainBar.Value = 20;
            this.gainBar.ValueChanged += new System.EventHandler(this.cameraParameter_Changed);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(501, 293);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 13);
            this.label4.TabIndex = 40;
            this.label4.Text = "Gain";
            // 
            // exposureBar
            // 
            this.exposureBar.Location = new System.Drawing.Point(559, 320);
            this.exposureBar.Maximum = 511;
            this.exposureBar.Name = "exposureBar";
            this.exposureBar.Size = new System.Drawing.Size(222, 45);
            this.exposureBar.TabIndex = 39;
            this.exposureBar.TickFrequency = 50;
            this.exposureBar.Value = 50;
            this.exposureBar.ValueChanged += new System.EventHandler(this.cameraParameter_Changed);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(502, 320);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(51, 13);
            this.label5.TabIndex = 40;
            this.label5.Text = "Exposure";
            // 
            // wbredBar
            // 
            this.wbredBar.Location = new System.Drawing.Point(559, 357);
            this.wbredBar.Maximum = 255;
            this.wbredBar.Name = "wbredBar";
            this.wbredBar.Size = new System.Drawing.Size(222, 45);
            this.wbredBar.TabIndex = 39;
            this.wbredBar.TickFrequency = 20;
            this.wbredBar.Value = 50;
            this.wbredBar.ValueChanged += new System.EventHandler(this.cameraParameter_Changed);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(502, 357);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(48, 13);
            this.label6.TabIndex = 40;
            this.label6.Text = "WB Red";
            // 
            // wbgreenBar
            // 
            this.wbgreenBar.Location = new System.Drawing.Point(559, 389);
            this.wbgreenBar.Maximum = 255;
            this.wbgreenBar.Name = "wbgreenBar";
            this.wbgreenBar.Size = new System.Drawing.Size(222, 45);
            this.wbgreenBar.TabIndex = 39;
            this.wbgreenBar.TickFrequency = 20;
            this.wbgreenBar.Value = 50;
            this.wbgreenBar.ValueChanged += new System.EventHandler(this.cameraParameter_Changed);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(502, 389);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(57, 13);
            this.label7.TabIndex = 40;
            this.label7.Text = "WB Green";
            // 
            // wbblueBar
            // 
            this.wbblueBar.Location = new System.Drawing.Point(558, 421);
            this.wbblueBar.Maximum = 255;
            this.wbblueBar.Name = "wbblueBar";
            this.wbblueBar.Size = new System.Drawing.Size(222, 45);
            this.wbblueBar.TabIndex = 39;
            this.wbblueBar.TickFrequency = 20;
            this.wbblueBar.Value = 50;
            this.wbblueBar.ValueChanged += new System.EventHandler(this.cameraParameter_Changed);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(501, 421);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(49, 13);
            this.label8.TabIndex = 40;
            this.label8.Text = "WB Blue";
            // 
            // TestFrm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(793, 662);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.wbblueBar);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.wbgreenBar);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.wbredBar);
            this.Controls.Add(this.exposureBar);
            this.Controls.Add(this.gainBar);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.selectCheck);
            this.Controls.Add(this.startCheck);
            this.Controls.Add(this.tCheck);
            this.Controls.Add(this.moveCheck);
            this.Controls.Add(this.circleCheck);
            this.Controls.Add(this.triangleCheck);
            this.Controls.Add(this.crossCheck);
            this.Controls.Add(this.squareCheck);
            this.Controls.Add(this.tempLabel);
            this.Controls.Add(this.batLabel);
            this.Controls.Add(this.magZLabel);
            this.Controls.Add(this.magYLabel);
            this.Controls.Add(this.magXLabel);
            this.Controls.Add(this.gyrYawLabel);
            this.Controls.Add(this.gyrPitchLabel);
            this.Controls.Add(this.gyrRollLabel);
            this.Controls.Add(this.accZLabel);
            this.Controls.Add(this.accYLabel);
            this.Controls.Add(this.accXLabel);
            this.Controls.Add(this.analogLabel);
            this.Controls.Add(this.buttonLabel);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.pictureBox1);
            this.Name = "TestFrm";
            this.Text = "TestFrm";
            this.Load += new System.EventHandler(this.TestFrm_Load);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.TestFrm_FormClosed);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.gainBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.exposureBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbredBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbgreenBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.wbblueBar)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label gyrYawLabel;
        private System.Windows.Forms.Label gyrPitchLabel;
        private System.Windows.Forms.Label gyrRollLabel;
        private System.Windows.Forms.Label accZLabel;
        private System.Windows.Forms.Label accYLabel;
        private System.Windows.Forms.Label accXLabel;
        private System.Windows.Forms.Label analogLabel;
        private System.Windows.Forms.Label buttonLabel;
        public System.Windows.Forms.Timer tmrContinuousDataCollect;
        private System.Windows.Forms.CheckBox squareCheck;
        private System.Windows.Forms.CheckBox crossCheck;
        private System.Windows.Forms.CheckBox triangleCheck;
        private System.Windows.Forms.CheckBox circleCheck;
        private System.Windows.Forms.CheckBox moveCheck;
        private System.Windows.Forms.CheckBox tCheck;
        private System.Windows.Forms.CheckBox startCheck;
        private System.Windows.Forms.CheckBox selectCheck;
        private System.Windows.Forms.Label magXLabel;
        private System.Windows.Forms.Label magYLabel;
        private System.Windows.Forms.Label magZLabel;
        private System.Windows.Forms.Label batLabel;
        private System.Windows.Forms.Label tempLabel;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TrackBar gainBar;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TrackBar exposureBar;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TrackBar wbredBar;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TrackBar wbgreenBar;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TrackBar wbblueBar;
        private System.Windows.Forms.Label label8;
    }
}