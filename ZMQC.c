#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <buffer.h>

#define MSG_SIZE 12
#define ACK_SIZE 23
int main(){
    char message[MSG_SIZE];
    char ack[ACK_SIZE];
    printf("Connecting to Motor Control server");

    //Create socket
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);

    //Connect to server on same host
    zmq_connect (requester, "tcp://localhost:5555");

    while(1){
        message = move_motors()
        zmq_send (requester, message, MSG_SIZE, 0);			
        zmq_recv (requester, ack, ACK_SIZE, 0);			/* Receive message of 255 bytes */
    }



}
