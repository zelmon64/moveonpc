using System;
using System.Collections.Generic;
using System.Text;

namespace GenericHid
{
    public struct PSMoveOutputReport
    {
        public int r, g, b;
        public int rumble;
        public int framecount;
    }

    public struct PSMoveInputReport
    {
        public int Buttons;
        public int TAnalog;
        public int RawBattery;
        public int RawForceX, RawForceY, RawForceZ;
        public int RawGyroYaw, RawGyroPitch, RawGyroRoll;
        public int RawTemperature;
        public int RawMagnetX, RawMagnetY, RawMagnetZ;
    }

    public struct PSMoveButtons
    {
        public bool MOVE_TRIANGLE_BUTTON { get; set; }
        public bool MOVE_SQUARE_BUTTON { get; set; }
        public bool MOVE_CIRCLE_BUTTON { get; set; }
        public bool MOVE_CROSS_BUTTON { get; set; }
        public bool MOVE_MOVE_BUTTON { get; set; }
        public int MOVE_T_BUTTON { get; set; }
        public bool MOVE_START_BUTTON { get; set; }
        public bool MOVE_SELECT_BUTTON { get; set; }

    }

    public class PSMove
    {
        private PSMoveOutputReport outputReport;

        public PSMoveInputReport inputReport;
        public PSMoveInputReport previnputReport;
        public PSMoveButtons psButtons;

       
        public PSMove(){}

        public void setInputReports(Byte[] inFeatureReportBuffer)
        {
            inputReport.Buttons = inFeatureReportBuffer[3] | (inFeatureReportBuffer[2] << 8) |
                            ((inFeatureReportBuffer[4] & 1) << 16) | ((inFeatureReportBuffer[5] & 0xF0) << 13);

            if ((inputReport.Buttons & 0x80) == 0x80)   psButtons.MOVE_SQUARE_BUTTON = true;            
            else psButtons.MOVE_SQUARE_BUTTON = false;
            if ((inputReport.Buttons & 0x40) == 0x40) psButtons.MOVE_CROSS_BUTTON = true;
            else psButtons.MOVE_CROSS_BUTTON = false;
            if ((inputReport.Buttons & 0x20) == 0x20) psButtons.MOVE_CIRCLE_BUTTON = true;
            else psButtons.MOVE_CIRCLE_BUTTON = false;
            if ((inputReport.Buttons & 0x10) == 0x10) psButtons.MOVE_TRIANGLE_BUTTON = true;
            else psButtons.MOVE_TRIANGLE_BUTTON = false;
            if ((inputReport.Buttons & 0x100) == 0x100) psButtons.MOVE_SELECT_BUTTON = true;
            else psButtons.MOVE_SELECT_BUTTON = false;
            if ((inputReport.Buttons & 0x800) == 0x800) psButtons.MOVE_START_BUTTON = true;
            else psButtons.MOVE_START_BUTTON = false;
            if ((inputReport.Buttons & 0x80000) == 0x80000) psButtons.MOVE_MOVE_BUTTON = true;
            else psButtons.MOVE_MOVE_BUTTON = false;
            if ((inputReport.Buttons & 0x100000) == 0x100000) psButtons.MOVE_T_BUTTON = inFeatureReportBuffer[7];
            else psButtons.MOVE_T_BUTTON = 0;


            inputReport.TAnalog = inFeatureReportBuffer[7];
            inputReport.RawBattery = inFeatureReportBuffer[13];
            inputReport.RawForceX = inFeatureReportBuffer[21] * 256 + inFeatureReportBuffer[20] - 32768;
            inputReport.RawForceY = inFeatureReportBuffer[23] * 256 + inFeatureReportBuffer[22] - 32768;
            inputReport.RawForceZ = inFeatureReportBuffer[25] * 256 + inFeatureReportBuffer[24] - 32768;
            inputReport.RawGyroPitch = inFeatureReportBuffer[33] * 256 + inFeatureReportBuffer[32] - 32768;
            inputReport.RawGyroRoll = inFeatureReportBuffer[35] * 256 + inFeatureReportBuffer[34] - 32768;
            inputReport.RawGyroYaw = 32768 - (inFeatureReportBuffer[37] * 256 + inFeatureReportBuffer[36]);

            inputReport.RawTemperature = inFeatureReportBuffer[38] * 16 + (inFeatureReportBuffer[39] >> 4);
            if (inputReport.RawTemperature > 0x800) inputReport.RawTemperature -= 0x1000;
            inputReport.RawMagnetX = (inFeatureReportBuffer[39] & 15) * 256 + inFeatureReportBuffer[40];
            if (inputReport.RawMagnetX > 0x800) inputReport.RawMagnetX -= 0x1000;
            inputReport.RawMagnetY = inFeatureReportBuffer[41] * 16 + (inFeatureReportBuffer[42] >> 4);
            if (inputReport.RawMagnetY > 0x800) inputReport.RawMagnetY -= 0x1000;
            inputReport.RawMagnetZ = (inFeatureReportBuffer[42] & 15) * 256 + inFeatureReportBuffer[43];
            if (inputReport.RawMagnetZ > 0x800) inputReport.RawMagnetZ -= 0x1000;

            previnputReport.TAnalog = inFeatureReportBuffer[6];
            previnputReport.RawForceX = inFeatureReportBuffer[15] * 256 + inFeatureReportBuffer[14] - 32768;
            previnputReport.RawForceY = inFeatureReportBuffer[17] * 256 + inFeatureReportBuffer[16] - 32768;
            previnputReport.RawForceZ = inFeatureReportBuffer[19] * 256 + inFeatureReportBuffer[18] - 32768;
            previnputReport.RawGyroPitch = inFeatureReportBuffer[27] * 256 + inFeatureReportBuffer[26] - 32768;
            previnputReport.RawGyroRoll = inFeatureReportBuffer[29] * 256 + inFeatureReportBuffer[28] - 32768;
            previnputReport.RawGyroYaw = 32768 - (inFeatureReportBuffer[31] * 256 + inFeatureReportBuffer[30]);
        }

        public Byte[] setOutputReport()
        {
            Byte[] report = new Byte[52];
            report[3] = 2;
            report[5] = Convert.ToByte(outputReport.r);
            report[6] = Convert.ToByte(outputReport.g);
            report[7] = Convert.ToByte(outputReport.b);
            report[8] = 0;
            report[9] = Convert.ToByte(outputReport.rumble);
            return report;
        }

        public void setColor(int r, int g, int b)
        {
            this.outputReport.r = r;
            this.outputReport.g = g;
            this.outputReport.b = b;
        }

        public void setRumble(int value)
        {
            this.outputReport.rumble = value;
        }
    }
}
