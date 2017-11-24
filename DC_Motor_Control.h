#ifndef DC_Motor_Control_H_
#define DC_Motor_Control_H_

#include "ZMQC.h"
/**
    * Moves car forward for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *forward(int speed, double t, struct zmq_socket *p_socket);

/**
    * Moves car backward for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *backward(int speed, int t, struct zmq_socket *p_socket);
/**
    * Moves car left for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *left(int speed, int t, struct zmq_socket *p_socket);
/**
    * Moves car back and left for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *back_left(int speed, int t, struct zmq_socket *p_socket);
/**
    * Moves car right for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *right(int speed, int t, struct zmq_socket *p_socket);
/**
    * Moves car back and right for specified amount of time with set speed
    *@param duty cycle of motors to control speed must be between 0-100
    *@param zmq_socket struct pointer
**/
void *back_right(int speed, int t, struct zmq_socket *p_socket);
/**
    *Stops car
    *@param zmq_socket struct pointer
**/
void *stop(struct zmq_socket *p_socket);

#endif /*DC_Motor_Control_H_*/
