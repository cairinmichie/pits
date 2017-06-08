/*
    Release Code for the MIRKA2
    Author: Cairin Michie
*/

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

#define RELEASE_GPIO 24 // Choose an appropriate GPIO pin - Needs to be off by default so that the burn wire doesn't activate prematurely.
#define BURN_LIMIT 10
#define BURST_ALT 30000
#define TEST 25

void *ReleaseLoop(void *some_void_ptr)
{
	struct TGPS *GPS;
	GPS = (struct TGPS *)some_void_ptr;
	bool burnt = false;

    // Code for sending a command to camera.
    char command[50];
    strcpy( command, "raspivid -t 30000 -w 1280 -h 720 -fps 60 -o pivideo.h264 & disown" ); // & necessary otherwise system will wait for command to complete.

	// This sets the GPIO pin to output mode to enable the TIP122 in the burn wire circuitry.
	// pinMode (RELEASE_GPIO, OUTPUT);
	// pinMode (TEST, OUTPUT);
	while (1) {
        // Will turn on the burn wire when the balloon reaches an altitude of 30km or if the balloon starts descending prematurely.
		// Reached release altitude?
		if (GPS->Altitude > BURST_ALT) {
			if(!burnt){
				system(command);
				// digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				// digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
			}
		}
		// Burst?
		else if (GPS->FlightMode >= fmBurst) {
			if(!burnt){
				system(command); // Signal is returned causing sleep to end prematurely. try disown otherwise http://man7.org/linux/man-pages/man2/nanosleep.2.html
				// digitalWrite(RELEASE_GPIO, 1);
				sleep(BURN_LIMIT);
				// digitalWrite(RELEASE_GPIO, 0);
				burnt = true;
			}
		}
		// Test camera command.
		if(!burnt){
			// digitalWrite(TEST, 1);
			// system(command);
			sleep(BURN_LIMIT);
			// digitalWrite(TEST, 0);
			burnt = true;
		}
		sleep(1);
	}

	return 0;
}
