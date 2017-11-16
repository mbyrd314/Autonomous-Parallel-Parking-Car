//Assume car is in position
//Car will move straight until open spot is detected
//Determine size of spot
//Initiate parallel parking protocol
#include "DC_Motor_Control.h"
#include "pthread.h"

pthread_mutex_t mutex_spot
pthread_mutex_t mutex_peds;
void main(){
    struct zmq_socket zsocket;
    struct gpio_pins IO;

    init_socket(&zsocket);
    pthread_t t1, t2, t3;

    pthread_mutexattr_t attr;
    pthread_mutex_init(&mutex, &attr);
    pthread_metxattr_destroy(&mymutexattr);

    pthread_create(&t1, NULL, find_spot, (void *) &zsocket);
    pthread_create(&t2, NULL, p_park, (void *) &zsocket);
    pthread_create(&t3, NULL, distance, (void *) &IO );
}

void find_spot(void *p_socket)
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    pthread_mutex_lock(mutex_spot);
        open_spot = spot;
    pthread_mutex_unlock(mutex_spot);

    pthread_mutex_lock(mutex_peds);
        walking_peds = peds;
    pthread_mutex_unlock(mutex_peds);

    while(open_spot == 0 && walking_peds == 0){
        forward(20, 1, &zsocket);
    }
}

//Hardcoded parallel park. Modify to dynamically park
void p_park((void *) &p_socket){
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    back_right(30, 2, &zsocket);
    back_left(30, 1, &zsocket);
    forward(30, 1, &zsocket);
    backward(20, 1, &zsocket);
    stop(&zsocket);
}
