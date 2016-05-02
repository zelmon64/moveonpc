#include "MovePrecompiled.h"
#include "MoveController.h"

namespace Move
{
	MoveController::MoveController(int p_id, MoveManager* p_manager)
	{
		id=p_id;
		manager=p_manager;

		calibration = new MoveCalibration;
		orientation = new MoveOrientation(calibration);

		lastSeqNumber=-1;
		firstPackage=true;

		packageGot=0;
		missingPackage=0;
	}


	MoveController::~MoveController(void)
	{
	}

	void MoveController::Update()
	{
		TMove m, old;
		if (!ReadMove(id, &m, &old)) return;

		if (firstPackage)
		{
			lastSeqNumber = m.SeqNumber;
			lastTimestamp = m.Timestamp;
			data.buttons = m.Buttons;
			firstPackage=false;
			return;
		}

		if (m.SeqNumber == lastSeqNumber)
			return;
		if (lastSeqNumber!=m.SeqNumber-1 && lastSeqNumber!=m.SeqNumber+15 && packageGot>200)
			missingPackage++;
		packageGot++;
		int ticksEllapsed =m.Timestamp-lastTimestamp;
		if (ticksEllapsed<0)
			ticksEllapsed+=65536;
		float timeEllapsed=(float)ticksEllapsed/100000.0f;
		lastSeqNumber = m.SeqNumber;
		lastTimestamp = m.Timestamp;

		Vector3 MoveAcc, MoveGyro, MoveMag;
		MoveAcc = Vector3(old.RawForceX,old.RawForceY,old.RawForceZ);
		MoveGyro = Vector3(-old.RawGyroPitch,old.RawGyroYaw,-old.RawGyroRoll);
		MoveMag = Vector3(old.RawMagnetX,old.RawMagnetY,old.RawMagnetZ);

		calibration->Update(MoveAcc,MoveGyro,MoveMag,timeEllapsed/2.0f);
		orientation->Update(MoveAcc,MoveGyro,MoveMag,timeEllapsed/2.0f);

		MoveAcc = Vector3(m.RawForceX,m.RawForceY,m.RawForceZ);
		MoveGyro = Vector3(-m.RawGyroPitch,m.RawGyroYaw,-m.RawGyroRoll);
		MoveMag = Vector3(m.RawMagnetX,m.RawMagnetY,m.RawMagnetZ);

		calibration->Update(MoveAcc,MoveGyro,MoveMag,timeEllapsed/2.0f);
		orientation->Update(MoveAcc,MoveGyro,MoveMag,timeEllapsed/2.0f);

		data.orientation=orientation->GetOrientation();
		data.angularVelocity=orientation->GetAngularVelocity();
		data.angularAcceleration=orientation->GetAngularAcceleration();

		if (m.Buttons & B_SQUARE && !(data.buttons & B_SQUARE))
		{
			orientation->HighGains();
		}
		if (m.Buttons & B_CROSS && !(data.buttons & B_CROSS))
		{
			orientation->Reset();
		}
		if ((m.Buttons & B_CIRCLE) && !(data.buttons & B_CIRCLE))
		{

		}
		if (m.Buttons & B_TRIANGLE)
		{
			if (manager->getEye())
			{
				manager->getEye()->RegisterBall();
			}
		}
		if (m.Buttons & B_PS)
		{
			//device->closeDevice();
		}
		for (int i=0; i<24; i++)
		{
			int key=(int)pow(2.0,i);
			if ((m.Buttons & key) && !(data.buttons & key))
			{
				manager->moveKeyPressed(id, key);
			}
			if (!(m.Buttons & key) && (data.buttons & key))
			{
				manager->moveKeyReleased(id, key);
			}
		}
		data.buttons = m.Buttons;
		SetMoveColour(id,255,255,255);

		//if camera is capturing
		if (manager->getEye())
		{
			//TODO: initial solution
			data.isOnDisplay=false;
			if (manager->getEye()->ballFound)
			{
				Vector3 previousPosition=data.position;
				Vector3 previousVelocity=data.velocity;
				data.position.x=manager->getEye()->ballX;
				data.position.y=manager->getEye()->ballY;
				data.position.z=2.0*manager->getEye()->ballZ-50;
				data.velocity=(data.position-previousPosition)/timeEllapsed;
				data.acceleration=(data.velocity-previousVelocity)/timeEllapsed;

				//TODO: initial solution
				if (data.position.x>-50.0f && data.position.x<50.0f && data.position.y>-50.0f && data.position.y<50.0f)
				{
					data.isOnDisplay=true;
					data.displayPos=(data.position+Vector3(50.0f,50.0f,0.0f))/100.0f;
				}
			}
		}
		manager->moveUpdated(id, data);
	}

	void MoveController::StartCalibration()
	{
		calibration->startCalibration();
	}

	void MoveController::EndCalibration()
	{
		calibration->endCalibration();
	}

}