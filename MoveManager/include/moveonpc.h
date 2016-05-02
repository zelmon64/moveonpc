// Playstation Move on the PC, shared header file
// this file contains function prototypes
// by Carl Kenner 4 October 2010 - 16 November 2010
/*
   This file (and anything derived from it, based on it, or using it) is released under a 
   non-military license, it may not be used for any military purpose, including military 
   research, or military funded research, training, recruitment, troop entertainment, 
   design of military systems, or anything else involving the military. I didn't write this
   to help kill people, sorry.
*/

#include "MovePrecompiled.h"

#pragma once

// Maximum number of moves to support.
#define MAXMOVES 4

#ifdef _MSC_VER 
#define snprintf sprintf_s
#endif

#define B_L2        0x1
#define B_R2        0x2
#define B_L1        0x4
#define B_R1        0x8
#define B_TRIANGLE 0x10
#define B_CIRCLE   0x20
#define B_CROSS    0x40
#define B_SQUARE   0x80
#define B_SELECT  0x100
#define B_L3      0x200
#define B_R3      0x400
#define B_START   0x800
#define B_UP     0x1000
#define B_RIGHT  0x2000
#define B_DOWN   0x4000
#define B_LEFT   0x8000
#define B_PS    0x10000
#define B_UNK1  0x20000
#define B_UNK2  0x40000
#define B_MOVE  0x80000
#define B_T    0x100000
#define B_UNK3 0x200000
#define B_UNK4 0x400000
#define B_UNK5 0x800000

#define BAT_LOW       0x02
#define BAT_CHARGING  0xEE
#define BAT_PLUGGEDIN 0xEF

// Vendor ID and Product ID of various devices
#define VID_NOKIA            0x0421
#define VID_LOGITECH         0x046D

#define VID_MICROSOFT        0x045E
#define     PID_XBOX360CONTROL  0x028E
#define     PID_KINECTMIC       0x02AD
#define     PID_KINECTCAMERA    0x02AE
#define     PID_KINECTMOTOR     0x02B0

#define VID_SONY             0x054C
#define     PID_SIXAXIS         0x0268
#define     PID_BDREMOTE        0x0306
#define     PID_KEYPAD          0x03A0
#define     PID_MOVE            0x03D5
#define     PID_NAVIGATION      0x042F
#define     PID_BUZZWIRELESS    0x1000

#define VID_NINTENDO         0x057E
#define     PID_WIIMOTE         0x0306
#define VID_MOTIONINJOY      0x8888
#define     PID_MOTIONINJOY0    0x0008
#define     PID_MOTIONINJOY1    0x0108
#define     PID_MOTIONINJOY2    0x0208
#define     PID_MOVEMOTIONINJOY 0x0308

#define VID_BROADCOM         0x0A5C
#define VID_MAXTOR           0x0D49

#define VID_IN2GAMESHUB      0x0F30
#define     PID_GAMETRAKHUB  0x001C
#define VID_IN2GAMES         0x14B7
#define     PID_GAMETRAK     0x0982

#define VID_VUZIX            0x1BAE
#define     PID_VR920           0x0002
#define VID_ESSENTIALREALITY 0x0D7F
#define     PID_P5              0x0100
#define VID_OCZNIA           0x1234
#define     PID_OCZNIA          0x0000
#define VID_RITMO            0x0810
#define     PID_RITMOPAD        0x0001
#define VID_PNI              0x20FF
#define     PID_SPACEPOINT      0x0100
#define VID_PPJOY            0xDEAD
#define     PID_PPJOY1          0xBEF0
#define     PID_PPJOY16         0xBEFF

typedef struct tmove {
	int Buttons;
	int TAnalog;
	int RawBattery;
	int RawForceX, RawForceY, RawForceZ;
	int RawGyroYaw, RawGyroPitch, RawGyroRoll;
	int RawTemperature;
	int RawMagnetX, RawMagnetY, RawMagnetZ;
	int Timestamp;
	int SeqNumber;
	unsigned char report[49];
} TMove, *PMove;

typedef struct tnav {
	int Buttons;
  int L1Analog, L2Analog;
  int UpAnalog, DownAnalog, LeftAnalog, RightAnalog;
  int LeftStickX, LeftStickY;
	unsigned char report[49];
} TNav, *PNav;

typedef struct tsixaxis {
	int Buttons;
	int L1Analog, L2Analog, R1Analog, R2Analog, 
      TriangleAnalog, CircleAnalog, SquareAnalog, CrossAnalog,
      UpAnalog, DownAnalog, LeftAnalog, RightAnalog;
	int RawForceX, RawForceY, RawForceZ;
  int LeftStickX, LeftStickY, RightStickX, RightStickY;
	int RawGyroYaw;
	unsigned char report[49];
} TSixaxis, *PSixaxis;

typedef struct tmoveoutput {
	unsigned char r, g, b;
	unsigned char rumble;
	unsigned framecount;
} TMoveOutput, *PMoveOutput;

typedef struct tsixaxisoutput {
  unsigned char report[49];
} TSixaxisOutput, *PSixaxisOutput;

typedef struct tmovecalib {
  short int AccHeader;
  short int AccVectors[6][3];
  short int GyroBiasHeaders[2];
  short int GyroBiasVectors[2][3];
  short int GyroHeader;
  short int GyroVectors[3][3];
  short int UnknownHeader;
  float UnknownVectors[2][3];
  float UnknownValues[2];
	unsigned char report[49+47+47];
} TMoveCalib, *PMoveCalib;

typedef struct tmovebluetooth {
  char MoveBtMacString[18], HostBtMacString[18];
  unsigned char MoveBtMac[6], HostBtMac[6];
  unsigned char unknown[3];
	unsigned char report[16];
} TMoveBluetooth, *PMoveBluetooth;

int OpenMoves();
int GetMoveCount();
int GetNavCount();
int GetSixaxisCount();
int GetGametrakCount();
int GetMotionInJoyCount();
int ReadMoveReport(int index, void *report);
int WriteMoveReport(int index, void *report);
int ReadMoveFeature(int index, int feature, void *report);
int WriteMoveFeature(int index, int feature, void *report);
int ReadNavReport(int index, void *report);
int WriteNavReport(int index, void *report);
int ReadNavFeature(int index, int feature, void *report);
int WriteNavFeature(int index, int feature, void *report);
int ReadSixaxisReport(int index, void *report);
int WriteSixaxisReport(int index, void *report);
int ReadSixaxisFeature(int index, int feature, void *report);
int WriteSixaxisFeature(int index, int feature, void *report);
int CloseMoves();

int SetMoveColour(int index, int r, int g, int b);
int SetMoveRumble(int index, int rumble);
int SetDualShock3Rumble(int index, int rumble);
int SetSixaxisLeds(int index, int leds);
int ReadMove(int index, PMove data, PMove old=NULL);
int ReadNav(int index, PNav data);
int ReadSixaxis(int index, PSixaxis data);
int ReadMoveCalibration(int index, PMoveCalib calib);
