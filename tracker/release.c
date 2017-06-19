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
#define BURN_LIMIT 10
#define BURST_ALT 30000

void *ReleaseLoop(void *some_void_ptr)
{
	struct TGPS *GPS;
	GPS = (struct TGPS *)some_void_ptr;
	bool burnt = false;

	// This sets the GPIO pin to output mode to enable the TIP122 in the burn wire circuitry.
	pinMode (RELEASE_GPIO, OUTPUT);
	while (1) {
        // Will turn on the burn wire when the balloon reaches an altitude of 30km or if the balloon starts descending prematurely.
		// Reached release altitude?
		if (GPS->Altitude > BURST_ALT) {
			if(!burnt){
				sleep(60); // wait a minute before release;
				digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
			}
		}
		// Burst?
		else if (GPS->FlightMode >= fmBurst) {
			if(!burnt){
				digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
			}
		}
		sleep(1);
	}
	return 0;
}
