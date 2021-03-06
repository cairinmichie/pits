#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>   	// Standard input/output definitions
#include <string.h>  	// String function definitions
#include <unistd.h>  	// UNIX standard function definitions
#include <fcntl.h>   	// File control definitions
#include <errno.h>   	// Error number definitions
#include <termios.h> 	// POSIX terminal control definitions
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <pthread.h>
#include <wiringPi.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <sys/statvfs.h>
#include <pigpio.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>

#include "gps.h"
#include "misc.h"

#define RELEASE_GPIO 6 // Choose an appropriate GPIO pin - Needs to be off by default so that the burn wire doesn't activate prematurely.
#define READY 22 // set hi to allow flight computer to know if capsule is ready for release.
#define RELEASE 23 // in case of emergency release.
#define DEAD 21 // If NOGO response is received.
#define BURN_LIMIT 10
#define BURST_ALT 25000
#define MIN_ALT 15000
#define RELEASE_GPIO_2 0

void *ReleaseLoop(void *some_void_ptr)
{
	struct TGPS *GPS;
	GPS = (struct TGPS *)some_void_ptr;
	bool burnt = false;
	GPS->Health = 'H';
	GPS->Burn = 'N';
	unsigned int releaseTime = 0;

	// This sets the GPIO pin to output mode to enable the TIP122 in the burn wire circuitry.
	pinMode (RELEASE_GPIO, OUTPUT);
	pinMode (READY, INPUT);
	pinMode (DEAD, INPUT);
	pinMode (RELEASE, OUTPUT);
	pinMode (RELEASE_GPIO_2, OUTPUT);

	digitalWrite(RELEASE, 0);
	digitalWrite(RELEASE_GPIO, 0);
	digitalWrite(RELEASE_GPIO_2, 0);

	while (1) {
        // Will turn on the burn wire when the balloon reaches an altitude of 30km or if the balloon starts descending prematurely. Checks the balloon is above the useful altitude limit.
		if (GPS->Altitude > BURST_ALT && GPS->Altitude > MIN_ALT) {
			if(!burnt && GPS->Health == 'H'){
				digitalWrite(RELEASE, 1);
				GPS->Burn = 'Y';
				sleep(60);
				digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
				digitalWrite(RELEASE, 0);
				releaseTime = (unsigned)time(NULL);
			}
		}
		// Burst?
		else if (GPS->FlightMode == fmBurst && GPS->Altitude > MIN_ALT) {
			if(!burnt && GPS->Health == 'H'){
				digitalWrite(RELEASE, 1);
				GPS->Burn = 'Y';
				sleep(60);
				digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
				digitalWrite(RELEASE, 0);
				releaseTime = (unsigned)time(NULL);
			}
		}
		// Passing out of flight zone so must release.
		else if (GPS->Longitude >= 20.0 && GPS->Altitude > MIN_ALT) {
			if(!burnt && GPS->Health == 'H'){
				digitalWrite(RELEASE, 1);
				GPS->Burn = 'Y';
				sleep(60);
				digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
				digitalWrite(RELEASE, 0);
				releaseTime = (unsigned)time(NULL);
			}
		}

		// Release payload 5 minutes after capsule.
		if((releaseTime + 300 <= (unsigned)time(NULL)) && burnt) {
			digitalWrite(RELEASE_GPIO_2, 1);
			sleep(BURN_LIMIT);
			sleep(BURN_LIMIT);
			digitalWrite(RELEASE_GPIO_2, 0);
		}


// TEST BLOCK STARTS
		if(digitalRead(READY) == 0) {
			digitalWrite(RELEASE, 1);
			GPS->Burn = 'Y';
			digitalWrite(RELEASE_GPIO, 1);
			sleep(BURN_LIMIT);
			digitalWrite(RELEASE_GPIO, 0);
			burnt = true;
            digitalWrite(RELEASE, 0);
            releaseTime = (unsigned)time(NULL);
		}
// TEST BLOCK ENDS

		if(digitalRead(DEAD) == 1) {
			GPS->Health = 'D';
		}
		else if(digitalRead(DEAD) == 0) {
			GPS->Health = 'H';
		}
		sleep(1);
	}
	return 0;
}
