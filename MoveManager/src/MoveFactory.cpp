#include "MovePrecompiled.h"
#include "MoveFactory.h"


namespace Move
{
	IMoveManager* createRealDevice()
	{
		return new MoveManager;
	}

	IMoveManager* createEmulator()
	{
		return new MoveEmulator;
	}
}