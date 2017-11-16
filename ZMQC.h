#ifndef ZMQC_H_
#define ZMQC_H_

#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MSG_SIZE 12
#define ACK_SIZE 22
/**
    *zmq socket variables
**/
struct zmq_socket{
    zmq_msg_t message;
    zmq_msg_t ack;
    void *context;
    void *requester;
};
/**
    *initializes zmq socket
    *@param zmq_socket struct pointer
    *@return void
**/
void init_socket(struct zmq_socket *p_socket);
/**
    *send instruction message of maximum length 12 and recieve
    *confirmation from reply server
    *@param instruction string comma delimited with function num,
    *speed, time
    *@param zmq_socket struct pointer
    *@return void
**/
void send_instr(char *instruction, struct zmq_socket *p_socket);
#endif /*ZMQC_C_*/
