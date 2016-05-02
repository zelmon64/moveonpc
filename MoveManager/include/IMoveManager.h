#pragma once

#include "MovePrecompiled.h"
#include "moveonpc.h"
#include "IMoveObserver.h"
#include "IMoveCalibrationObserver.h"

namespace Move
{

	class MOVE_EXPORT IMoveManager
	{
	public:
		//initialization
		virtual int initMoves()=0;
		virtual void initCamera()=0;

		//observers
		virtual void subsribeMove(IMoveObserver* observer)=0;
		virtual void unsubsribeMove(IMoveObserver* observer)=0;

		virtual void subsribeCalibration(IMoveCalibrationObserver* observer)=0;
		virtual void unsubsribeCalibration(IMoveCalibrationObserver* observer)=0;

		//move data
		virtual Quaternion getOrientation(int id)=0;
		virtual Vector3 getAngularVelocity(int id)=0;
		virtual Vector3 getAngularAcceleration(int id)=0;
		virtual Vector3 getPosition(int id)=0;
		virtual Vector3 getVelocity(int id)=0;
		virtual Vector3 getAcceleration(int id)=0;
		virtual bool getButtonState(int id, int buttonId)=0;
		virtual bool pointingToDisplay(int id)=0;
		virtual Vector3 displayPosition(int id)=0;
		virtual int getFrameRate(){return 0;}
		virtual PBYTE getEyeBuffer(){return 0;}
		virtual void getEyeDimensions(int &x, int &y){}

		//move calibration
		virtual bool readCalibrationData(int id){return false;}
		virtual void startCalibration(int id){}
		virtual void endCalibration(int id){}

		//events for emulator
		virtual void processKeyInput(int key){}
		//virtual void PassKeyPressed(const OIS::KeyEvent &arg){}
		//virtual void PassKeyReleased(const OIS::KeyEvent &arg){}
		//virtual void PassmouseMoved( const OIS::MouseEvent &arg ){}
		//virtual void PassWindowParam(int width, int height){}
	};
}