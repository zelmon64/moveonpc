#include "../include/MovePrecompiled.h"
#include "../include/MoveRawCalibration.h"
#include "../include/IniFile.h"

using namespace std;

namespace Move
{

	MoveRawCalibration::MoveRawCalibration(char* deviceName)
	{
		string fileName = "raw_calibration.cfg";
		AccHeader=CIniFile::GetInt("accHeader",deviceName, fileName);
		AccVectors[0]=CIniFile::GetVector3("accMinX",deviceName, fileName);
		AccVectors[1]=CIniFile::GetVector3("accMaxX", deviceName, fileName);
		AccVectors[2]=CIniFile::GetVector3("accMinY", deviceName, fileName);
		AccVectors[3]=CIniFile::GetVector3("accMaxY", deviceName, fileName);
		AccVectors[4]=CIniFile::GetVector3("accMinZ", deviceName, fileName);
		AccVectors[5]=CIniFile::GetVector3("accMaxZ", deviceName, fileName);
		GyroBiasHeaders[0]=CIniFile::GetInt("gyroBiasHeader1", deviceName, fileName);
		GyroBiasHeaders[1]=CIniFile::GetInt("gyroBiasHeader2", deviceName, fileName);
		GyroBiasVectors[0]=CIniFile::GetVector3("gyroBias1", deviceName, fileName);
		GyroBiasVectors[1]=CIniFile::GetVector3("gyroBias2", deviceName, fileName);
		GyroHeader=CIniFile::GetInt("gyroHeader", deviceName, fileName);
		GyroVectors[0]=CIniFile::GetVector3("gyroX", deviceName, fileName);
		GyroVectors[1]=CIniFile::GetVector3("gyroY", deviceName, fileName);
		GyroVectors[2]=CIniFile::GetVector3("gyroZ", deviceName, fileName);
		UnknownHeader=CIniFile::GetInt("unknownHeader", deviceName, fileName);
		UnknownVectors[0]=CIniFile::GetVector3("unknownVector1", deviceName, fileName);
		UnknownVectors[1]=CIniFile::GetVector3("unknownVector2", deviceName, fileName);
		UnknownValues[0]=CIniFile::GetFloat("unknownValue1", deviceName, fileName);
		UnknownValues[1]=CIniFile::GetFloat("unknownValue2", deviceName, fileName);
	}

	MoveRawCalibration::MoveRawCalibration(TMoveCalib calib)
	{
		AccHeader=calib.AccHeader;
		for (int i=0; i<6; i++)
			AccVectors[i]=Vector3(calib.AccVectors[i][0],calib.AccVectors[i][1],calib.AccVectors[i][2]);
		for (int i=0; i<2; i++)
			GyroBiasHeaders[i]=calib.GyroBiasHeaders[i];
		for (int i=0; i<2; i++)
			GyroBiasVectors[i]=Vector3(calib.GyroBiasVectors[i][0],calib.GyroBiasVectors[i][1],calib.GyroBiasVectors[i][2]);
		GyroHeader=calib.GyroHeader;
		for (int i=0; i<3; i++)
			GyroVectors[i]=Vector3(calib.GyroVectors[i][0],calib.GyroVectors[i][1],calib.GyroVectors[i][2]);
		UnknownHeader=calib.UnknownHeader;
		for (int i=0; i<2; i++)
			UnknownVectors[i]=Vector3(calib.UnknownVectors[i][0],calib.UnknownVectors[i][1],calib.UnknownVectors[i][2]);
		for (int i=0; i<2; i++)
			UnknownValues[i]=calib.UnknownValues[i];
	}

	void MoveRawCalibration::save(char* deviceName)
	{
		string fileName="raw_calibration.cfg";
		CIniFile::AddSection (deviceName, fileName);

		CIniFile::SetValue("accHeader", AccHeader, deviceName, fileName);
		CIniFile::SetValue("accMinX", AccVectors[0], deviceName, fileName);
		CIniFile::SetValue("accMaxX", AccVectors[1], deviceName, fileName);
		CIniFile::SetValue("accMinY", AccVectors[2], deviceName, fileName);
		CIniFile::SetValue("accMaxY", AccVectors[3], deviceName, fileName);
		CIniFile::SetValue("accMinZ", AccVectors[4], deviceName, fileName);
		CIniFile::SetValue("accMaxZ", AccVectors[5], deviceName, fileName);
		CIniFile::SetValue("gyroBiasHeader1", GyroBiasHeaders[0], deviceName, fileName);
		CIniFile::SetValue("gyroBiasHeader2", GyroBiasHeaders[1], deviceName, fileName);
		CIniFile::SetValue("gyroBias1", GyroBiasVectors[0], deviceName, fileName);
		CIniFile::SetValue("gyroBias2", GyroBiasVectors[1], deviceName, fileName);
		CIniFile::SetValue("gyroHeader", GyroHeader, deviceName, fileName);
		CIniFile::SetValue("gyroX", GyroVectors[0], deviceName, fileName);
		CIniFile::SetValue("gyroY", GyroVectors[1], deviceName, fileName);
		CIniFile::SetValue("gyroZ", GyroVectors[2], deviceName, fileName);
		CIniFile::SetValue("unknownHeader", UnknownHeader, deviceName, fileName);
		CIniFile::SetValue("unknownVector1", UnknownVectors[0], deviceName, fileName);
		CIniFile::SetValue("unknownVector2", UnknownVectors[1], deviceName, fileName);
		CIniFile::SetValue("unknownValue1", UnknownValues[0], deviceName, fileName);
		CIniFile::SetValue("unknownValue2", UnknownValues[1], deviceName, fileName);
	}

	MoveRawCalibration::~MoveRawCalibration(void)
	{
	}

}