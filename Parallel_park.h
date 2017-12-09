#ifndef Parallel_park_H
#define Parallel_park_H

#include "DC_Motor_Control.h"
#include <pthread.h>
#include "distance.h"
#include <signal.h>

#define MIN_DISTANCE 20
<<<<<<< HEAD
#define MIN_DISTANCE_SIDES 10
=======
#define MIN_DISTANCE_SIDES 8
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#define MAX_DISTANCE 40
#define OPEN_TIME 1.5
#define PARKING_MOVES 4

//Car will drive forward until open spot is found
<<<<<<< HEAD
/**
 * Drives forward and repeatedly checks the distance sensed by both side ultrasonic sensors. If OPEN_TIME
 * seconds pass between when the front sensor detects a distance farther than MIN_DISTANCE_SIDES
 * and when the back sensor detects a distance closer than MIN_DISTANCE_SIDES, then an open spot
 * has been detected. This also stops the car if it detects an obstacle in front of it.
 * @param p_socket - A void pointer to a zmq_socket struct. This is needed to control the car.
**/
void *find_spot(void *p_socket);

/**
 * Once an open spot has been found, this will command the car to perform a set of instructions that
 * will make it parallel park. If an obstacle appears in the car's path, it will stop until the obstacle
 * is moved. It will then finish the parking instructions.
 * @param p_socket - A void pointer to a zmq_socket struct. This is needed to control the car.
**/
void *p_park(void *p_socket);

/**
 * Samples one of the four distance sensors and does a different action depending on which sensor it sampled.
 * If it was the front or back sensors, it checks to see if there is an obstacle in the car's path and changes
 * the corresponding shared variable and signals the correct conditional variable if one is detected. This prevents
 * the car from hitting obstacles. If it was the front side sensor, it checks for when a parking spot is first detected
 * and changes a shared variable if it detects an open spot for long enough. If it was the back side sensor, it
 * detects when the distance goes from greater than MIN_DISTANCE_SIDES to less than MIN_DISTANCE_SIDES and stops the
 * car when this occurs. This ensures that the car is in the correct position to parallel park.
 * @param zgpio - A void pointer to a gpio_pins object. This is needed to use the ultrasonic sensors.
**/
void *dist_detect(void *zgpio);

/**
 * Initializes a bluetooth connection to the Android phone and receives messages from it. All messages that are received
 * are sent to the Python motor control server to actually move the motors. This also changes a shared variable indicating
 * whether the car is currently moving forward or backward. This allows the correct sensors to be used to prevent collision.
 * @param p_socket - A void pointer to a zmq_socket struct. This is needed to control the car.
**/
void *User(void *p_socket);

/**
 * Overrides user control if the commanded instruction would cause the car to collide with
 * something in front of it
 * @param p_socket - This is a void pointer to a zmq_socket struct. It is needed to send 
 * instructions to the car.
**/
void *User_Override(void *p_socket){
	
/**
 * Overrides user control if the commanded instruction would cause the car to collide with
 * something behind it
 * @param p_socket - This is a void pointer to a zmq_socket struct. It is needed to send 
 * instructions to the car.
**/	
void *User_Override2(void *p_socket){

=======
void *find_spot(void *p_socket);

//Once open spot is found parking protocol will commence
void *p_park(void *p_socket);

//detects distance of objects from sensor.Depending on checker
//int changes various shared variables
void *dist_detect(void *zgpio);

//recieves user input via bluetooth and passes it
void *User(void *p_socket);

>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#endif /*Parallel_Park_H_*/
