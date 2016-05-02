#pragma once
#include "MoveOrientation.h"
#include "MoveData.h"
#include "MoveManager.h"
#include "EyeInterface.h"

namespace Move
{
	//forward decleration
	class MoveManager;

	class MoveController
	{
		int id;
		MoveManager* manager;

		MoveOrientation* orientation;
		MoveCalibration* calibration;

		int lastSeqNumber;
		int lastTimestamp;

		int packageGot;
		int missingPackage;

		bool firstPackage;

	public:
		MoveData data;

		MoveController(int p_id, MoveManager* p_manager);
		~MoveController(void);

		void Update();
		void StartCalibration();
		void EndCalibration();
	};
}