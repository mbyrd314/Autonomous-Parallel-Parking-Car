#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
//#include <zhelpers.h>
//#include <buffer.h>

#define MSG_SIZE 12
#define ACK_SIZE 22
int main(){
    zmq_msg_t message;
    int err1 = zmq_msg_init_size(&message, MSG_SIZE);

    if(err1 == -1){
        perror("Size initalization error: ");
    }

    zmq_msg_t ack;
    int err2 = zmq_msg_init_size(&ack, ACK_SIZE);
    if(err2 == -1){
        perror("Size initalization error: ");
    }

    printf("Connecting to Motor Control server\n");

    //Create socket
    void *context = zmq_init(1);
    void *requester = zmq_socket(context, ZMQ_REQ);

    printf("Socket initalization success\n");
    //Connect to server on same host
    int err3 = zmq_connect (requester, "tcp://localhost:5555");
    if(err3 == -1){
        perror("Connection error: ");
    }

    char instruction[MSG_SIZE] = "0,30,1";
    char confirm[ACK_SIZE];
    int x = 1;

    while(x == 1){
        //message = move_motors()
        //instruction = "0,30,1";
        memcpy (zmq_msg_data (&message), instruction, MSG_SIZE);
        int err4 = zmq_send (requester,&message,0);
        if(err4 == -1){
            perror("Send error: ");
        }

        int err5 = zmq_recv (requester,&ack,0);
        if(err5 == -1){
            perror("Recv error: ");
        }

        memcpy (confirm ,zmq_msg_data (&ack), ACK_SIZE);
        printf("Confirmation: %s \n", confirm);
        x = 0;
    }



}
