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
#include "Sensors/GroveRelay.h"
#include "Sensors/GroveOledDisplay96x96.h"
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

void DisplayReading(char* prefix, int sleep)
{
	putString(prefix);

	usleep(sleep);
}


/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    static bool state = true;

    Log_Debug("Application starting\n");

    // Register a SIGTERM handler for termination requests
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    void *relaysensor = GroveRelay_Open(0);

	clearDisplay();
	setNormalDisplay();
	setVerticalMode();

    // Main loop
    const struct timespec sleepTime = {2, 0};
    while (!terminationRequested) {   
    	
        if(state) {
            GroveRelay_On(relaysensor);
			DisplayReading("Relay on\n", 100000);
			Log_Debug("Relay on\n");
            
        } else {
            GroveRelay_Off(relaysensor);
			DisplayReading("Relay off\n", 100000);
			Log_Debug("Relay off\n");
        }

		state = !state;
        
        nanosleep(&sleepTime, NULL);
    }

    Log_Debug("Application exiting\n");
    return 0;
}
