/**
 * \brief  Demo program for the PS3 Move Tracker
 * \author Alexander Nitsch <nitscha@cs.tu-berlin.de>
 *
 * This started as student project at Technische Universitaet Berlin's
 * Communications Systems Group (http://www.nue.tu-berlin.de/) and is now part
 * of the Move on PC project (http://code.google.com/p/moveonpc/).
 *
 * This is a complete demo program that sets up the PlayStation Eye camera and
 * the PlayStation Move motion controller itself to demonstrate tracking of the
 * colored sphere and very simple determination of orientation from
 * accelerometers and magnetometers.
 *
 * Sphere color can be changed by holding down X, Circle or Triangle on the
 * controller and using the T button to control intensity. Use the min hue and
 * max hue sliders in the GUI to select the hue accordingly.
 *
 * Rename calibration.yaml.template to calibration.yaml and put in your own
 * calibration data (especially the magnetometer since this might vary heavily
 * depending on location). This will hopefully be replaced with something that
 * reads the proper calibration data from the Move (once we fully understand
 * how to interpret the data).
 */

#include <stdio.h>

#include "Ps3MoveTracker.h"


int main(int argc, char **argv) {
	// exit if no camera found
	if (CLEyeGetCameraCount() < 1) {
		printf("No PlayStation Eye camera found.\n");
		return -1;
	}
	
	Ps3MoveTracker *pcTracker = new Ps3MoveTracker("PS3 Move Tracker",
		CLEyeGetCameraUUID(0), CLEYE_VGA, 60);

	if (!pcTracker->start()) {
		printf("Failed to start the PS3 Move tracker.\n");
		delete pcTracker;
		return -2;
	}

	// wait infinitely for user to press any key
	cvWaitKey();

	if (!pcTracker->stop()) {
		printf("Failed to properly stop the PS3 Move tracker. Whatever ...\n");
	}

	delete pcTracker;

	return 0;
}
