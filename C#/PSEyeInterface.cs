using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using System.Threading;

namespace GenericHid
{
     #region [ Camera Parameters ]
    // camera color mode
    public enum CLEyeCameraColorMode
    {
        CLEYE_MONO_PROCESSED,
        CLEYE_COLOR_PROCESSED,
        CLEYE_MONO_RAW,
        CLEYE_COLOR_RAW,
        CLEYE_BAYER_RAW
    };

    // camera resolution
    public enum CLEyeCameraResolution
    {
        CLEYE_QVGA,
        CLEYE_VGA
    };
    // camera parameters
    public enum CLEyeCameraParameter
    {
        // camera sensor parameters
        CLEYE_AUTO_GAIN,			// [false, true]
        CLEYE_GAIN,					// [0, 79]
        CLEYE_AUTO_EXPOSURE,		// [false, true]
        CLEYE_EXPOSURE,				// [0, 511]
        CLEYE_AUTO_WHITEBALANCE,	// [false, true]
        CLEYE_WHITEBALANCE_RED,		// [0, 255]
        CLEYE_WHITEBALANCE_GREEN,	// [0, 255]
        CLEYE_WHITEBALANCE_BLUE,	// [0, 255]
        // camera linear transform parameters
        CLEYE_HFLIP,				// [false, true]
        CLEYE_VFLIP,				// [false, true]
        CLEYE_HKEYSTONE,			// [-500, 500]
        CLEYE_VKEYSTONE,			// [-500, 500]
        CLEYE_XOFFSET,				// [-500, 500]
        CLEYE_YOFFSET,				// [-500, 500]
        CLEYE_ROTATION,				// [-500, 500]
        CLEYE_ZOOM,					// [-500, 500]
        // camera non-linear transform parameters
        CLEYE_LENSCORRECTION1,		// [-500, 500]
        CLEYE_LENSCORRECTION2,		// [-500, 500]
        CLEYE_LENSCORRECTION3,		// [-500, 500]
        CLEYE_LENSBRIGHTNESS		// [-500, 500]
    };

    public enum CLEyeFrameRate
    { 
        CLEYE_FRAMERATE_15 = 15,
        CLEYE_FRAMERATE_30 = 30,
        CLEYE_FRAMERATE_40 = 40,
        CLEYE_FRAMERATE_50 = 50,
        CLEYE_FRAMERATE_60 = 60,
        CLEYE_FRAMERATE_75 = 75,
        CLEYE_FRAMERATE_100 = 100,
        CLEYE_FRAMERATE_125 = 125
    };
    #endregion

    public struct CameraParameters
    {
        public int AutoGain;
        public int AutoExposure;
        public int AutoWhiteBalance;
        public int Gain;
        public int Exposure;
        public int WhiteBalanceRed;
        public int WhiteBalanceGreen;
        public int WhiteBalanceBlue;
    }

    public class ImageArrivedEventArgs : EventArgs
    {
        public int Width;
        public int Height;
        public int Stride;
        public PixelFormat PixelFormat;
        public IntPtr DataPointer;
        
        public ImageArrivedEventArgs(int width, int height, int stride, PixelFormat pixForm, IntPtr ptr)
        {
            this.Width = width;
            this.Height = height;
            this.Stride = stride;
            this.PixelFormat = pixForm;
            this.DataPointer = ptr;
        }
    }

    public class PSEyeInterface
    {
        #region [ CLEyeMulticam Imports ]
        [DllImport("CLEyeMulticam.dll")]
        public static extern int CLEyeGetCameraCount();
        [DllImport("CLEyeMulticam.dll")]
        public static extern Guid CLEyeGetCameraUUID(int camId);
        [DllImport("CLEyeMulticam.dll")]
        public static extern IntPtr CLEyeCreateCamera(Guid camUUID, CLEyeCameraColorMode mode, CLEyeCameraResolution res, float frameRate);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeDestroyCamera(IntPtr camera);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeCameraStart(IntPtr camera);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeCameraStop(IntPtr camera);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeCameraLED(IntPtr camera, bool on);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeSetCameraParameter(IntPtr camera, CLEyeCameraParameter param, int value);
        [DllImport("CLEyeMulticam.dll")]
        public static extern int CLEyeGetCameraParameter(IntPtr camera, CLEyeCameraParameter param);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeCameraGetFrameDimensions(IntPtr camera, ref int width, ref int height);
        [DllImport("CLEyeMulticam.dll")]
        public static extern bool CLEyeCameraGetFrame(IntPtr camera, IntPtr pData, int waitTimeout);
        #endregion

        public delegate void ImageArrivedEventHandler(object sender, ImageArrivedEventArgs e);
        public event ImageArrivedEventHandler ImageArrivedEvent;
        public CameraParameters camParams;

        protected virtual void OnImageArrived(ImageArrivedEventArgs e)
        {
            if (ImageArrivedEvent != null)
                ImageArrivedEvent(this, e);
        }
                
        bool threadRunning;
        ManualResetEvent exitEvent;
        
        public IntPtr ptrBmpPixels;        
        IntPtr camera;
        int width = 0;
        int height = 0;
        int stride = 0;
        PixelFormat pixForm;

        public static int CameraCount { get { return CLEyeGetCameraCount(); } }
        public static Guid CameraUUID(int idx) { return CLEyeGetCameraUUID(idx); }

        public PSEyeInterface(CLEyeCameraColorMode cameraMode, CLEyeCameraResolution cameraResolution, CLEyeFrameRate framerate)
        {   
            //initialize camera and get image dimensions
            camera = CLEyeCreateCamera(CameraUUID(0), cameraMode, cameraResolution,(float)framerate);
            CLEyeCameraGetFrameDimensions(camera, ref width, ref height);

            camParams.AutoExposure = 0;
            camParams.AutoGain = 0;
            camParams.AutoWhiteBalance = 0;
            camParams.Exposure = 100;
            camParams.Gain = 30;
            camParams.WhiteBalanceBlue = 50;
            camParams.WhiteBalanceGreen = 50;
            camParams.WhiteBalanceRed = 50;

            setCamParams();

            //set format and allocate memory according to image format
            if (cameraMode == CLEyeCameraColorMode.CLEYE_MONO_PROCESSED || cameraMode == CLEyeCameraColorMode.CLEYE_MONO_RAW)
            {
                pixForm = PixelFormat.Format8bppIndexed;
                stride = width;
                ptrBmpPixels = Marshal.AllocHGlobal(width * height);
            }
            else
            {
                pixForm = PixelFormat.Format32bppRgb;
                stride = width * 4;
                ptrBmpPixels = Marshal.AllocHGlobal(width * height * 4);
            }
            
            //start capturing thread
            exitEvent = new ManualResetEvent(false);
            ThreadPool.QueueUserWorkItem(Capture);

            threadRunning = false;
            exitEvent.WaitOne(3000);
        }

        // capture thread
        void Capture(object obj)
        {
            threadRunning = true;
            Random rng = new Random();
            CLEyeCameraStart(camera);
            while (threadRunning)
            {
                if (CLEyeCameraGetFrame(camera, ptrBmpPixels, 500))
                    OnImageArrived(new ImageArrivedEventArgs(width, height, stride, pixForm, ptrBmpPixels));
            }
            CLEyeCameraStop(camera);
            CLEyeDestroyCamera(camera);
            exitEvent.Set();
        }

        public void setCamParams()
        {
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_AUTO_EXPOSURE, camParams.AutoExposure);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_AUTO_GAIN, camParams.AutoGain);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_AUTO_WHITEBALANCE, camParams.AutoWhiteBalance);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_EXPOSURE, camParams.Exposure);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_GAIN, camParams.Gain);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_WHITEBALANCE_BLUE, camParams.WhiteBalanceBlue);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_WHITEBALANCE_GREEN, camParams.WhiteBalanceGreen);
            CLEyeSetCameraParameter(camera, CLEyeCameraParameter.CLEYE_WHITEBALANCE_RED, camParams.WhiteBalanceRed);
        }
    }
}
