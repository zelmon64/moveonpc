/* Playstation Move on the PC.
   This file reads all the values from the Move, and changes its colour, and rumbles.
   There should be no platform specific code here, and no knowledge of the transport layer
   by Carl Kenner 4 October 2010 - 16 November 2010
   This file (and anything derived from it, based on it, or using it) is released under a 
   non-military license, it may not be used for any military purpose, including military 
   research, or military funded research, training, recruitment, troop entertainment, 
   design of military systems, or anything else involving the military. I didn't write this
   to help kill people, sorry.
*/

#include "MovePrecompiled.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "moveonpc.h"

#define FALSE 0
#define TRUE 1

TMoveOutput MoveOutput[MAXMOVES] = {0};
TSixaxisOutput SixaxisOutput[MAXMOVES] = {0};

// sends output to the Move, must be done every second or so, 
// or the light and rumble switch themselves off
static int SendMoveOutput(int index) {
	unsigned char report[49];
	memset(report,0,49);
	report[0] = 2; // must be 2 or 6 to control colour
  report[1] = 0; // unknown
	report[2] = MoveOutput[index].r;
	report[3] = MoveOutput[index].g;
	report[4] = MoveOutput[index].b;
	report[5] = 0; // must be an even number to allow rumble
	report[6] = MoveOutput[index].rumble;
	MoveOutput[index].framecount=0; // don't need to resend for a while
	return WriteMoveReport(index, report);
}

// sends output to the Sixaxis, output will last until modified, unlike Move
static int SendSixaxisOutput(int index) {
  SixaxisOutput[index].report[0] = 1;
	return WriteSixaxisReport(index, SixaxisOutput[index].report);
}

// Sets the colour of the move's glowing ball, each component is 0x00 to 0xFF, 
// but the green led is much darker than the other leds, so FF FF FF looks purple.
// Use something like FF AF FF for white.
int SetMoveColour(int index, int r, int g, int b) {
	if (index>=MAXMOVES || index<0) return FALSE;
	if (MoveOutput[index].framecount<100 && MoveOutput[index].r==r && MoveOutput[index].g==g && MoveOutput[index].b==b) return TRUE;
	MoveOutput[index].r = r;
	MoveOutput[index].g = g;
	MoveOutput[index].b = b;
	return SendMoveOutput(index);
}

// Sets the Move's amount of vibration. Rumble can be 0 to 255.
// Some low values of rumble will not turn rumble on if it is off,
// but also won't turn rumble off if it is on. 0 will always turn it off.
int SetMoveRumble(int index, int rumble) {
	if (index>=MAXMOVES || index<0) return FALSE;
	if (MoveOutput[index].framecount < 100 && MoveOutput[index].rumble == rumble) return TRUE;
	MoveOutput[index].rumble = rumble;
	return SendMoveOutput(index);
}

// Dual Shock 3 controllers are classified as a SIXAXIS, but they can rumble.
// I haven't tested this yet, since I only have a SIXAXIS.
int SetDualShock3Rumble(int index, int rumble) {
	if (index>=MAXMOVES || index<0) return FALSE;
  unsigned char *report = SixaxisOutput[index].report;
  report[0] = 1; // report number
  report[2] = 0; // small duration
  report[3] = 0; // small voltage
  report[4] = 0xFF; // big duration, gyro on SIXAXIS, rumble on DS3
  report[5] = rumble; // big voltage, gyro on SIXAXIS, rumble on DS3
  return SendSixaxisOutput(index);
}

int SetSixaxisLeds(int index, int leds) {
	if (index>=MAXMOVES || index<0) return FALSE;
  unsigned char *report = SixaxisOutput[index].report;
  report[0] = 1; // report number
//  report[1] = 0; // feature select
//  report[6] = 0; // no clue (6,7,8,9)
  report[10] = leds << 1; // LEDS, bit0 = unknown, bit1-bit4 = LEDS
  for (int i=0; i<6; i++) {
    report[11+i*5] = 0xff; // led duration
    report[12+i*5] = 0; // flash frequency
    report[13+i*5] = 0x10; // block 1
    report[14+i*5] = 0x10; // block 2
    report[15+i*5] = 0x10; // block 3
  }
  return SendSixaxisOutput(index);
}

// decode 12 bit signed twos-complement
int TwelveBits(int n) {
  if (n>0x800) n-=0x1000;
  return n;
}

// I originally thought it was more complicated than this. Oops.
int NineBits(int n) {
  return n-511;
}


// old provides the data from half a frame before. That means we have double the frame rate.
// Note that the data in old was not previously reported.
// Unfortunately, I don't know what the frame rate is. I think it's about 85 Hz.
// NOTE! All the addresses are 1 less than they were before to remove MotionInJoy specific stuff!
int ReadMove(int index, PMove data, PMove old) {
	if (index>=MAXMOVES || index<0) {
		return FALSE;
	}
	MoveOutput[index].framecount++;
	if (MoveOutput[index].framecount >= 100) {
		SendMoveOutput(index);
	}
	if (!data) {
		return FALSE;
	}
	if (!ReadMoveReport(index, data->report)) {
		return FALSE;
	}

	data->Buttons    = data->report[2] | (data->report[1] << 8) 
						| ((data->report[3] & 1) << 16) | ((data->report[4] & 0xF0) << 13);
	data->TAnalog    = data->report[6]; // report 5 is the same, but slightly older
	data->RawBattery = data->report[12];
	data->Timestamp  = data->report[11]*256 + data->report[43];
	data->SeqNumber  = data->report[4] & 0x0F;

	// Uses left handed coordinates like GlovePIE or Direct3D: x is right, y is up, z is forwards
	// rotations are like GlovePIE: yaw is turn right, pitch is tilt up, roll is tilt right
	data->RawForceX = data->report[20]*256 + data->report[19] - 32768; // same as 6 bytes earlier
	data->RawForceZ = data->report[22]*256 + data->report[21] - 32768;
	data->RawForceY = data->report[24]*256 + data->report[23] - 32768;
	
	data->RawGyroPitch = (data->report[32]*256 + data->report[31]) - 32768; // same as 6 bytes earlier
	data->RawGyroRoll  = (data->report[34]*256 + data->report[33]) - 32768;
	data->RawGyroYaw   = 32768 - (data->report[36]*256 + data->report[35]);
	
	// -1733 = 29.0, -1685.5 = 29 degrees C // -1681.5 = 29.5
	data->RawTemperature = TwelveBits(data->report[37]*16+(data->report[38] >> 4)); 
	
	data->RawMagnetX = -TwelveBits((data->report[38] & 15)*256+data->report[39]);
	data->RawMagnetZ =  TwelveBits(data->report[40]*16 + (data->report[41] >> 4));
	data->RawMagnetY = -TwelveBits((data->report[41] & 15)*256+data->report[42]);
  
	if (old) {
		*old = *data;
		old->Buttons = data->report[2] | (data->report[1] << 8) | (data->report[3] << 16);
		old->TAnalog = data->report[5];
		old->RawForceX = old->report[14]*256 + old->report[13] - 32768; // same as 6 bytes earlier
		old->RawForceZ = old->report[16]*256 + old->report[15] - 32768;
		old->RawForceY = old->report[18]*256 + old->report[17] - 32768;
		old->RawGyroPitch = (old->report[26]*256 + old->report[25]) - 32768; // same as 6 bytes earlier
		old->RawGyroRoll  = (old->report[28]*256 + old->report[27]) - 32768;
		old->RawGyroYaw   = 32768-(old->report[30]*256 + old->report[29]);
	}

	return TRUE;
}

// Read the Navigation Controller. The navigation controller has no motion sensing, and no rumble,
// but it has an analog joystick and DPad, and analog L1 and L2 buttons.
// Unlike the Move, it uses the same interface as a SIXAXIS or Dual Shock 3.
int ReadNav(int index, PNav data) {
	if (index>=MAXMOVES || index<0) return FALSE;
	if (!data) return FALSE;
	if (!ReadNavReport(index, data->report)) return FALSE;
	data->Buttons=data->report[3] | (data->report[2] << 8) | (data->report[4] << 16);
  data->LeftStickX=data->report[6];
  data->LeftStickY=data->report[7];
  data->UpAnalog=data->report[14];
  data->RightAnalog=data->report[15];
  data->DownAnalog=data->report[16];
  data->LeftAnalog=data->report[17];
	data->L2Analog=data->report[18];
	data->L1Analog=data->report[20];
  // There is no motion sensor in the Navigation Controller, and none of the buttons are analog.
	return TRUE;
}

// Read the SIXAXIS or Dual Shock 3. Currently the Gyro will not work on a SIXAXIS, since it needs
// to be switched on, which has the side effect of switching on rumble on a Dual Shock 3.
int ReadSixaxis(int index, PSixaxis data) {
	if (index>=MAXMOVES || index<0) return FALSE;
	if (!data) return FALSE;
	if (!ReadSixaxisReport(index, data->report)) return FALSE;
	data->Buttons=data->report[3] | (data->report[2] << 8) | (data->report[4] << 16);
  data->LeftStickX=data->report[6];
  data->LeftStickY=data->report[7];
  data->RightStickX=data->report[8];
  data->RightStickY=data->report[9];
  data->UpAnalog=data->report[14];
  data->RightAnalog=data->report[15];
  data->DownAnalog=data->report[16];
  data->LeftAnalog=data->report[17];
	data->L2Analog=data->report[18];
	data->R2Analog=data->report[19];
	data->L1Analog=data->report[20];
	data->R1Analog=data->report[21];
	data->TriangleAnalog=data->report[22];
	data->CircleAnalog=data->report[23];
	data->CrossAnalog=data->report[24];
	data->SquareAnalog=data->report[25];
  data->RawForceX=NineBits(data->report[41]*256+data->report[42]);
  data->RawForceZ=NineBits(data->report[43]*256+data->report[44]);
  data->RawForceY=-NineBits(data->report[45]*256+data->report[46]);
  data->RawGyroYaw=data->report[47]*256+data->report[48];
  return TRUE;
}

// This is as far as I have got with the calibration. Note that this function will only work if it's
// connected via libusb-win32, or normal windows HID (not finished implementing yet), since MotionInJoy
// doesn't give us a way of reading this data.
int ReadMoveCalibration(int index, PMoveCalib calib) {
	if (index>=MAXMOVES || index<0 || !calib) return false;
  // the calibration has 3 parts, which you get by reading report 0x10 3 times
  unsigned char temp[49];
  unsigned char *report = calib->report;
  int parts=0;
  while (parts!=(1|2|4)) {
	  if (!ReadMoveFeature(index, 0x10, temp)) return false;
	switch (temp[1] & 0xF) {
	  case 0: memcpy(report, temp, 49); parts |= 1; break;
	  case 1: memcpy(&report[49], &temp[2], 47); parts |= 2; break;
	  case 2: memcpy(&report[49+47], &temp[2], 47); parts |= 4; break;
	}
  }
  // I don't know what these headers are. Maybe temperatures?
  calib->AccHeader = report[2]+report[3]*256;
  char map[6] = {1,3, 2,0, 5,4}; // convert to order minx,maxx,miny,maxy,minz,maxz
  // accelerometer readings for 6 different orientations
  for (int v=0; v<6; v++) {
	int i = map[v];
	calib->AccVectors[v][0] = report[4+i*6+0]+report[4+i*6+1]*256-32768;
	calib->AccVectors[v][1] = report[4+i*6+4]+report[4+i*6+5]*256-32768;
	calib->AccVectors[v][2] = report[4+i*6+2]+report[4+i*6+3]*256-32768;
  }
  //this values have to be subtracted from the gyro values
  for (int i=0; i<2; i++) {
	calib->GyroBiasHeaders[i] = report[40+i*8] + report[41+i*8]*256;
	calib->GyroBiasVectors[i][0] = report[42+i*8+0] + report[42+i*8+1]*256-32768;
	calib->GyroBiasVectors[i][1] = report[42+i*8+4] + report[42+i*8+5]*256-32768;
	calib->GyroBiasVectors[i][2] = report[42+i*8+2] + report[42+i*8+3]*256-32768;
  }
  // from 56 to 64 are all zeros, 65 is 0x01, I don't know what (if anything) it means

  // this part is actually a matrix, or 3 vectors, in little-endian, with each one preceded by 0x01E0 (480)
  // the matrix contains the gyro readings at 8 rpm to remove the bias, one of the bias vectors must be substracted
  calib->GyroHeader = report[66]+report[67]*256; // another strange header
  char map2[3] = {0,2,1}; // convert to order x,y,z
  for (int v=0; v<3; v++) {
	int i = map2[v];
	calib->GyroVectors[v][0] = report[70+i*8+0]+report[70+i*8+1]*256-32768;
	calib->GyroVectors[v][1] = report[70+i*8+4]+report[70+i*8+5]*256-32768;
	calib->GyroVectors[v][2] = report[70+i*8+2]+report[70+i*8+3]*256-32768;
  }

  calib->UnknownHeader = report[92]+report[93]*256; // another strange header
  // from 92 to 126 there are 8 float values, 2 vectors and than 2 independent values
  for (int i=0; i<2; i++) {
	  int intValue;
	  intValue=(report[94+i*12+0]+(report[94+i*12+1]<<8)+(report[94+i*12+2]<<16)+(report[94+i*12+3]<<24));
	  calib->UnknownVectors[i][0]=*reinterpret_cast<float*>(&intValue);
	  intValue=(report[94+i*12+8]+(report[94+i*12+9]<<8)+(report[94+i*12+10]<<16)+(report[94+i*12+11]<<24));
	  calib->UnknownVectors[i][1]=*reinterpret_cast<float*>(&intValue);
	  intValue=(report[94+i*12+4]+(report[94+i*12+5]<<8)+(report[94+i*12+6]<<16)+(report[94+i*12+7]<<24));
	  calib->UnknownVectors[i][2]=*reinterpret_cast<float*>(&intValue);
  }
  //the remaining 2 float values
  for (int i=0; i<2; i++) {
	  int intValue=(report[118+i*4+0]+(report[118+i*4+1]<<8)+(report[118+i*4+2]<<16)+(report[118+i*4+3]<<24));
	  calib->UnknownValues[i]=*reinterpret_cast<float*>(&intValue);
  }
	return TRUE;
}

int ReadMoveBluetoothSettings(int index, PMoveBluetooth bt) {
	if (index>=MAXMOVES || index<0 || !bt) return FALSE;
  unsigned char *report = bt->report;
  if (!ReadMoveFeature(index, 0x10, report)) return FALSE;
  memcpy(bt->MoveBtMac,&report[1],6);
  memcpy(bt->HostBtMac,&report[10],6);
  memcpy(bt->unknown,&report[7],3);
  snprintf(bt->MoveBtMacString,18,"%02X:%02X:%02X:%02X:%02X:%02X",report[6],report[5],report[4],report[3],report[2],report[1]);
  snprintf(bt->HostBtMacString,18,"%02X:%02X:%02X:%02X:%02X:%02X",report[15],report[14],report[13],report[12],report[11],report[10]);
	return TRUE;
}

