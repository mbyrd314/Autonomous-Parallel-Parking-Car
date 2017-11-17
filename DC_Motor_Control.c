#include "DC_Motor_Control.h"


void forward(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"0,%d,%d", speed, t);
    send_instr(message,  p_socket);
}

void backward(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"1,%d,%d", speed, t);
    send_instr(message,  p_socket);
}
void left(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"2,%d,%d", speed, t);
    send_instr(message,  p_socket);
}

void back_left(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"3,%d,%d", speed, t);
    send_instr(message, p_socket);
}

void right(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"4,%d,%d", speed, t);
    send_instr(message, p_socket);
}

void back_right(int speed, int t, struct zmq_socket *p_socket){
    char message[MSG_SIZE];
    sprintf(message,"5,%d,%d", speed, t);
    send_instr(message, p_socket);
}

void stop(struct zmq_socket *p_socket){
    //int speed = 0;
    //int t = 0;
    char message[MSG_SIZE];
    sprintf(message,"6,0,0");
    send_instr(message, p_socket);
}

/*int main(){
    struct zmq_socket zsocket;
    init_socket(&zsocket);
    forward(20,1,&zsocket);
    stop(&zsocket);
    stop(&zsocket);
    stop(&zsocket);
    stop(&zsocket);
    stop(&zsocket);
    backward(20,1,&zsocket);
    back_right(20,1,&zsocket);
    right(20,1,&zsocket);
    left(20,1,&zsocket);
    back_left(20,1,&zsocket);
}*/
