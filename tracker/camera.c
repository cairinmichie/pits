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

#define BURST_ALT 30000
#define PHOTO 4
#define VIDEO 4

void *CameraLoop(void *some_void_ptr)
{
	struct TGPS *GPS;
	GPS = (struct TGPS *)some_void_ptr;
	bool burnt = false;

      int n, a=5, b=3;
      n=sprintf (buffer, "%d plus %d is %d", a, b, a+b);

    // Code for sending a command to camera.
    char PhotoCommand[50];
    fprintf(fp, "raspistill -st -w 2592 -h 1944 -t 3000 -ex auto -mm matrix %s -o %s\n", width, height, Config.CameraSettings, FileName);
    sprintf( PhotoCommand, "raspistill -st -w 2592 -h 1944 -t 3000 -ex auto -mm matrix -o %s.jpg", FileName);
    char VideoCommand[50];
    sprintf( VideoCommand, "raspivid -t 180000 -w 1280 -h 720 -fps 60 -o pivideo.h264 & disown");

	// This sets the GPIO pin to output mode to enable the TIP122 in the burn wire circuitry.
	pinMode (RELEASE_GPIO, OUTPUT);
	pinMode (PHOTO, OUTPUT);
	pinMode (VIDEO, OUTPUT);

	while (1) {
		// Reached release altitude?
		if (GPS->Altitude > BURST_ALT) {
			if(!burnt){
				system(VideoCommand);
				digitalWrite(VIDEO, 1); // handled as an interrupt - can immediately set to zero again.
				digitalWrite(VIDEO, 0);
				burnt = true;
			}
		}
		// Burst? Takes GPS and pressure into account.
		else if (GPS->FlightMode >= fmBurst) {
			if(!burnt){
				system(VideoCommand);
				digitalWrite(VIDEO, 1); // handled as an interrupt - can immediately set to zero again.
				digitalWrite(VIDEO, 0);
				burnt = true;
			}
		}
        else {

            sleep(1);
        }

	}

	return 0;
}
