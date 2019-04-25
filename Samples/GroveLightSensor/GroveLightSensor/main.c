#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>
#include "mt3620_rdb.h"
#include "Grove.h"
#include "Sensors/GroveLightSensor.h"
#include "Sensors/GroveOledDisplay96x96.h"
#include "Sensors/GroveAD7992.h"
#include "HAL/GroveI2C.h"
#include <stdio.h>

// This C application for the MT3620 Reference Development Board (Azure Sphere)
// outputs a string every second to Visual Studio's Device Output window
//
// It uses the API for the following Azure Sphere application libraries:
// - log (messages shown in Visual Studio's Device Output window during debugging)

static volatile sig_atomic_t terminationRequested = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async signal safe
    terminationRequested = true;
}

void DisplayReading(float reading, char *prefix, int sleep)
{
	char buf[50];

	sprintf(buf, "%f", reading);

	char result[100];
	strcpy(result, prefix);
	strcat(result, buf);

	putString(result);

	usleep(sleep);
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("Application starting\n");

    // Register a SIGTERM handler for termination requests
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

	// Initialize Grove Shield
	int i2cFd;
	GroveShield_Initialize(&i2cFd, 230400);
	
	// INIT OLED	
	GroveOledDisplay_Init(i2cFd, SH1107G);

	// Initialize Light Sensor
	/*void *light = GroveLightSensor_Init(i2cFd, 0);*/

	// Word display
	clearDisplay();
	setNormalDisplay();
	setVerticalMode();

    // Main loop
    const struct timespec sleepTime = {1, 0};
    while (!terminationRequested) {
		/*float value = GroveLightSensor_Read(light);
		value = GroveAD7992_ConvertToMillisVolt(value);*/
		DisplayReading(12.2, "Light value:", 100000);
        //Log_Debug("Light value %dmV\n", (uint16_t)value);

        nanosleep(&sleepTime, NULL);
    }

    Log_Debug("Application exiting\n");
    return 0;
}
