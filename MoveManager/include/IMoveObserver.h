#pragma once

#include "MovePrecompiled.h"
#include "MoveData.h"

namespace Move
{
	class MOVE_EXPORT IMoveObserver
	{
	public:
		virtual void moveUpdated(int moveId, MoveData data)=0;
		virtual void moveKeyPressed(int moveId, int keyCode)=0;
		virtual void moveKeyReleased(int moveId, int keyCode)=0;
	};
}