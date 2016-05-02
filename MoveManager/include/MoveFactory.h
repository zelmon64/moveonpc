#pragma once

#include "IMoveManager.h"
#include "MoveManager.h"
#include "MoveEmulator.h"
#include "MovePrecompiled.h"

namespace Move
{
	MOVE_EXPORT IMoveManager* createRealDevice();
	MOVE_EXPORT IMoveManager* createEmulator();
}