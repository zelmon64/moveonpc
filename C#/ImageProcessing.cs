using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using Emgu.Util;
using System.Runtime.InteropServices;

namespace GenericHid
{
    class ImageProcessing
    {
        //private Capture cap;
        private PSEyeInterface myEye;
        Image<Bgra, Byte> tempimg = new Image<Bgra,byte>(640, 480);
        Image<Bgr, Byte> img = new Image<Bgr,byte>(640, 480);
        Image<Hsv, Byte> hsvImage;
        Image<Gray, Byte> gray;
 
        public ImageProcessing()
        {
            myEye = new PSEyeInterface(CLEyeCameraColorMode.CLEYE_COLOR_RAW,
                CLEyeCameraResolution.CLEYE_VGA,
                CLEyeFrameRate.CLEYE_FRAMERATE_30);
            myEye.ImageArrivedEvent += new PSEyeInterface.ImageArrivedEventHandler(myEye_ImageArrivedEvent);           
        }

        void myEye_ImageArrivedEvent(object sender, ImageArrivedEventArgs e)
        {
            try
            {
                tempimg = new Image<Bgra, byte>(e.Width, e.Height, e.Stride, e.DataPointer);
                img[0] = tempimg[0];
                img[1] = tempimg[1];
                img[2] = tempimg[2];
            }
            catch (CvException exc)
            {
                Console.Write(exc.Message);
            }
        }

        public Bitmap returnThresholdedImage()
        {
            gray = img[0].Resize(480, 320, INTER.CV_INTER_CUBIC).ThresholdBinary(new Gray(240), new Gray(255));
            DenseHistogram hist = new DenseHistogram(150, new RangeF(0, 255));
            CvInvoke.cvCalcHist(new System.IntPtr[] { gray.Ptr }, hist, false, IntPtr.Zero);
            return gray.Bitmap;
        }

        public int[] getThresholdedHistogram()
        {
            
            hsvImage = new Image<Hsv, byte>(new Size(480, 320));
            hsvImage.ConvertFrom<Bgr, Byte>(img);
            gray = img[0].Resize(480, 320, INTER.CV_INTER_CUBIC).ThresholdBinary(new Gray(240), new Gray(255));
            DenseHistogram hist = new DenseHistogram(150, new RangeF(0, 255));
            CvInvoke.cvCalcHist(new System.IntPtr[] { gray.Ptr }, hist, false, IntPtr.Zero);
            int[] values = new int[] { (int)hist[0], (int)hist[1] };
            return values;
        }

        public Bitmap returnImage()
        {
            return img.Resize(480, 320, INTER.CV_INTER_CUBIC).ToBitmap();            
        }

        public void setCameraParameters(int autogain, int autoexposure, int autowhitebalance,
            int gain, int exposure, int wbred, int wbgreen, int wbblue)
        {
            myEye.camParams.AutoExposure = autoexposure;
            myEye.camParams.AutoGain = autogain;
            myEye.camParams.AutoWhiteBalance = autowhitebalance;
            myEye.camParams.Exposure = exposure;
            myEye.camParams.Gain = gain;
            myEye.camParams.WhiteBalanceBlue = wbblue;
            myEye.camParams.WhiteBalanceGreen = wbgreen;
            myEye.camParams.WhiteBalanceRed = wbred;

            myEye.setCamParams();
        }
    }
}
