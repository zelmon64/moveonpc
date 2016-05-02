#pragma once

#include "MovePrecompiled.h"

namespace Move
{
	class MOVE_EXPORT IMoveCalibrationObserver
	{
	public:
		virtual bool calibrationDone()=0;
	};
}