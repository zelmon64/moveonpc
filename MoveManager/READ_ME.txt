In order to use the library:
1. Compile the .MoveManager project.
2. Copy the MoveManager.dll and the settings.cfg to your output directory.
3. Use the .MoveManager/include and .MoveManager/lib directories in your solution.

In order to get an accurate calibration (it depens on your move device), get the calibration tool from:
http://ogremove.codeplex.com/releases/view/67445
And follow the steps at:
http://ogremove.codeplex.com/documentation?

Buttons for testing:
CROSS: normal tracking
SQUARE: tracking only with the gyroscope
TRIANGLE: start ball tracking

The glowing ball tracking is only initial. Improvements coming soon.
I wont use opencv for that, because it needs to be much faster than opencv...

If you have questions, ask me at darress AT gmail DOT com

-----------------------------------------
A short example to use the the framework (working version in solution)

//Decleration:
Move::IMoveManager* move;

//Initialization:
move = Move::createRealDevice();
move->initCamera();
if (move->initMoves()<1)
{
	//handle that there's no moves found
}

//In the main loop:
Move::Vector3 movePos = move->getPosition(0);
Move::Quaternion moveOri = move->getOrientation(0);


