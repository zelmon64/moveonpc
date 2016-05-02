#pragma once
#include "MovePrecompiled.h"
#include "MoveRawCalibration.h"

#define SWAP_MIN(min,value) if(min>value) min=value
#define SWAP_MAX(max,value) if(max<value) max=value
#define SKIPDATA 2
#define MAG_ITERATIONS 100

namespace Move
{
	struct MoveCalibrationData
	{
		Matrix3 gyroGain;

		Vector3 accBias;
		Matrix3 accGain;

		Vector3 magBias;
		Vector3 magGain;
	};

	class MoveCalibration
	{
	private:
		bool isCalibrating;

		int bufLength;
		Vector3* magBuf;
		int skipData;

		MoveCalibrationData data;	
		MoveRawCalibration* rawData;

	public:
		MoveCalibration();
		//MoveCalibration(char btMac[18]);
		~MoveCalibration(void);

		void Update(Vector3 acc, Vector3 gyro, Vector3 mag, float deltat);
		MoveCalibrationData getCalibrationData();

		//move calibration
		void startCalibration();
		void endCalibration();

	public:
		static MoveCalibration* instance;
		static double integrateMagError(std::vector<double> x);
		static double integrateAccError(std::vector<double> x);
		static double integrateGyroError(std::vector<double> x);
	};

}
