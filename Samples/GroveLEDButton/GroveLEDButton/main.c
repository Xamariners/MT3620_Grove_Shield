﻿#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>

#include "mt3620_rdb.h"

#include "Grove.h"
#include "Sensors/GroveLEDButton.h"

#include "Sensors/GroveOledDisplay96x96.h"


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

void DisplayReading(char* text, int sleep)
{
	setTextXY(0, 0);
	putString("                       ");
	setTextXY(0, 0);
	putString(text);
	usleep(sleep);
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
	static GPIO_Value_Type btn_sta, last_btn_sta;
    Log_Debug("Application starting\n");

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

	void *btn = GroveLEDButton_Init(1, 0);
	last_btn_sta = GroveLEDButton_GetBtnState(btn);

	// Word display
	clearDisplay();
	setNormalDisplay();
	setVerticalMode();

    // Main loop
    const struct timespec sleepTime = {0, 1000};

	DisplayReading("BUTTON READY", 1);
    while (!terminationRequested) {

		btn_sta = GroveLEDButton_GetBtnState(btn);
		
		if (btn_sta != last_btn_sta) {
			if (btn_sta == 0) {
				GroveLEDButton_LedOn(btn);
				Log_Debug("Button pressed.\n");
				DisplayReading("pressed ", 100);
			}
			else {
				GroveLEDButton_LedOff(btn);
				Log_Debug("Button released.\n");
				DisplayReading("released ", 100);
			}
		}
		last_btn_sta = btn_sta;

        nanosleep(&sleepTime, NULL);
    }

    Log_Debug("Application exiting\n");
    return 0;
}
