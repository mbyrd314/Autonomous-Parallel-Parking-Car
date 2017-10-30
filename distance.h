//HeaderFile
#include "libsoc_GPIO.h"
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

/**
Calulate distance to object in front of sensor
@param integer representing pin used by BBB for trigger
@param integer representing pin used by BBB for echo
@return Returns distance as a float
**/
double distance(int trigger, int echo);

/**Sleeps for user specified number of nanoseconds.
@param integer number representing nanoseconds
@return no output
**/
void wait(int nanosec);
