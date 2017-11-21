//Assume car is in position
//Car will move straight until open spot is detected
//Determine size of spot
//Initiate parallel parking protocol
#include "DC_Motor_Control.h"
#include <pthread.h>
#include "distance.h"

#define MIN_DISTANCE 6
#define MIN_DISTANCE_SIDES 10
#define MAX_DISTANCE 50
#define OPEN_TIME 6

pthread_mutex_t mutex_spot;
pthread_mutex_t mutex_peds;
pthread_mutex_t mutex_peds_back;

//Shared variables representing if there is an open
//parking spot and if there is anything blocking
//the path in the front or behind the car
int spot;
int peds;
int peds_back;
int open_found;

//determine if open spot exists with which to park
void *find_spot(void *p_socket){
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    int available;
    int nsafe;
    pthread_mutex_lock(&mutex_spot);
        available = spot;
    pthread_mutex_unlock(&mutex_spot);

    pthread_mutex_lock(&mutex_peds);
        nsafe = peds;
    pthread_mutex_unlock(&mutex_peds);

    //Available == 0 when parking spot has not yet been found
    //nsafe == 0 when there is nothing around the car
    while(available == 0 && nsafe == 0){
        forward(20, 1, zsocket);                //Change function so accepts number less than 1
        pthread_mutex_lock(&mutex_spot);
            available = spot;
        pthread_mutex_unlock(&mutex_spot);

        pthread_mutex_lock(&mutex_peds);
            nsafe = peds;
        pthread_mutex_unlock(&mutex_peds);
    }
    return (void *)1;
}

void *p_park(void *p_socket){           //Hardcoded parallel park. Modify to dynamically park
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    int nsafe_back;
    int nsafe;
    //checks ultrasonic sensor in back
    pthread_mutex_lock(&mutex_peds_back);
        nsafe_back = peds_back;
    pthread_mutex_unlock(&mutex_peds_back);
    //checks ultrasonic sensor in front
    pthread_mutex_lock(&mutex_peds);
        nsafe = peds;
    pthread_mutex_unlock(&mutex_peds);

    //while instructions are left in parking list buffer
    while(1){
        if(nsafe == 1){
            stop(zsocket);
        }
        //else{
            //READ NEXT INSTRUCTION FROM THE LIST
            /*
            back_right(30, 2, &zsocket);
            back_left(30, 1, &zsocket);
            forward(30, 1, &zsocket);
            backward(20, 1, &zsocket);
            stop(&zsocket);
            */
    //    }

        pthread_mutex_lock(&mutex_peds_back);
            nsafe_back = peds_back;
        pthread_mutex_unlock(&mutex_peds_back);

        pthread_mutex_lock(&mutex_peds);
            nsafe = peds;
        pthread_mutex_unlock(&mutex_peds);
    }
    return NULL;
}

//MODIFY THIS FUNCTION SO THAT IT TAKES IN AN INT AS WELL AS *P_GPIO
//INT WILL REPRESENT WHICH GLOBAL VARIABLE TO CHANGE IF
void *dist_detect(void *zgpio){
    double dist;
    struct gpio_pins *p_gpio = (struct gpio_pins *) zgpio;
    double prev_state;
    double time_elapsed;
    double time_start;
    double time_stop;

    sensor_init(p_gpio);

    while(1){
        dist = distance(p_gpio);
        //writes to shared variable if distance is unsafe
        if((p_gpio->checker == 1) && (dist < MIN_DISTANCE)){
            pthread_mutex_lock(&mutex_peds);
                peds = 1;
            pthread_mutex_unlock(&mutex_peds);
        }
        //writes to shared variable if distance is unsafe
        else if((p_gpio->checker == 2) && (dist < MIN_DISTANCE)){
            pthread_mutex_lock(&mutex_peds_back);
                peds_back = 1;
            pthread_mutex_unlock(&mutex_peds_back);
        }
        //writes to shared variable spot if distance has been greater than max distance for
        //specified length of time
        //Corresponds to frontright ultrasonic sensor
        else if (p_gpio->checker == 3 && dist > MAX_DISTANCE){
            if(dist > MAX_DISTANCE){
                //start timer here
            }
            else{
                //stop timer
            }
            time_elapsed = time_start - time_stop;
            if(time_elapsed > OPEN_TIME){
                open_found = 1;
            }

        }
        //Causes car to stop when distance changes from greater than Max Distance
        //to less than max distance
        //Corresponds to backright ultrasonic sensor.
        else if(p_gpio->checker == 4){
            if (prev_state > MAX_DISTANCE && dist < MIN_DISTANCE_SIDES && open_found == 1){
                spot = 1;
            }
            prev_state = dist;
        }
    }
    return NULL;
}

int main(){
    //initialize sockets and io structs
    struct zmq_socket zsocket;
    struct gpio_pins *IO_forward;
    struct gpio_pins *IO_backward;
    struct gpio_pins *IO_right_f;
    struct gpio_pins *IO_right_b;
    void *start_park;


    //Pin initialization of ultrasonic sensors
    IO_forward->trigger = 45;
    IO_forward->echo = 44;
    IO_forward->checker = 1;

    IO_backward->trigger = 45;
    IO_backward->echo = 44;
    IO_backward->checker = 1;

    IO_right_f->trigger = 45;
    IO_right_f->echo = 44;

    IO_right_b->trigger = 45;
    IO_right_b->echo = 44;

    //initialize socket
    init_socket(&zsocket);

    //create threads
    pthread_t t1, uf, ub, us1, us2, t2, t3;

    //Create mutexes
    pthread_mutexattr_t attr;
    pthread_mutex_init(&mutex_spot, &attr);
    pthread_mutex_init(&mutex_peds, &attr);
    pthread_mutex_init(&mutex_peds_back, &attr);
    pthread_mutexattr_destroy(&attr);

    //Create thread to drive forward until open spot is found
    pthread_create(&t1, NULL, find_spot, (void *) &zsocket);
    //pthread_create(&t2, NULL, p_park, (void *) &zsocket);

    //Create threads that check the forward and side sensors
    pthread_create(&uf, NULL, dist_detect, (void *) IO_forward);
    pthread_create(&us1, NULL, dist_detect, (void *) IO_right_f);
    pthread_create(&us2, NULL, dist_detect, (void *) IO_right_b);

    //Create thread that detects open parking spot
    //thread_create(&t2, NULL, open_spot, (void *) &IO_right_b );
    //pthread_join(t2,NULL);

    pthread_join(t1, &start_park);

    if ((int)start_park == 1){
        pthread_create(&t3, NULL, p_park, (void *) &zsocket);
        pthread_create(&ub, NULL, dist_detect, (void *) IO_backward);
    }

    pthread_join(t3,NULL);
    pthread_join(ub,NULL);
    pthread_join(uf,NULL);
    pthread_join(us1,NULL);
    pthread_join(us2,NULL);

    stop(&zsocket);
    return 0;
}
