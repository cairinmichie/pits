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
#define VIDEO 5
#define READY 22

void *CameraLoop(void *some_void_ptr)
{
	struct TGPS *GPS;
	GPS = (struct TGPS *)some_void_ptr;
	bool burnt = false;

    // Code for sending a command to camera.
	char PhotoCommand[1000];
    sprintf( PhotoCommand, "raspistill -st -w 2592 -h 1944 -t 3000 -ex auto -mm matrix -o /home/pi/pits-camera/camera/photos/%u.jpg &disown", (unsigned)time(NULL));

	pinMode (READY, INPUT);
	pinMode (PHOTO, OUTPUT);
	pinMode (VIDEO, OUTPUT);

	// TODO: Use ISR
	while (1) {
		// Capsule ready for release?
		if(digitalRead(READY)) {
			if(!burnt){
				system(VideoCommand);
				digitalWrite(VIDEO, 1); // handled as an interrupt.
				sleep(1);
				digitalWrite(VIDEO, 0);
				burnt = true;
				sleep(184); // Ensure 3 minutes of Video
			}
		}
		system(PhotoCommand);
		digitalWrite(PHOTO, 1); // handled as an interrupt - can immediately set to zero again.
		sleep(1);
		digitalWrite(PHOTO, 0);
		sleep(60); // Take photo every minute.
	}

	return 0;
}
