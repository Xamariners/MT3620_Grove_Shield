#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>

#include "mt3620_rdb.h"

#include "Grove.h"
#include "Sensors/GroveRotaryAngleSensor.h"
#include "Sensors/GroveOledDisplay96x96.h"

#include <unistd.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include "bobo.h"

#include <applibs/gpio.h>


// This C application for the MT3620 Reference Development Board (Azure Sphere)
// outputs a string every second to Visual Studio's Device Output window
//
// It uses the API for the following Azure Sphere application libraries:
// - log (messages shown in Visual Studio's Device Output window during debugging)


// File descriptors - initialized to invalid value
static int blinkingLedGpioFd = -1;


// Button state variables

static GPIO_Value_Type ledState = GPIO_Value_High;

static volatile sig_atomic_t terminationRequested = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async signal safe
    terminationRequested = true;
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static int InitPeripheralsAndHandlers(void)
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

	// Open LED GPIO, set as output with value GPIO_Value_High (off), and set up a timer to poll it
	Log_Debug("Opening MT3620_RDB_LED1_RED.\n");
	blinkingLedGpioFd = GPIO_OpenAsOutput(MT3620_RDB_LED1_RED, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (blinkingLedGpioFd < 0) {
		Log_Debug("ERROR: Could not open LED GPIO");
		return -1;
	}

	return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
	// Leave the LED off
	if (blinkingLedGpioFd >= 0) {
		GPIO_SetValue(blinkingLedGpioFd, GPIO_Value_High);
	}

	Log_Debug("Closing file descriptors.\n");
}


void DisplayReading(float reading, char* prefix, int sleep)
{
	setTextXY(0, 0);
	putString("                       ");

	char buf[50];

	sprintf(buf, "%f", reading);

	char result[100];
	strcpy(result, prefix);
	strcat(result, buf);

	putString(result);

	//usleep(sleep);
}


/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("Application starting\n");

	if (InitPeripheralsAndHandlers() != 0) {
		terminationRequested = true;
	}

    // Register a SIGTERM handler for termination requests
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

	// Initialize Grove Shield
	int i2cFd;
	GroveShield_Initialize(&i2cFd, 9600);

	// INIT OLED	
	GroveOledDisplay_Init(i2cFd, SH1107G);

	// INIT ROTARY
	void* rotarysensor = GroveRotaryAngleSensor_Init(i2cFd, 0);

	// Word display
	/*clearDisplay();
	setNormalDisplay();
	setVerticalMode();
*/
	//DisplayReading(0, "ROTARY READY - ", 1);

    while (!terminationRequested) {
		float occupy = 1.0f - GroveRotaryAngleSensor_Read(rotarysensor);

		int angle = 1 + ((int)(occupy*10)) * 10000;

		ledState = (ledState == GPIO_Value_Low ? GPIO_Value_High : GPIO_Value_Low);

		int result = GPIO_SetValue(blinkingLedGpioFd, ledState);

		usleep(angle);

		//DisplayReading(occupy, "angle: ", 1);
    }

	ClosePeripheralsAndHandlers();

    Log_Debug("Application exiting\n");
    return 0;
}
