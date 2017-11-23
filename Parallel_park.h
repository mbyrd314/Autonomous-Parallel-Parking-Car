#ifndef Parallel_park_H
#define Parallel_park_H

#include "DC_Motor_Control.h"
#include <pthread.h>
#include "distance.h"

#define MIN_DISTANCE 20
#define MIN_DISTANCE_SIDES 10
#define MAX_DISTANCE 50
#define OPEN_TIME 6

//Car will drive forward until open spot is found
void *find_spot(void *p_socket);

//Once open spot is found parking protocol will commence
void *p_park(void *p_socket);

//detects distance of objects from sensor.Depending on checker
//int changes various shared variables
void *dist_detect(void *zgpio);
#endif /*Parallel_Park_H_*/
