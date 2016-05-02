#include "../include/MovePrecompiled.h"
#include "../include/MoveEmulator.h"
#include "../include/IniFile.h"

using namespace std;


namespace Move
{
	MoveEmulator::MoveEmulator(void)
	{
		roll =0;
		pitch =0;
		yaw =0;

		EmulatorSpeed = 0.0005;

		for(int i=0; i<3; i++)
			for(int j=0; j<2; j++)
				KeyMap[i][j] = false;

	}


	MoveEmulator::~MoveEmulator(void)
	{
	}

	int MoveEmulator::initMoves()
	{
		string emu_mode = CIniFile::GetValue("mode","Emulator","settings.cfg");
		if(emu_mode.compare("controller")==0)
		{
			EmuMode = CONTROLLER;
			return 1;
		}
		if(emu_mode.compare("read_log")==0)
		{
			EmuMode = READ_LOG;
			string emu_file = CIniFile::GetValue("emu_file","Emulator","settings.cfg");
			std::ifstream file;
			file.open( emu_file, std::ifstream::in);
			float w,x,y,z;
			char c;
			while(file>>w>>c>>x>>c>>y>>c>>z)
			{
				Quaternion q(w,x,y,z);
				orientations.push(q);
			}
			file.close();
			return 1;
		}
		// returns the number of moves
		return 0;
	}

	Quaternion MoveEmulator::getOrientation(int id)
	{

		if(EmuMode == READ_LOG)
		{
			if(!orientations.empty())
			{
				Quaternion q = orientations.front();
				orientations.pop();
				return q;
			}
			else
				return Quaternion(1,0,0,0);
		}


		else if(EmuMode == CONTROLLER)
		{
			//[cos(a/2), sin(a/2) * nx, sin(a/2)* ny, sin(a/2) * nz]


			/*if(KeyMap[1][0])
				pitch -= EmulatorSpeed;

			if(KeyMap[1][1])
				pitch += EmulatorSpeed;

			if(KeyMap[0][1])
				roll += EmulatorSpeed;

			if(KeyMap[2][1])
				roll -= EmulatorSpeed;

			if(KeyMap[0][0])
				yaw += EmulatorSpeed;

			if(KeyMap[2][0])
				yaw -= EmulatorSpeed;*/



			float roll_angle = cos(roll/2.0);
			float pitch_angle = cos(pitch/2.0);
			float yaw_angle = cos(yaw/2.0);
			
			Quaternion q1(roll_angle,0,0,(sin(roll/2.0)));
			Quaternion q2(pitch_angle,(sin(pitch/2.0)),0,0);
			Quaternion q3(yaw_angle,0,(sin(yaw/2.0)),0);

			//if (abs(test)>=3.14)
			//	//add *= -1;
			//	test = 3.14;
			//else
			//test += add;


			//float add = 0.005;
			/*if (abs(test)>=(3.14/2.0))
			test = (3.14/2.0);
			else
			test += add;*/
			//return q2*q1;
			return q2*q1*q3;
			//return q1*q2;
		}
		else
				return Quaternion(1,0,0,0);

	}

	Vector3 MoveEmulator::getPosition(int id)
	{
		if(EmuMode ==CONTROLLER)
			return position;
		else
			return Vector3(0,0,0);
	}

	Vector3 MoveEmulator::getAngularVelocity(int id){return Vector3(0,0,0);}
	Vector3 MoveEmulator::getAngularAcceleration(int id){return Vector3(0,0,0);}
	Vector3 MoveEmulator::getVelocity(int id){return Vector3(0,0,0);}
	Vector3 MoveEmulator::getAcceleration(int id){return Vector3(0,0,0);}
	bool MoveEmulator::getButtonState(int id, int buttonId){return false;}
	bool MoveEmulator::pointingToDisplay(int id){return false;}
	Vector3 MoveEmulator::displayPosition(int id){return Vector3(0,0,0);}

	void MoveEmulator::initCamera()
	{
	}

	void MoveEmulator::processKeyInput(int key)
	{
		/*switch(key)
		{
		case OIS::KC_I: pitch -= EmulatorSpeed;
						break;
		
		case OIS::KC_K: pitch += EmulatorSpeed;
						break;

		case OIS::KC_J: roll += EmulatorSpeed;
						break;

		case OIS::KC_L: roll -= EmulatorSpeed;
						break;

		case OIS::KC_U: yaw += EmulatorSpeed;
						break;

		case OIS::KC_O: yaw -= EmulatorSpeed;
						break;

		case OIS::KC_N: EmulatorSpeed -= 0.00005;
						if(EmulatorSpeed <=0.0001 )
						EmulatorSpeed =0.0001;
						break;

		case OIS::KC_M: EmulatorSpeed += 0.00005;
						break;
	
		}*/
	
	}

	/*void MoveEmulator::PassKeyPressed(const OIS::KeyEvent &arg)
	{
		if(arg.key == OIS::KC_I)
		{
			KeyMap[1][0] = true;
		}
		if(arg.key == OIS::KC_K)
		{
			KeyMap[1][1] = true;
		}
		if(arg.key == OIS::KC_J)
		{
			KeyMap[0][1] = true;
		}
		if(arg.key == OIS::KC_L)
		{
			KeyMap[2][1] = true;
		}
		if(arg.key == OIS::KC_U)
		{
			KeyMap[0][0] = true;
		}
		if(arg.key == OIS::KC_O)
		{
			KeyMap[2][0] = true;
		}
		if(arg.key == OIS::KC_N)
		{
			EmulatorSpeed -= 0.0005;
			if(EmulatorSpeed <=0.0001 )
				EmulatorSpeed =0.0001;
		}
		if(arg.key == OIS::KC_M)
		{
			EmulatorSpeed += 0.0005;
		}

	}

	void MoveEmulator::PassKeyReleased(const OIS::KeyEvent &arg)
	{
		if(arg.key == OIS::KC_I)
		{
			KeyMap[1][0] = false;
		}
		if(arg.key == OIS::KC_K)
		{
			KeyMap[1][1] = false;
		}
		if(arg.key == OIS::KC_J)
		{
			KeyMap[0][1] = false;
		}
		if(arg.key == OIS::KC_L)
		{
			KeyMap[2][1] = false;
		}
		if(arg.key == OIS::KC_U)
		{
			KeyMap[0][0] = false;
		}
		if(arg.key == OIS::KC_O)
		{
			KeyMap[2][0] = false;
		}
	}*/

	/*void MoveEmulator::PassmouseMoved( const OIS::MouseEvent &arg )
	{
		position.x = -(WindowW/20.0)+arg.state.X.abs/10.0;
		position.y = (WindowH/20.0)+-arg.state.Y.abs/10.0;
		position.z = -arg.state.Z.abs/10.0;
	}*/

	void MoveEmulator::PassWindowParam(int width, int height)
	{
		WindowW = width;
		WindowH = height;
	}

	void MoveEmulator::subsribeMove(IMoveObserver* observer)
	{
		observers.push_back(observer);
	}
	void MoveEmulator::unsubsribeMove(IMoveObserver* observer)
	{
		observers.remove(observer);
	}

	void MoveEmulator::subsribeCalibration(IMoveCalibrationObserver* observer)
	{
		calibrationObservers.push_back(observer);
	}
	void MoveEmulator::unsubsribeCalibration(IMoveCalibrationObserver* observer)
	{
		calibrationObservers.remove(observer);
	}
}
