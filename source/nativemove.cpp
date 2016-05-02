#include <wtypes.h>
#include <setupapi.h>
extern "C" {
#include "hidsdi.h"
}
#include <iostream>
#include <iomanip>

void* operator new(size_t size,size_t minSize)
{
	return operator new(size > minSize ? size: minSize);
}

int main()
{
	GUID hidGuid;
	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	HDEVINFO hDevInfo;
	HidD_GetHidGuid(&hidGuid);
	hDevInfo = SetupDiGetClassDevs(&hidGuid,NULL,NULL,DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	for (int i = 0;SetupDiEnumDeviceInterfaces(hDevInfo,NULL,&hidGuid,i,&devInterfaceData);i ++)
	{
		DWORD requiredSize;
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevInterfaceDetailData;
		HANDLE hFile;
		SetupDiGetDeviceInterfaceDetail(hDevInfo,&devInterfaceData,NULL,0,&requiredSize,NULL);
		pDevInterfaceDetailData = new(requiredSize) SP_DEVICE_INTERFACE_DETAIL_DATA;
		pDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		SetupDiGetDeviceInterfaceDetail(hDevInfo,&devInterfaceData,pDevInterfaceDetailData,requiredSize,NULL,NULL);
		hFile = CreateFile(pDevInterfaceDetailData->DevicePath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,NULL);
		if (hFile)
		{
			PHIDP_PREPARSED_DATA pPreparsedData;
			if (HidD_GetPreparsedData(hFile,&pPreparsedData))
			{
				HIDP_CAPS capabilities;
				if (HidP_GetCaps(pPreparsedData,&capabilities) == HIDP_STATUS_SUCCESS && capabilities.UsagePage == 1 && capabilities.Usage == 5)
				{
					HIDD_ATTRIBUTES attributes;
					attributes.Size = sizeof(attributes);
					if (HidD_GetAttributes(hFile,&attributes) && attributes.VendorID == 0x054C && attributes.ProductID == 0x03D5)
					{
						OVERLAPPED overlapped;
						BYTE cal[49];
						BYTE out[49];
						BYTE in[49];
						BYTE bt4[16];
						BYTE bt5[23];
						overlapped.hEvent = CreateEvent(NULL,FALSE,TRUE,"");
						overlapped.Offset = 0;
						overlapped.OffsetHigh = 0;
						for (int i = 1;i < sizeof(cal);i ++)
							cal[i] = 0;
						for (int i = 1;i < sizeof(bt4);i ++)
							bt4[i] = 0;
						DWORD read;
						cal[0] = 0x10; //calibration data
						for (int i = 0;i < 3;i ++)
							if (! HidD_GetFeature(hFile,cal,sizeof(cal)))
								std::cerr << "error getting calibration" << std::endl;
						bt4[0] = 0x04; //bluetooth
						if (HidD_GetFeature(hFile,bt4,sizeof(bt4)))
							std::cout << std::hex << /*std::setfill('0') << std::setw(2) <<*/ "move MAC address: " << unsigned int(bt4[6]) << ':' << unsigned int(bt4[5]) << ':' << unsigned int(bt4[4]) << ':' << unsigned int(bt4[3]) << ':' << unsigned int(bt4[2]) << ':' << unsigned int(bt4[1]) << std::endl << "host MAC address: " << unsigned int(bt4[15]) << ':' << unsigned int(bt4[14]) << ':' << unsigned int(bt4[13]) << ':' << unsigned int(bt4[12]) << ':' << unsigned int(bt4[11]) << ':' << unsigned int(bt4[10]) << std::dec << std::endl;
						else
							std::cerr << "error getting bluetooth information" << std::endl;
#ifdef SETBT
						bt5[0] = 0x05; //bluetooth
						for (int i = 1;i < sizeof(bt5);i ++)
							bt5[i] = 0;
						/*specify BT MAC address here!*/
						bt5[6] = 0x00;
						bt5[5] = 0x16;//0x0c;//0x04;//0x19;
						bt5[4] = 0x38;//0x76;//0x61;//0x15;
						bt5[3] = 0x3a;//0xd3;//0x80;//0x4d;
						bt5[2] = 0x2f;//0xff;//0x53;//0xb1;
						bt5[1] = 0xd6;//0xd3;//0x28;//0x32;
						if (! HidD_SetFeature(hFile,bt5,sizeof(bt5)))
							std::cerr << "error setting bluetooth information" << std::endl;
						/*After setting BT MAC address, pairing is a little bit tricky! Press PS button and run BT discovery. Pair without passkey and select HID services for establishing a permanent BT connection.
						Instructions may have to be done several times on a trial-and-error basis. Having another (dummy) BT device may be necessary for successfull pairing as well.
						Timing for pressing the PS button is crucial! Pay attention to the blinking LED and react fast!
						However, pairing and establishing the connection can be done in subsequent steps. This was successfully tested with the original Windows XP BT stack.*/
#endif
						for (int i = 1;i < sizeof(in);i ++)
							in[i] = 0;
						in[0] = 1;
#ifdef WRITE
						for (int i = 1;i < sizeof(out);i ++)
							out[i] = 0;
						out[0] = 0x02; //0x06
#ifdef RUMBLE
						out[5] = 0x00; //enable rumble
#else
						out[5] = 0x01; //disable rumble
#endif
						for (unsigned c = 0x0;c < 0x1000000;c ++)
#else
						for (;;)
#endif
						{
#ifdef WRITE
							out[2] = c & 0xFF; //R
							out[3] = (c >> 8) & 0xFF; //G
							out[4] = (c >> 16) & 0xFF; //B
							out[6] = (c >> 0/*8*/) & 0xFF; //rumble
							DWORD written;
							if (! WriteFile(hFile,out,sizeof(out),&written,&overlapped))
								if (WaitForSingleObject(overlapped.hEvent,INFINITE) != WAIT_OBJECT_0)
								{
									CancelIo(hFile);
									std::cerr << "error writing to device" << std::endl;
								}
#endif
							if (! ReadFile(hFile,in,sizeof(in),&read,&overlapped))
								if (WaitForSingleObject(overlapped.hEvent,300) != WAIT_OBJECT_0)
								{
									CancelIo(hFile);
									std::cerr << "error reading from device" << std::endl;
								}
								else
								{
									std::cout << "sequence number: " << (in[4] & 0x0F) << std::endl;
									std::cout << "buttons: " << std::hex << ((unsigned int((in[3] & 0x01) | ((in[4] & 0xF0) >> 3)) << 16) | (unsigned int(in[2]) << 8) | (unsigned int(in[1]) << 0)) << std::dec << " analog: " << unsigned short(in[6]) << " (" << unsigned short(in[5]) << ") battery: " << unsigned short(in[12]) << " temperature: " << (short(((short(in[37]) << 4) | (short((in[38] & 0xf0) >> 4))) << 4) >> 4) << std::endl;
									std::cout << "accelerometer: (" << short(((short(in[20]) << 8) | (short(in[19]) << 0)) ^ 0x8000) << '|' << short(((short(in[22]) << 8) | (short(in[21]) << 0)) ^ 0x8000) << '|' << short(((short(in[24]) << 8) | (short(in[23]) << 0)) ^ 0x8000) << ") ";
									std::cout << "((" << short(((short(in[14]) << 8) | (short(in[13]) << 0)) ^ 0x8000) << '|' << short(((short(in[16]) << 8) | (short(in[15]) << 0)) ^ 0x8000) << '|' << short(((short(in[18]) << 8) | (short(in[17]) << 0)) ^ 0x8000) << "))" << std::endl;
									std::cout << "gyroscope: (" << short(((short(in[32]) << 8) | (short(in[31]) << 0)) ^ 0x8000) << '|' << short(((short(in[34]) << 8) | (short(in[33]) << 0)) ^ 0x8000) << '|' << short(((short(in[36]) << 8) | (short(in[35]) << 0)) ^ 0x8000) << ") ";
									std::cout << "((" << short(((short(in[26]) << 8) | (short(in[25]) << 0)) ^ 0x8000) << '|' << short(((short(in[28]) << 8) | (short(in[27]) << 0)) ^ 0x8000) << '|' << short(((short(in[30]) << 8) | (short(in[29]) << 0)) ^ 0x8000) << "))" << std::endl;
									std::cout << "magnetometer: (" << (short(((short(in[38] & 0x0f) << 4) | (short(in[39]) << 0)) << 4) >> 4) << '|' << (short(((short(in[40]) << 4) | (short((in[41] & 0xf0) >> 4))) << 4) >> 4) << '|' << (short(((short(in[41] & 0x0f) << 4) | (short(in[42]) << 0)) << 4) >> 4) << ")" << std::endl;
								}
						}
					}
				}
			}
			CloseHandle(hFile);
		}
		delete pDevInterfaceDetailData;
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return 0;
}