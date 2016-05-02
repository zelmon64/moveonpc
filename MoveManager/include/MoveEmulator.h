#pragma once
#include "IMoveManager.h"
#include "MovePrecompiled.h"

namespace Move
{
	class MoveEmulator : public IMoveManager
	{
		enum Mode {READ_LOG,WRITE_LOG,CONTROLLER};
		Mode EmuMode;

		int WindowH;
		int WindowW;

		std::queue<Quaternion> orientations;
		Vector3 position;
		float roll;
		float pitch;
		float yaw;
		float EmulatorSpeed;

		bool KeyMap[3][2];

		std::list<IMoveObserver*> observers;
		std::list<IMoveCalibrationObserver*> calibrationObservers;

	public:
		MoveEmulator(void);
		~MoveEmulator(void);

		//initialization
		int initMoves();
		void initCamera();

		//observers
		void subsribeMove(IMoveObserver* observer);
		void unsubsribeMove(IMoveObserver* observer);

		void subsribeCalibration(IMoveCalibrationObserver* observer);
		void unsubsribeCalibration(IMoveCalibrationObserver* observer);

		//move data
		Quaternion getOrientation(int id);
		Vector3 getAngularVelocity(int id);
		Vector3 getAngularAcceleration(int id);
		Vector3 getPosition(int id);
		Vector3 getVelocity(int id);
		Vector3 getAcceleration(int id);
		bool getButtonState(int id, int buttonId);
		bool pointingToDisplay(int id);
		Vector3 displayPosition(int id);

		//events for emulator
		virtual void processKeyInput(int key);
		//void PassKeyPressed(const OIS::KeyEvent &arg);
		//void PassKeyReleased(const OIS::KeyEvent &arg);
		//void PassmouseMoved( const OIS::MouseEvent &arg );
		void PassWindowParam(int width, int height);
	};
}

