using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;
using Microsoft.VisualBasic;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;

namespace GenericHid
{
    public partial class TestFrm : Form
    {
        private IntPtr deviceNotificationHandle;
        private Boolean exclusiveAccess;
        private SafeFileHandle hidHandle;
        private String hidUsage;
        private Boolean myDeviceDetected;
        private String myDevicePathName;
        private SafeFileHandle readHandle;
        private SafeFileHandle writeHandle;

        private Debugging MyDebugging = new Debugging(); //  For viewing results of API calls via Debug.Write.
        private DeviceManagement MyDeviceManagement = new DeviceManagement();
        private Hid MyHid = new Hid();

        private PSMove myMove = new PSMove();
        private ImageProcessing imgProc = new ImageProcessing();
        

        public TestFrm()
        {
            InitializeComponent();            
        }
        
        
        ///  <summary>
        ///  Define a class of delegates that point to the Hid.ReportIn.Read function.
        ///  The delegate has the same parameters as Hid.ReportIn.Read.
        ///  Used for asynchronous reads from the device.       
        ///  </summary>
        
        private delegate void ReadInputReportDelegate( SafeFileHandle hidHandle, SafeFileHandle readHandle, SafeFileHandle writeHandle, ref Boolean myDeviceDetected, ref Byte[] readBuffer, ref Boolean success );
        
        //  This delegate has the same parameters as AccessForm.
        //  Used in accessing the application's form from a different thread.
        
        private delegate void MarshalToForm( String action, String textToAdd );
        
        ///  <summary>
        ///  Called when a WM_DEVICECHANGE message has arrived,
        ///  indicating that a device has been attached or removed.
        ///  </summary>
        ///  
        ///  <param name="m"> a message with information about the device </param>
        
        internal void OnDeviceChange( Message m ) 
        {             
            Debug.WriteLine( "WM_DEVICECHANGE" ); 
            
            try 
            {
                if ((m.WParam.ToInt32() == DeviceManagement.DBT_DEVICEARRIVAL)) 
                { 
                    
                    //  If WParam contains DBT_DEVICEARRIVAL, a device has been attached.
                    
                    Debug.WriteLine( "A device has been attached." ); 
                    
                    //  Find out if it's the device we're communicating with.
                    
                                       
                }
                else if ((m.WParam.ToInt32() == DeviceManagement.DBT_DEVICEREMOVECOMPLETE)) 
                { 
                    
                    //  If WParam contains DBT_DEVICEREMOVAL, a device has been removed.
                    
                    Debug.WriteLine( "A device has been removed." ); 
                    
                    //  Find out if it's the device we're communicating with.
                    
                    if ( MyDeviceManagement.DeviceNameMatch( m, myDevicePathName ) ) 
                    {   
                        //  Set MyDeviceDetected False so on the next data-transfer attempt,
                        //  FindTheHid() will be called to look for the device 
                        //  and get a new handle.
                        
                        this.myDeviceDetected = false; 
                    } 
                }                 
                //ScrollToBottomOfListBox();                 
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            } 
        }         
        
        ///  <summary>
        ///  Uses a series of API calls to locate a HID-class device
        ///  by its Vendor ID and Product ID.
        ///  </summary>
        ///          
        ///  <returns>
        ///   True if the device is detected, False if not detected.
        ///  </returns>
        
        private Boolean FindTheHid() 
        {             
            Boolean deviceFound = false; 
            String[] devicePathName = new String[ 128 ];
            String functionName = "";
            Guid hidGuid = Guid.Empty; 
            Int32 memberIndex = 0; 
            Int16 myProductID = 0; 
            Int16 myVendorID = 0; 
            Boolean success = false;            
            
            try 
            { 
                myDeviceDetected = false;
                myVendorID = Convert.ToInt16(Conversion.Val("&h" + "8888"));
                myProductID = Convert.ToInt16(Conversion.Val("&h" + "0308"));       
                //  ***
                //  API function: 'HidD_GetHidGuid
                
                //  Purpose: Retrieves the interface class GUID for the HID class.
                
                //  Accepts: 'A System.Guid object for storing the GUID.
                //  ***
                
                Hid.HidD_GetHidGuid( ref hidGuid );

                functionName = "GetHidGuid"; 
                Debug.WriteLine( MyDebugging.ResultOfAPICall( functionName ) ); 
                Debug.WriteLine( "  GUID for system HIDs: " + hidGuid.ToString() ); 
                
                //  Fill an array with the device path names of all attached HIDs.
                
                deviceFound = MyDeviceManagement.FindDeviceFromGuid( hidGuid, ref devicePathName ); 
                
                //  If there is at least one HID, attempt to read the Vendor ID and Product ID
                //  of each device until there is a match or all devices have been examined.
                
                if ( deviceFound ) 
                {                     
                    memberIndex = 0; 
                    
                    do 
                    { 
                        //  ***
                        //  API function:
                        //  CreateFile
                        
                        //  Purpose:
                        //  Retrieves a handle to a device.
                        
                        //  Accepts:
                        //  A device path name returned by SetupDiGetDeviceInterfaceDetail
                        //  The type of access requested (read/write).
                        //  FILE_SHARE attributes to allow other processes to access the device while this handle is open.
                        //  A Security structure or IntPtr.Zero. 
                        //  A creation disposition value. Use OPEN_EXISTING for devices.
                        //  Flags and attributes for files. Not used for devices.
                        //  Handle to a template file. Not used.
                        
                        //  Returns: a handle without read or write access.
                        //  This enables obtaining information about all HIDs, even system
                        //  keyboards and mice. 
                        //  Separate handles are used for reading and writing.
                        //  ***

                        hidHandle = FileIO.CreateFile(devicePathName[memberIndex], 0, FileIO.FILE_SHARE_READ | FileIO.FILE_SHARE_WRITE, IntPtr.Zero, FileIO.OPEN_EXISTING, 0, 0);

                        functionName = "CreateFile"; 
                        Debug.WriteLine( MyDebugging.ResultOfAPICall( functionName ) ); 
                        Debug.WriteLine( "  Returned handle: " + hidHandle.ToString() ); 
                        
                        if (!hidHandle.IsInvalid)  
                        {                             
                            //  The returned handle is valid, 
                            //  so find out if this is the device we're looking for.
                            
                            //  Set the Size property of DeviceAttributes to the number of bytes in the structure.
                            
                            MyHid.DeviceAttributes.Size = Marshal.SizeOf( MyHid.DeviceAttributes ); 
                            
                            //  ***
                            //  API function:
                            //  HidD_GetAttributes
                            
                            //  Purpose:
                            //  Retrieves a HIDD_ATTRIBUTES structure containing the Vendor ID, 
                            //  Product ID, and Product Version Number for a device.
                            
                            //  Accepts:
                            //  A handle returned by CreateFile.
                            //  A pointer to receive a HIDD_ATTRIBUTES structure.
                            
                            //  Returns:
                            //  True on success, False on failure.
                            //  ***                            
                            
                            success = Hid.HidD_GetAttributes(hidHandle, ref MyHid.DeviceAttributes); 
                            
                            if ( success ) 
                            {                                
                                Debug.WriteLine( "  HIDD_ATTRIBUTES structure filled without error." ); 
                                Debug.WriteLine( "  Structure size: " + MyHid.DeviceAttributes.Size );                                                                                               
                                Debug.WriteLine("  Vendor ID: " + Convert.ToString(MyHid.DeviceAttributes.VendorID, 16));                               
                                Debug.WriteLine("  Product ID: " + Convert.ToString(MyHid.DeviceAttributes.ProductID, 16));                               
                                Debug.WriteLine("  Version Number: " + Convert.ToString(MyHid.DeviceAttributes.VersionNumber, 16)); 
                                
                                //  Find out if the device matches the one we're looking for.
                                
                                if ( ( MyHid.DeviceAttributes.VendorID == myVendorID ) && ( MyHid.DeviceAttributes.ProductID == myProductID ) ) 
                                { 
                                    
                                    Debug.WriteLine( "  My device detected" ); 
                                    
                                    //  Display the information in form's list box.
                                    
                                                                     
                                                                        
                                    myDeviceDetected = true; 
                                    
                                    //  Save the DevicePathName for OnDeviceChange().
                                    
                                    myDevicePathName = devicePathName[ memberIndex ]; 
                                } 
                                else 
                                {                                     
                                    //  It's not a match, so close the handle.
                                    
                                    myDeviceDetected = false;                                     
                                    hidHandle.Close();                                     
                                }                                 
                            } 
                            else 
                            { 
                                //  There was a problem in retrieving the information.
                                
                                Debug.WriteLine( "  Error in filling HIDD_ATTRIBUTES structure." ); 
                                myDeviceDetected = false; 
                                hidHandle.Close(); 
                            }                             
                        } 
                        
                        //  Keep looking until we find the device or there are no devices left to examine.
                        
                        memberIndex = memberIndex + 1;                         
                    } 
                    while (  ! ( ( myDeviceDetected || ( memberIndex == devicePathName.Length ) ) ) );                     
                } 
                
                if ( myDeviceDetected ) 
                {                     
                    //  The device was detected.
                    //  Register to receive notifications if the device is removed or attached.
                    
                    success = MyDeviceManagement.RegisterForDeviceNotifications( myDevicePathName, this.Handle, hidGuid, ref deviceNotificationHandle ); 
                    
                    Debug.WriteLine( "RegisterForDeviceNotifications = " + success ); 
                    
                    //  Learn the capabilities of the device.
                    
                    MyHid.Capabilities = MyHid.GetDeviceCapabilities( hidHandle ); 
                    
                    if ( success ) 
                    {                         
                        //  Find out if the device is a system mouse or keyboard.
                        
                        hidUsage = MyHid.GetHidUsage( MyHid.Capabilities ); 
                        
                        //  Get the Input report buffer size.
                                               
                        //  Get handles to use in requesting Input and Output reports.

                        readHandle = FileIO.CreateFile(myDevicePathName, FileIO.GENERIC_READ, FileIO.FILE_SHARE_READ | FileIO.FILE_SHARE_WRITE, IntPtr.Zero, FileIO.OPEN_EXISTING, FileIO.FILE_FLAG_OVERLAPPED, 0);

                        functionName = "CreateFile, ReadHandle";
                        Debug.WriteLine(MyDebugging.ResultOfAPICall(functionName)); 
                        Debug.WriteLine( "  Returned handle: " + readHandle.ToString() ); 
                        
                        if ( readHandle.IsInvalid ) 
                        {                             
                            exclusiveAccess = true;                                                    
                        } 
                        else 
                        {
                            writeHandle = FileIO.CreateFile(myDevicePathName, FileIO.GENERIC_WRITE, FileIO.FILE_SHARE_READ | FileIO.FILE_SHARE_WRITE, IntPtr.Zero, FileIO.OPEN_EXISTING, 0, 0);

                            functionName = "CreateFile, WriteHandle";
                            Debug.WriteLine(MyDebugging.ResultOfAPICall(functionName)); 
                            Debug.WriteLine( "  Returned handle: " + writeHandle.ToString() ); 
                            
                            //  Flush any waiting reports in the input buffer. (optional)
                            
                            MyHid.FlushQueue( readHandle );                             
                        } 
                    } 
                } 
                else 
                { 
                    //  The device wasn't detected.
                    
                    Debug.WriteLine( " Device not found." );                     
                }                 
                return myDeviceDetected;                 
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            } 
        }         
       
        
        ///  <summary>
        ///  Sends a Feature report, then retrieves one.
        ///  Assumes report ID = 0 for both reports.
        ///  </summary>
        
        private void ExchangeFeatureReports() 
        {             
            String byteValue = null; 
            Int32 count = 0; 
            Byte[] inFeatureReportBuffer = null; 
            Byte[] outFeatureReportBuffer = null; 
            Boolean success = false; 
            
            try 
            { 
                Hid.InFeatureReport myInFeatureReport = new Hid.InFeatureReport(); 
                Hid.OutFeatureReport myOutFeatureReport = new Hid.OutFeatureReport(); 
                
                inFeatureReportBuffer = null; 
                
                if ( ( MyHid.Capabilities.FeatureReportByteLength > 0 ) ) 
                {                     
                    //  The HID has a Feature report.
                    
                    outFeatureReportBuffer = new Byte[ MyHid.Capabilities.FeatureReportByteLength ];
                                        
                    outFeatureReportBuffer = myMove.setOutputReport();                   
                    
                    
                    //  Write a report to the device
                    
                    success = myOutFeatureReport.Write( outFeatureReportBuffer, hidHandle );                    
                    
                    
                    //  Read a report from the device.
                    
                    //  Set the size of the Feature report buffer. 
                    //  Subtract 1 from the value in the Capabilities structure because 
                    //  the array begins at index 0.
                    
                    if ( ( MyHid.Capabilities.FeatureReportByteLength > 0 ) ) 
                    { 
                        inFeatureReportBuffer = new Byte[ MyHid.Capabilities.FeatureReportByteLength ]; 
                    } 
                    
                    //  Read a report.
                    
                    myInFeatureReport.Read( hidHandle, readHandle, writeHandle, ref myDeviceDetected, ref inFeatureReportBuffer, ref success ); 
                    
                    if ( success ) 
                    {
                        //fill input variables
                        myMove.setInputReports(inFeatureReportBuffer);
                        
                        setLabels();
                        setCheckBoxes();                       
                    } 
                    else 
                    { 
                        //lstResults.Items.Add( "The attempt to read a Feature report failed." ); 
                    }                     
                } 
                else 
                { 
                    //lstResults.Items.Add( "The HID doesn't have a Feature report." ); 
                }
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            }             
        }    
        
         ///  <summary>
        ///  Initiates exchanging reports. 
        ///  The application sends a report and requests to read a report.
        ///  </summary>
        
        private void ReadAndWriteToDevice() 
        {             
            // Report header for the debug display:
            
            Debug.WriteLine( "" ); 
            Debug.WriteLine( "***** HID Test Report *****" ); 
            Debug.WriteLine( DateAndTime.Today + ": " + DateAndTime.TimeOfDay ); 
            
            try 
            { 
                //  If the device hasn't been detected, was removed, or timed out on a previous attempt
                //  to access it, look for the device.
                
                if ( ( myDeviceDetected == false ) ) 
                {                     
                    myDeviceDetected = FindTheHid();                     
                } 
                
                if ( ( myDeviceDetected == true ) ) 
                {                     
                   ExchangeFeatureReports();                                          
                }                 
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            }             
        } 
      
        
        ///  <summary>
        ///  Perform actions that must execute when the program ends.
        ///  </summary>
        
        private void Shutdown() 
        {             
            try 
            { 
                //  Close open handles to the device.
                
                if ( !( hidHandle == null ) ) 
                { 
                    if ( !( hidHandle.IsInvalid ) ) 
                    { 
                        hidHandle.Close(); 
                    } 
                } 
                
                if ( !( readHandle == null ) ) 
                { 
                    if ( !( readHandle.IsInvalid ) ) 
                    { 
                        readHandle.Close(); 
                    } 
                } 
                
                if ( !( writeHandle == null ) ) 
                { 
                    if ( !( writeHandle.IsInvalid ) ) 
                    { 
                        writeHandle.Close(); 
                    } 
                } 
                
                //  Stop receiving notifications.
                
                MyDeviceManagement.StopReceivingDeviceNotifications( deviceNotificationHandle );                 
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            }             
        } 
                
        ///  <summary>
        ///  Perform actions that must execute when the program starts.
        ///  </summary>
        
        private void Startup() 
        {            
            try 
            { 
                MyHid = new Hid();       
                          
                tmrContinuousDataCollect.Enabled = true; 
                tmrContinuousDataCollect.Interval = 20;                 
                
                FindTheHid();
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            }             
        }                 
   
        ///  <summary>
        ///   Overrides WndProc to enable checking for and handling WM_DEVICECHANGE messages.
        ///  </summary>
        ///  
        ///  <param name="m"> a Windows Message </param>        
        protected override void WndProc( ref Message m ) 
        {            
            try 
            { 
                //  The OnDeviceChange routine processes WM_DEVICECHANGE messages.
                
                if ( m.Msg == DeviceManagement.WM_DEVICECHANGE ) 
                { 
                    OnDeviceChange( m ); 
                } 
                
                //  Let the base form process the message.
                
                base.WndProc( ref m );                 
            } 
            catch ( Exception ex ) 
            { 
                DisplayException( this.Name, ex ); 
                throw ; 
            }             
        }         
        
        ///  <summary>
        ///  Provides a central mechanism for exception handling.
        ///  Displays a message box that describes the exception.
        ///  </summary>
        ///  
        ///  <param name="moduleName"> the module where the exception occurred. </param>
        ///  <param name="e"> the exception </param>
        
        internal static void DisplayException( String moduleName, Exception e ) 
        {             
            String message = null; 
            String caption = null; 
            
            //  Create an error message.
            
            message = "Exception: " + e.Message + ControlChars.CrLf + "Module: " + moduleName + ControlChars.CrLf + "Method: " + e.TargetSite.Name; 
            
            caption = "Unexpected Exception"; 
            
            MessageBox.Show( message, caption, MessageBoxButtons.OK ); 
            Debug.Write( message );             
        }

        private void TestFrm_FormClosed(object sender, FormClosedEventArgs e)
        {
            try
            {
                Shutdown();
            }
            catch (Exception ex)
            {
                DisplayException(this.Name, ex);
                throw;
            } 
        }

        private void TestFrm_Load(object sender, EventArgs e)
        {
            try
            {
                Startup();
            }
            catch (Exception ex)
            {
                DisplayException(this.Name, ex);
                throw;
            }    
        }

        private void tmrContinuousDataCollect_Tick_1(object sender, EventArgs e)
        {
            //Dancing Move Code
            //int[] test = new int[] {0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 255};
            //Random r = new Random();
            
            //myMove.outputReport.r = test[r.Next(11) % 11];
            //myMove.outputReport.b = test[r.Next(11) % 11];
            //myMove.outputReport.g = test[r.Next(11) % 11];
            //myMove.outputReport.rumble = 255;

            if(myMove.psButtons.MOVE_CIRCLE_BUTTON)
                myMove.setColor(255, 0, 255);
            if (myMove.psButtons.MOVE_MOVE_BUTTON)
                myMove.setColor(255, 0, 0);

            ExchangeFeatureReports();

            pictureBox1.Image = imgProc.returnThresholdedImage();
            pictureBox2.Image = imgProc.returnImage();

            //do a rough distance detection based on histogram

            double histPercentInImg = (320f * 480f - imgProc.getThresholdedHistogram()[0]) / (320f * 480f);
            label3.Text = histPercentInImg.ToString();

            //assumption that 10% of image is ball it is near
            //assumption that 0.04% of image is ball it is far
            double progBarValue = ((0.04 / histPercentInImg) - 0.4);

            //binning
            if (progBarValue > 100) progBarValue = 100;
            if (progBarValue < 0) progBarValue = 0;

            progressBar1.Value = (int)progBarValue;
        }

        private void setLabels()
        {
            buttonLabel.Text =      "Buttons:   " + myMove.inputReport.Buttons.ToString();            
            analogLabel.Text =      "Analog:    " + myMove.inputReport.TAnalog.ToString();
            accXLabel.Text =        "Acc X:     " + myMove.inputReport.RawForceX.ToString();
            accYLabel.Text =        "Acc Y:     " + myMove.inputReport.RawForceY.ToString();
            accZLabel.Text =        "Acc Z:     " + myMove.inputReport.RawForceZ.ToString();
            gyrPitchLabel.Text =    "Pitch:     " + myMove.inputReport.RawGyroPitch.ToString();
            gyrRollLabel.Text =     "Roll:      " + myMove.inputReport.RawGyroRoll.ToString();
            gyrYawLabel.Text =      "Yaw:       " + myMove.inputReport.RawGyroYaw.ToString();
            magXLabel.Text =        "Mag X:     " + myMove.inputReport.RawMagnetX.ToString();
            magYLabel.Text =        "Mag Y:     " + myMove.inputReport.RawMagnetY.ToString();
            magZLabel.Text =        "Mag Z:     " + myMove.inputReport.RawMagnetZ.ToString();
            tempLabel.Text =        "Temp:      " + myMove.inputReport.RawTemperature.ToString();
            batLabel.Text = "BatLevel:  " + myMove.inputReport.RawBattery.ToString();

        }

        private void setCheckBoxes()
        {
            if (myMove.psButtons.MOVE_SQUARE_BUTTON) squareCheck.Checked = true;
            else squareCheck.Checked = false;
            if (myMove.psButtons.MOVE_CROSS_BUTTON) crossCheck.Checked = true;
            else crossCheck.Checked = false;
            if (myMove.psButtons.MOVE_TRIANGLE_BUTTON) triangleCheck.Checked = true;
            else triangleCheck.Checked = false;
            if (myMove.psButtons.MOVE_CIRCLE_BUTTON) circleCheck.Checked = true;
            else circleCheck.Checked = false;
            if (myMove.psButtons.MOVE_MOVE_BUTTON) moveCheck.Checked = true;
            else moveCheck.Checked = false;
            if (myMove.psButtons.MOVE_T_BUTTON > 0) tCheck.Checked = true;
            else tCheck.Checked = false;
            if (myMove.psButtons.MOVE_START_BUTTON) startCheck.Checked = true;
            else startCheck.Checked = false;
            if (myMove.psButtons.MOVE_SELECT_BUTTON) selectCheck.Checked = true;
            else selectCheck.Checked = false;
        }

        private void cameraParameter_Changed(object sender, EventArgs e)
        {
            imgProc.setCameraParameters(0, 0, 0, gainBar.Value, exposureBar.Value, wbredBar.Value,
                wbgreenBar.Value, wbblueBar.Value);
        }
    }
}
