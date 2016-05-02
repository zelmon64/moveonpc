#include "MovePrecompiled.h"
#include "MoveManager.h"
#include "moveonpc.h"
#include "MoveRawCalibration.h"

namespace Move
{
	MoveManager::MoveManager(void)
	{
		FPS=0;
		eyeInt=0;
		moveCount=-1;
	}


	MoveManager::~MoveManager(void)
	{
		for (int i=0;i<moveCount;++i)
		{
			delete moves[i];
		}
		if (eyeInt)
		{
			delete eyeInt;
		}
	}

	int MoveManager::initMoves()
	{
		//if already initialized or there is no moves found, return -1
		if (moveCount!=-1 || !OpenMoves())
		{
			return -1;
		}
		moveCount = GetMoveCount();

		for (int i=0;i<moveCount;++i)
		{
			//TODO: read BT Mac address to identify the device
			moves.push_back(new MoveController(i, this));
		}

		_hThread = CreateThread(NULL, 0, &MoveManager::controlThread, this, 0, 0);
		SetPriorityClass(_hThread,REALTIME_PRIORITY_CLASS);
		SetThreadPriority(_hThread,THREAD_PRIORITY_TIME_CRITICAL);
		return moveCount;
	}

	void MoveManager::getEyeDimensions(int &x, int &y)
	{
		eyeInt->getDimensions(x,y);
	}

	Quaternion MoveManager::getOrientation(int id)
	{
		return moves[id]->data.orientation;
	}

	Vector3 MoveManager::getPosition(int id)
	{
		//TODO: initial solution
		if (eyeInt->ballFound)
			return moves[id]->data.position;
		else
			return Vector3(0,0,0);
	}

	int MoveManager::getFrameRate()
	{
		return FPS;
	}

	void MoveManager::initCamera()
	{
		eyeInt=new Eye::EyeInterface;
		eyeInt->startCapture();
	}

	PBYTE MoveManager::getEyeBuffer()
	{
		if (eyeInt && eyeInt->img)
		{
			return eyeInt->img->getData();
		}
		return 0;
	}

	void MoveManager::run()
	{
		while (true)
		{
			for (int i=0;i<moveCount;++i)
			{
				moves[i]->Update();
			}
			Sleep(10);
		}
	}

	Vector3 MoveManager::getAngularVelocity(int id)
	{
		return moves[id]->data.angularVelocity;
	}

	Vector3 MoveManager::getAngularAcceleration(int id)
	{
		return moves[id]->data.angularAcceleration;
	}

	Vector3 MoveManager::getVelocity(int id)
	{
		return moves[id]->data.velocity;
	}	

	Vector3 MoveManager::getAcceleration(int id)
	{
		return moves[id]->data.acceleration;
	}

	bool MoveManager::getButtonState(int id, int buttonId)
	{
		return moves[id]->data.buttons & buttonId;
	}

	bool MoveManager::pointingToDisplay(int id)
	{
		return moves[id]->data.isOnDisplay;
	}

	Vector3 MoveManager::displayPosition(int id)
	{
		return moves[id]->data.displayPos;
	}

	DWORD WINAPI MoveManager::controlThread(LPVOID instance)
	{
		MoveManager *pThis = (MoveManager *)instance;
		pThis->run();
		return 0;
	}

	bool MoveManager::readCalibrationData(int id)
	{
		TMoveCalib calib;
		if (!ReadMoveCalibration(id,&calib))
			return false;

		//TMoveBluetooth bt;
		//if (!Move::ReadMoveBluetoothSettings(id,&bt))
		//	return false;

		////save calibration
		//device name
		char deviceName[20];
		//TODO: generate a unique device name
		sprintf(deviceName,"Device");
		MoveRawCalibration rawCalib=MoveRawCalibration(calib);
		rawCalib.save(deviceName);

		return true;
	}

	void MoveManager::startCalibration(int id)
	{
		moves[id]->StartCalibration();
	}

	void MoveManager::endCalibration(int id)
	{
		moves[id]->EndCalibration();
		//TODO: event handling
		//eventCalibrationDone();
	}

	Eye::EyeInterface* MoveManager::getEye()
	{
		return eyeInt;
	}

	void MoveManager::subsribeMove(IMoveObserver* observer)
	{
		observers.push_back(observer);
	}
	void MoveManager::unsubsribeMove(IMoveObserver* observer)
	{
		observers.remove(observer);
	}

	void MoveManager::subsribeCalibration(IMoveCalibrationObserver* observer)
	{
		calibrationObservers.push_back(observer);
	}
	void MoveManager::unsubsribeCalibration(IMoveCalibrationObserver* observer)
	{
		calibrationObservers.remove(observer);
	}

	void MoveManager::moveUpdated(int moveId, MoveData data)
	{
		std::list<IMoveObserver*>::iterator it;
		for ( it=observers.begin() ; it != observers.end(); it++ )
		{
			(*it)->moveUpdated(moveId, data);
		}
	}
	void MoveManager::moveKeyPressed(int moveId, int keyCode)
	{
		std::list<IMoveObserver*>::iterator it;
		for ( it=observers.begin() ; it != observers.end(); it++ )
		{
			(*it)->moveKeyPressed(moveId, keyCode);
		}
	}
	void MoveManager::moveKeyReleased(int moveId, int keyCode)
	{
		std::list<IMoveObserver*>::iterator it;
		for ( it=observers.begin() ; it != observers.end(); it++ )
		{
			(*it)->moveKeyReleased(moveId, keyCode);
		}
	}
}

