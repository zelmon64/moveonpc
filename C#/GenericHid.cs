/// <summary>
///  Runs the application.
/// </summary> 
///  
using System;
using System.Collections;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace GenericHid
{   
    public class GenericHid  
    {   
        /// <summary>
        ///  Displays the application's main form.
        /// </summary> 
        
        public static void Main() 
        {
            TestFrm Test = new TestFrm();
            Application.Run( Test ); 
        }
    } 
} 
