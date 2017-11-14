#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//#include <zhelpers.h>
//#include <buffer.h>

#define MSG_SIZE 12
#define ACK_SIZE 23
int main(){
    zmq_msg_t message;
    zmq_msg_init_size(&message, MSG_SIZE);

    zmq_msg_t ack;
    zmq_msg_init_size(&ack, ACK_SIZE);

    //char ack[ACK_SIZE];
    printf("Connecting to Motor Control server\n");

    //Create socket
    void *context = zmq_init (0);
    void *requester = zmq_socket (context, ZMQ_REQ);

    printf("Created Socket\n");
    //Connect to server on same host
    zmq_connect (requester, "tcp://localhost:5555");
    char instruction[MSG_SIZE] = "0,30,1";
    char confirm[ACK_SIZE];
    int x = 1;

    printf("Begin while loop\n");
    while(x == 1){
        //message = move_motors()
        //instruction = "0,30,1";
        memcpy (zmq_msg_data (&message), instruction, MSG_SIZE);
        zmq_send (&message, requester,0);
        zmq_recv (&ack, requester,0);

        memcpy (confirm ,zmq_msg_data (&ack), ACK_SIZE);
        printf("Confirmation: %s", confirm);
        x = 0;
    }



}
