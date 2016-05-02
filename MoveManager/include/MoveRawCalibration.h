#pragma once
#include "MovePrecompiled.h"
#include "moveonpc.h"

namespace Move
{
	class MoveRawCalibration
	{
	public:
		MoveRawCalibration(char* deviceName);
		MoveRawCalibration(TMoveCalib calib);
		~MoveRawCalibration(void);

		void save(char* deviceName);

	public:
		short int AccHeader;
		Vector3 AccVectors[6];
		short int GyroBiasHeaders[2];
		Vector3 GyroBiasVectors[2];
		short int GyroHeader;
		Vector3 GyroVectors[3];
		short int UnknownHeader;
		Vector3 UnknownVectors[2];
		float UnknownValues[2];
	};

}