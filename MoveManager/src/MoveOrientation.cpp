#include "MovePrecompiled.h"
#include "MoveOrientation.h"


namespace Move
{
	MoveOrientation::MoveOrientation(MoveCalibration* calib)
	{
		calibration=calib;

		// filter variables and constants
		gyroMeasError = 30;                                            // gyroscope measurement error (in degrees per second)
		gyroBiasDrift = 2;   

		angularVel=Vector3(0,0,0);
		angularAcc=Vector3(0,0,0);

		AEq_1 = 1, AEq_2 = 0, AEq_3 = 0, AEq_4 = 0;

		beta = sqrt(3.0 / 4.0) * (PI * (gyroMeasError / 180.0));   // filter gain beta
		zeta = sqrt(3.0 / 4.0) * (PI * (gyroBiasDrift / 180.0));   // filter gain zeta
		SEq_1 = 1, SEq_2 = 0, SEq_3 = 0, SEq_4 = 0;                          // estimated orientation quaternion elements with initial conditions
		b_x = 1, b_z = 0;                                                    // estimated direction of earth's magnetic field in the earth frame
		w_bx = 0, w_by = 0, w_bz = 0;   
	}


	MoveOrientation::~MoveOrientation(void)
	{
	}

	void MoveOrientation::Update(Vector3 acc, Vector3 gyro, Vector3 mag, float deltat)
	{
		if (deltat==0.0f) return;

		MoveCalibrationData calib = calibration->getCalibrationData();

		acc=calib.accGain*(acc-calib.accBias);
		gyro=calib.gyroGain*gyro;
		mag=calib.magGain*(mag-calib.magBias);
		acc=glm::normalize(acc);
		mag=glm::normalize(mag);

		// local system variables
		float norm;                                                            // vector norm
		float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4;  // quaternion rate from gyroscopes elements
		float f_1, f_2, f_3, f_4, f_5, f_6;                                    // objective function elements
		float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33,              // objective function Jacobian elements
		J_41, J_42, J_43, J_44, J_51, J_52, J_53, J_54, J_61, J_62, J_63, J_64; //
		float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4;              // estimated direction of the gyroscope error (quaternion derrivative)
		float w_err_x, w_err_y, w_err_z;                                       // estimated direction of the gyroscope error (angular)
		float h_x, h_y, h_z;                                                   // computed flux in the earth frame

		// axulirary variables to avoid reapeated calcualtions
		float halfSEq_1 = 0.5f * SEq_1;
		float halfSEq_2 = 0.5f * SEq_2;
		float halfSEq_3 = 0.5f * SEq_3;
		float halfSEq_4 = 0.5f * SEq_4;
		float twoSEq_1 = 2.0f * SEq_1;
		float twoSEq_2 = 2.0f * SEq_2;
		float twoSEq_3 = 2.0f * SEq_3;
		float twoSEq_4 = 2.0f * SEq_4;
		float twob_x = 2 * b_x;
		float twob_z = 2 * b_z;
		float twob_xSEq_1 = 2 * b_x * SEq_1;
		float twob_xSEq_2 = 2 * b_x * SEq_2;
		float twob_xSEq_3 = 2 * b_x * SEq_3;
		float twob_xSEq_4 = 2 * b_x * SEq_4;
		float twob_zSEq_1 = 2 * b_z * SEq_1;
		float twob_zSEq_2 = 2 * b_z * SEq_2;
		float twob_zSEq_3 = 2 * b_z * SEq_3;
		float twob_zSEq_4 = 2 * b_z * SEq_4;
		float SEq_1SEq_2;
		float SEq_1SEq_3 = SEq_1 * SEq_3;
		float SEq_1SEq_4;
		float SEq_2SEq_3;
		float SEq_2SEq_4 = SEq_2 * SEq_4;
		float SEq_3SEq_4;
		float twom_x = 2 * mag[0];
		float twom_y = 2 * mag[1];
		float twom_z = 2 * mag[2];

		// compute the objective function and Jacobian
		f_1 = twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - acc[0];
		f_2 = twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - acc[1];
		f_3 = 1.0 - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - acc[2];
		f_4 = twob_x * (0.5 - SEq_3 * SEq_3 - SEq_4 * SEq_4) + twob_z * (SEq_2SEq_4 - SEq_1SEq_3) - mag[0];
		f_5 = twob_x * (SEq_2 * SEq_3 - SEq_1 * SEq_4) + twob_z * (SEq_1 * SEq_2 + SEq_3 * SEq_4) - mag[1];
		f_6 = twob_x * (SEq_1SEq_3 + SEq_2SEq_4) + twob_z * (0.5 - SEq_2 * SEq_2 - SEq_3 * SEq_3) - mag[2];
		J_11or24 = twoSEq_3;                                                    // J_11 negated in matrix multiplication
		J_12or23 = 2 * SEq_4;
		J_13or22 = twoSEq_1;                                                    // J_12 negated in matrix multiplication
		J_14or21 = twoSEq_2;
		J_32 = 2 * J_14or21;                                                    // negated in matrix multiplication
		J_33 = 2 * J_11or24;                                                    // negated in matrix multiplication
		J_41 = twob_zSEq_3;                                                     // negated in matrix multiplication
		J_42 = twob_zSEq_4;
		J_43 = 2 * twob_xSEq_3 + twob_zSEq_1;                                   // negated in matrix multiplication
		J_44 = 2 * twob_xSEq_4 - twob_zSEq_2;                                   // negated in matrix multiplication
		J_51 = twob_xSEq_4 - twob_zSEq_2;                                       // negated in matrix multiplication
		J_52 = twob_xSEq_3 + twob_zSEq_1;
		J_53 = twob_xSEq_2 + twob_zSEq_4;
		J_54 = twob_xSEq_1 - twob_zSEq_3;                                       // negated in matrix multiplication
		J_61 = twob_xSEq_3;
		J_62 = twob_xSEq_4 - 2 * twob_zSEq_2;
		J_63 = twob_xSEq_1 - 2 * twob_zSEq_3;
		J_64 = twob_xSEq_2;

		// compute the gradient (matrix multiplication)
		SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1 - J_41 * f_4 - J_51 * f_5 + J_61 * f_6;
		SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3 + J_42 * f_4 + J_52 * f_5 + J_62 * f_6;
		SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1 - J_43 * f_4 + J_53 * f_5 + J_63 * f_6;
		SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2 - J_44 * f_4 - J_54 * f_5 + J_64 * f_6;

		// normalise the gradient to estimate direction of the gyroscope error
		norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
		SEqHatDot_1 = SEqHatDot_1 / norm;
		SEqHatDot_2 = SEqHatDot_2 / norm;
		SEqHatDot_3 = SEqHatDot_3 / norm;
		SEqHatDot_4 = SEqHatDot_4 / norm;

		// compute angular estimated direction of the gyroscope error
		w_err_x = twoSEq_1 * SEqHatDot_2 - twoSEq_2 * SEqHatDot_1 - twoSEq_3 * SEqHatDot_4 + twoSEq_4 * SEqHatDot_3;
		w_err_y = twoSEq_1 * SEqHatDot_3 + twoSEq_2 * SEqHatDot_4 - twoSEq_3 * SEqHatDot_1 - twoSEq_4 * SEqHatDot_2;
		w_err_z = twoSEq_1 * SEqHatDot_4 - twoSEq_2 * SEqHatDot_3 + twoSEq_3 * SEqHatDot_2 - twoSEq_4 * SEqHatDot_1;

		// compute and remove the gyroscope baises
		w_bx += w_err_x * deltat * zeta;
		w_by += w_err_y * deltat * zeta;
		w_bz += w_err_z * deltat * zeta;
		gyro[0] -= w_bx;
		gyro[1] -= w_by;
		gyro[2] -= w_bz;

		angularAcc=(gyro-angularVel)/deltat;
		angularVel=gyro;

		// compute the quaternion rate measured by gyroscopes
		SEqDot_omega_1 = -halfSEq_2 * gyro[0] - halfSEq_3 * gyro[1] - halfSEq_4 * gyro[2];
		SEqDot_omega_2 = halfSEq_1 * gyro[0] + halfSEq_3 * gyro[2] - halfSEq_4 * gyro[1];
		SEqDot_omega_3 = halfSEq_1 * gyro[1] - halfSEq_2 * gyro[2] + halfSEq_4 * gyro[0];
		SEqDot_omega_4 = halfSEq_1 * gyro[2] + halfSEq_2 * gyro[1] - halfSEq_3 * gyro[0];

		// compute then integrate the estimated quaternion rate
		SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
		SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
		SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
		SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;

		// normalise quaternion
		norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
		SEq_1 /= norm;
		SEq_2 /= norm;
		SEq_3 /= norm;
		SEq_4 /= norm;

		// compute flux in the earth frame
		SEq_1SEq_2 = SEq_1 * SEq_2;                                             // recompute axulirary variables
		SEq_1SEq_3 = SEq_1 * SEq_3;
		SEq_1SEq_4 = SEq_1 * SEq_4;
		SEq_3SEq_4 = SEq_3 * SEq_4;
		SEq_2SEq_3 = SEq_2 * SEq_3;
		SEq_2SEq_4 = SEq_2 * SEq_4;
		h_x = twom_x * (0.5 - SEq_3 * SEq_3 - SEq_4 * SEq_4) + twom_y * (SEq_2SEq_3 - SEq_1SEq_4) + twom_z * (SEq_2SEq_4 + SEq_1SEq_3);
		h_y = twom_x * (SEq_2SEq_3 + SEq_1SEq_4) + twom_y * (0.5 - SEq_2 * SEq_2 - SEq_4 * SEq_4) + twom_z * (SEq_3SEq_4 - SEq_1SEq_2);
		h_z = twom_x * (SEq_2SEq_4 - SEq_1SEq_3) + twom_y * (SEq_3SEq_4 + SEq_1SEq_2) + twom_z * (0.5 - SEq_2 * SEq_2 - SEq_3 * SEq_3);

		// normalise the flux vector to have only components in the x and z
		b_x = sqrt((h_x * h_x) + (h_y * h_y));
		b_z = h_z;

		float ESq_1, ESq_2, ESq_3, ESq_4;                              // quaternion describing orientation of sensor relative to earth

		// compute the quaternion conjugate
		ESq_1 = SEq_1;
		ESq_2 = -SEq_2;
		ESq_3 = -SEq_3;
		ESq_4 = -SEq_4;

		// compute the quaternion product
		ASq_1 = ESq_1 * AEq_1 - ESq_2 * AEq_2 - ESq_3 * AEq_3 - ESq_4 * AEq_4;
		ASq_2 = ESq_1 * AEq_2 + ESq_2 * AEq_1 + ESq_3 * AEq_4 - ESq_4 * AEq_3;
		ASq_3 = ESq_1 * AEq_3 - ESq_2 * AEq_4 + ESq_3 * AEq_1 + ESq_4 * AEq_2;
		ASq_4 = ESq_1 * AEq_4 + ESq_2 * AEq_3 - ESq_3 * AEq_2 + ESq_4 * AEq_1;
	}

	/*void MoveOrientation::Update2(Vector3 acc, Vector3 gyro, Vector3 mag, float deltat) 
	{
		if (deltat==0.0f) return;

		float Kp =8.0f;			
		float Ki =0.005f;
		float halfT = deltat/2.0f;

		float norm;
		float hx, hy, hz, bx, bz;
		float vx, vy, vz, wx, wy, wz;
		float ex, ey, ez;

		// auxiliary variables to reduce number of repeated operations
		float SEq_1SEq_1 = SEq_1*SEq_1;
		float SEq_1SEq_2 = SEq_1*SEq_2;
		float SEq_1SEq_3 = SEq_1*SEq_3;
		float SEq_1SEq_4 = SEq_1*SEq_4;
		float SEq_2SEq_2 = SEq_2*SEq_2;
		float SEq_2SEq_3 = SEq_2*SEq_3;
		float SEq_2SEq_4 = SEq_2*SEq_4;
		float SEq_3SEq_3 = SEq_3*SEq_3;   
		float SEq_3SEq_4 = SEq_3*SEq_4;
		float SEq_4SEq_4 = SEq_4*SEq_4;          
	
		MoveCalibrationData calib = calibration->getCalibrationData();
		acc=calib.accGain*(acc-calib.accBias);
		gyro=calib.gyroGain*gyro;
		mag=calib.magGain*(mag-calib.magBias);
		acc.normalise();
		mag.normalise();   
	
		// compute reference direction of flux
		hx = 2*mag.x*(0.5 - SEq_3SEq_3 - SEq_4SEq_4) + 2*mag.y*(SEq_2SEq_3 - SEq_1SEq_4) + 2*mag.z*(SEq_2SEq_4 + SEq_1SEq_3);
		hy = 2*mag.x*(SEq_2SEq_3 + SEq_1SEq_4) + 2*mag.y*(0.5 - SEq_2SEq_2 - SEq_4SEq_4) + 2*mag.z*(SEq_3SEq_4 - SEq_1SEq_2);
		hz = 2*mag.x*(SEq_2SEq_4 - SEq_1SEq_3) + 2*mag.y*(SEq_3SEq_4 + SEq_1SEq_2) + 2*mag.z*(0.5 - SEq_2SEq_2 - SEq_3SEq_3);         
		bx = sqrt((hx*hx) + (hy*hy));
		bz = hz;        
	
		// estimated direction of gravity and flux (v and w)
		vx = 2*(SEq_2SEq_4 - SEq_1SEq_3);
		vy = 2*(SEq_1SEq_2 + SEq_3SEq_4);
		vz = SEq_1SEq_1 - SEq_2SEq_2 - SEq_3SEq_3 + SEq_4SEq_4;
		wx = 2*bx*(0.5 - SEq_3SEq_3 - SEq_4SEq_4) + 2*bz*(SEq_2SEq_4 - SEq_1SEq_3);
		wy = 2*bx*(SEq_2SEq_3 - SEq_1SEq_4) + 2*bz*(SEq_1SEq_2 + SEq_3SEq_4);
		wz = 2*bx*(SEq_1SEq_3 + SEq_2SEq_4) + 2*bz*(0.5 - SEq_2SEq_2 - SEq_3SEq_3);  
	
		// error is sum of cross product between reference direction of fields and direction measured by sensors
		ex = (acc.y*vz - acc.z*vy) + (mag.y*wz - mag.z*wy);
		ey = (acc.z*vx - acc.x*vz) + (mag.z*wx - mag.x*wz);
		ez = (acc.x*vy - acc.y*vx) + (mag.x*wy - mag.y*wx);
	
		// integral error scaled integral gain
		w_bx = w_bx + ex*Ki;
		w_by = w_by + ey*Ki;
		w_bz = w_bz + ez*Ki;
	
		// adjusted gyroscope measurements
		gyro.x = gyro.x + Kp*ex + w_bx;
		gyro.y = gyro.y + Kp*ey + w_by;
		gyro.z = gyro.z + Kp*ez + w_bz;

		angularAcc=(gyro-angularVel)/deltat;
		angularVel=gyro;
	
		// integrate quaternion rate and normalise
		SEq_1 = SEq_1 + (-SEq_2*gyro.x - SEq_3*gyro.y - SEq_4*gyro.z)*halfT;
		SEq_2 = SEq_2 + (SEq_1*gyro.x + SEq_3*gyro.z - SEq_4*gyro.y)*halfT;
		SEq_3 = SEq_3 + (SEq_1*gyro.y - SEq_2*gyro.z + SEq_4*gyro.x)*halfT;
		SEq_4 = SEq_4 + (SEq_1*gyro.z + SEq_2*gyro.y - SEq_3*gyro.x)*halfT;  
	
		// normalise quaternion
		norm = sqrt(SEq_1*SEq_1 + SEq_2*SEq_2 + SEq_3*SEq_3 + SEq_4*SEq_4);
		SEq_1 = SEq_1 / norm;
		SEq_2 = SEq_2 / norm;
		SEq_3 = SEq_3 / norm;
		SEq_4 = SEq_4 / norm;

		float ESq_1, ESq_2, ESq_3, ESq_4;                              // quaternion describing orientation of sensor relative to earth
		// compute the quaternion conjugate
		ESq_1 = SEq_1;
		ESq_2 = -SEq_2;
		ESq_3 = -SEq_3;
		ESq_4 = -SEq_4;

		// compute the quaternion product
		ASq_1 = ESq_1 * AEq_1 - ESq_2 * AEq_2 - ESq_3 * AEq_3 - ESq_4 * AEq_4;
		ASq_2 = ESq_1 * AEq_2 + ESq_2 * AEq_1 + ESq_3 * AEq_4 - ESq_4 * AEq_3;
		ASq_3 = ESq_1 * AEq_3 - ESq_2 * AEq_4 + ESq_3 * AEq_1 + ESq_4 * AEq_2;
		ASq_4 = ESq_1 * AEq_4 + ESq_2 * AEq_3 - ESq_3 * AEq_2 + ESq_4 * AEq_1;
	}*/

	Quaternion MoveOrientation::GetOrientation()
	{
		return Quaternion((double)ASq_1,(double)ASq_2,(double)ASq_3,-(double)ASq_4);
	}

	Vector3 MoveOrientation::GetAngularVelocity()
	{
		return angularVel;
	}

	Vector3 MoveOrientation::GetAngularAcceleration()
	{
		return angularAcc;
	}

	void MoveOrientation::HighGains()
	{
		w_bx= w_by= w_bz=0;  
		//if (beta==0)
		//{
		//	beta = 10;
		//	zeta = 5;
		//}
		//else
		//{
			beta = 0;
			zeta = 0;
		//}
		AEq_1 = SEq_1;                                                              // store orientation of auxiliary frame
		AEq_2 = SEq_2;
		AEq_3 = SEq_3;
		AEq_4 = SEq_4;
		//gyroBiasDrift*=1.1;
		//beta = sqrt(3.0 / 4.0) * (Math::PI * (gyroMeasError / 180.0));
		//zeta = sqrt(3.0 / 4.0) * (Math::PI * (gyroBiasDrift / 180.0));
	}

	void MoveOrientation::Reset()
	{
		//gyroBiasDrift/=1.1;
		w_bx= w_by= w_bz=0;    
		beta = sqrt(3.0 / 4.0) * (PI * (gyroMeasError / 180.0));
		zeta = sqrt(3.0 / 4.0) * (PI * (gyroBiasDrift / 180.0));
		AEq_1 = SEq_1;                                                              // store orientation of auxiliary frame
		AEq_2 = SEq_2;
		AEq_3 = SEq_3;
		AEq_4 = SEq_4;
	}
}