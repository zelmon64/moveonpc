#pragma once

#include "MovePrecompiled.h"
#include "MoveCalibration.h"

namespace Move
{
	class MoveOrientation
	{
		Vector3 ori;
		Vector3 ori2;

		Vector3 angularVel;
		Vector3 angularAcc;

		MoveCalibration* calibration;

		float AEq_1, AEq_2, AEq_3, AEq_4;  // quaternion orientation of earth frame relative to auxiliary frame

		// filter variables and constants
		float gyroMeasError;                                            // gyroscope measurement error (in degrees per second)
		float gyroBiasDrift;    
		float beta;   // filter gain beta
		float zeta;   // filter gain zeta
		float SEq_1, SEq_2, SEq_3, SEq_4;                          // estimated orientation quaternion elements with initial conditions
		float b_x, b_z;                                                    // estimated direction of earth's magnetic field in the earth frame
		float w_bx, w_by, w_bz;                                        // estimate gyroscope biases error

		volatile float ASq_1, ASq_2, ASq_3, ASq_4;

	public:
		MoveOrientation(MoveCalibration* calib);
		~MoveOrientation(void);
		void Update(Vector3 acc, Vector3 gyro, Vector3 mag, float deltat);
		void Update2(Vector3 acc, Vector3 gyro, Vector3 mag, float deltat);
		Quaternion GetOrientation();
		Vector3 GetAngularVelocity();
		Vector3 GetAngularAcceleration();
		void MoveOrientation::HighGains();
		void Reset();
	};
}
