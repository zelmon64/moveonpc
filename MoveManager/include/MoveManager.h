#pragma once

#include "IMoveManager.h"
#include "MoveCalibration.h"
#include "MoveOrientation.h"
#include "EyeInterface.h"
#include "MoveController.h"


namespace Move
{
	//forward decleration
	class MoveController;

	class MoveManager : public IMoveManager
	{
		HANDLE _hThread;

		Eye::EyeInterface* eyeInt;

		int moveCount;
		std::vector<MoveController*> moves;

		std::list<IMoveObserver*> observers;
		std::list<IMoveCalibrationObserver*> calibrationObservers;

		volatile int FPS;

	public:
		MoveManager(void);
		~MoveManager(void);

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
		int getFrameRate();
		PBYTE getEyeBuffer();
		void getEyeDimensions(int &x, int &y);

		//move calibration
		bool readCalibrationData(int id);

		void startCalibration(int id);
		void endCalibration(int id);

		Eye::EyeInterface* getEye();

		void moveUpdated(int moveId, MoveData data);
		void moveKeyPressed(int moveId, int keyCode);
		void moveKeyReleased(int moveId, int keyCode);

	private:
		void run();
		static DWORD WINAPI controlThread(LPVOID instance);
	};
}
