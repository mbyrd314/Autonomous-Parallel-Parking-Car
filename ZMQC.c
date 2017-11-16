#include "ZMQC.h"
//TO DO: make instruction an input to this function

/**
    *initializes zmq socket
**/
void init_socket(struct zmq_socket *p_socket){

    printf("Connecting to Motor Control server\n");

    //Create socket
    p_socket->context = zmq_init(1);
    p_socket->requester = zmq_socket(p_socket->context, ZMQ_REQ);

    printf("Socket initalization success\n");
    //Connect to server on same host
    int err3 = zmq_connect(p_socket->requester, "tcp://localhost:5555");
    if(err3 == -1){
        perror("Connection error: ");
    }

}
/**
    *send instruction message of maximum length 12 and recieve
    *confirmation from reply server
**/
void send_instr(char *instruction, struct zmq_socket *p_socket){

    zmq_msg_init_size(&(p_socket->message), MSG_SIZE);
    zmq_msg_init_size(&(p_socket->ack), ACK_SIZE);

    char confirm[ACK_SIZE];
    memcpy(zmq_msg_data(&(p_socket->message)), instruction, MSG_SIZE);
    int err4 = zmq_send ((p_socket->requester),&(p_socket->message),0);
    if(err4 == -1){
        perror("Send error: ");
    }

    int err5 = zmq_recv ((p_socket->requester),&(p_socket->ack),0);
    if(err5 == -1){
        perror("Recv error: ");
    }

    memcpy (confirm ,zmq_msg_data (&(p_socket->ack)), ACK_SIZE);
    printf("Confirmation: %s\n", confirm);
}
