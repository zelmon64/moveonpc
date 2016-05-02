/**
 * \brief  Interactively light up multiple PlayStation Move controllers and monitor their sensor data
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * Every controller gets its own thread to exchange Input and Output reports
 * with the operating system. Press Ctrl+C to exit the program. Press and hold
 * X, Triangle or Circle button to select an RGB channel and use the T button
 * to select intensity for this channel. Sensor data will be displayed for each
 * connected controller -- it is updated every 100 ms.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "moveonpc.h"


typedef struct _controllerThread {
	bool   running;
	int    index;
	HANDLE threadHandle;
	HANDLE timerHandle;
} controllerThread_t;


// global variables

bool g_exitRequested = false;
CRITICAL_SECTION g_criticalSection;
TMove g_moveData[MAXMOVES];


// function forward declarations

void signalHandler(int);
int createControllerThread(controllerThread_t *);
int endControllerThread(controllerThread_t *);
DWORD WINAPI controllerThreadProc(void *);




void signalHandler(int sig) {
	UNREFERENCED_PARAMETER(sig);
	g_exitRequested = true;
}


int createControllerThread(controllerThread_t *pThread) {
	// create periodic 10ms-timer to generate clock ticks for the thread
	// (needed to prevent flooding the controller with thousands of requests
	// per second)
	pThread->timerHandle = CreateWaitableTimer(NULL, FALSE, NULL);
	if (pThread->timerHandle == NULL) {
		printf("Failed to create timer for controller thread %d (error %d).\n",
			pThread->index, GetLastError());
		return -1;
	}
	LARGE_INTEGER liDueTime; liDueTime.QuadPart = 0;
	if (!SetWaitableTimer(pThread->timerHandle, &liDueTime, 10, NULL, NULL, FALSE)) {
		printf("Failed to set timer for controller thread %d (error %d).\n", 
			pThread->index, GetLastError());
		return -1;
	}

	// create thread that will exchange data with the Move controller
	pThread->running = true;
	pThread->threadHandle = CreateThread(NULL, 0, controllerThreadProc, pThread, 0, NULL);
	if (pThread->threadHandle == NULL) {
		printf("Failed to create controller thread %d.\n", pThread->index);
		return -1;
	}

	return 0;
}


int endControllerThread(controllerThread_t *pThread) {
	if (pThread->running) {
		pThread->running = false;

		// wait for thread to exit, timeout after 2 seconds
		WaitForSingleObject(pThread->threadHandle, 2000);
		CloseHandle(pThread->threadHandle);
	}

	return 0;
}


DWORD WINAPI controllerThreadProc(void *arg) {
	TMove moveData;
	struct { unsigned int r, g, b; } moveColor = { 0, 0, 0 };
	controllerThread_t *pThread = (controllerThread_t *)arg;

	while (pThread->running) {
		// read sensor data from controller
		ReadMove(pThread->index, &moveData);

		// set Move's color depending on the buttons pressed
		if (moveData.Buttons) {
			if (moveData.Buttons & B_CIRCLE) {
				moveColor.r = moveData.TAnalog;
			}
			if (moveData.Buttons & B_TRIANGLE) {
				moveColor.g = moveData.TAnalog;
			}
			if (moveData.Buttons & B_CROSS) {
				moveColor.b = moveData.TAnalog;
			}

			// ... some button could be used to exit this thread ...
		}
		SetMoveColour(pThread->index, moveColor.r, moveColor.g, moveColor.b);

		EnterCriticalSection(&g_criticalSection);
		memcpy(&(g_moveData[pThread->index]), &moveData, sizeof(TMove));
		LeaveCriticalSection(&g_criticalSection);

		// wait for next clock tick, timeout after 1 second
		WaitForSingleObject(pThread->timerHandle, 1000);

		// TODO: Report timeouts or other problems with this thread's clock.
	}

	// switch off the LEDs
	SetMoveColour(pThread->index, 0, 0, 0);

	return 0;
}


void gotoxy(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


void printOutputValues(TMove *pData, int index, int x, int y) {
	gotoxy(x, y);
	printf("--------- Move %d ---------", index);

	gotoxy(x, y+2);
	printf("acc:  %+6d %+6d %+6d\n", pData->RawForceX, pData->RawForceY, pData->RawForceZ);
	gotoxy(x, y+3);
	printf("mag:  %+6d %+6d %+6d\n", pData->RawMagnetX, pData->RawMagnetY, pData->RawMagnetZ);
	gotoxy(x, y+4);
	printf("gyr:  %+6d %+6d %+6d\n", pData->RawGyroPitch, pData->RawGyroYaw, pData->RawGyroRoll);

	gotoxy(x, y+6);
	printf("temp: %+6d", pData->RawTemperature);
	gotoxy(x, y+7);
	if (pData->RawBattery == 0xEE) {
		printf("bat:   (USB)");
	} else {
		printf("bat:  %6d", pData->RawBattery); 
	}
	gotoxy(x, y+8);
	printf("seq:  %6d", pData->SeqNumber);
	gotoxy(x, y+9);
	printf("time: %6d", pData->Timestamp);
}


int main() {
	controllerThread_t controllerThreads[MAXMOVES];
	int numMoves = 0;

	if (!OpenMoves() || ((numMoves = GetMoveCount()) < 1)) {
		printf("\nFailed to open Move controller(s).\n");
		return -1;
	}

	// print number of controllers we have found
	printf("\n");
	if (numMoves == 1) {
		printf("Found 1 Move controller.\n");
	} else {
		printf("Found %d Move controllers.\n", numMoves);
	}

	// register signal handler for SIGINT (Ctrl+C)
	signal(SIGINT, signalHandler);

	InitializeCriticalSection(&g_criticalSection);

	for (int i = 0; i < numMoves; i++) {
		controllerThreads[i].index = i;
		createControllerThread(&(controllerThreads[i]));
	}

	// wait 2 seconds before switching to sensor output
	Sleep(2000);
	system("cls"); // clear screen

	// program main loop	
	while (!g_exitRequested) {
		EnterCriticalSection(&g_criticalSection);
		for (int i = 0; i < numMoves; i++) {
			printOutputValues(&(g_moveData[i]), i, (i%2)*32, (i>1)*13);
		}
		LeaveCriticalSection(&g_criticalSection);
		Sleep(100);
	}

	for (int i = 0; i < numMoves; i++) {
		endControllerThread(&(controllerThreads[i]));
	}

	DeleteCriticalSection(&g_criticalSection);

	printf("Closing Move controller(s).\n");
	CloseMoves();

	return 0;
}
