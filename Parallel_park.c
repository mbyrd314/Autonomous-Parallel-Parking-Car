//Assume car is in position
//Car will move straight until open spot is detected
//Determine size of spot
//Initiate parallel parking protocol
#include "DC_Motor_Control.h"
#include "pthread.h"

int MIN_DISTANCE 6;
pthread_mutex_t mutex_spot
pthread_mutex_t mutex_peds;
pthread_mutex_t mutex_peds_back;

//Shared variables representing if there is an open
//parking spot and if there is anything blocking
//the path in the front or behind the car
static int spot;
static int peds;
static int peds_back;

//This struct will be used to pass gpio_pins struct
//and an int indicating which shared variable to change
//peds or peds back
struct dist_vars{
    struct gpio_pins IO;
    int checker;
};

int main(){
    //initialize sockets and io structs
    struct zmq_socket zsocket;
    struct dist_vars IO_forward;
    struct dist_vars IO_backward;
    struct dist_vars IO_right_f;
    struct dist_vars IO_right_b;


    //Pin initialization of ultrasonic sensors
    IO_forward->trigger = 45;
    IO_forward->echo = 44;

    IO_backward->trigger = 45;
    IO_backward->echo = 44;

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
    pthread_mutex_init(&mutex, &attr);
    pthread_mutex_init(&mutex_peds, &attr);
    pthread_mutex_init(&mutex_peds_back, &attr);
    pthread_metxattr_destroy(&attr);

    //Create thread to drive forward until open spot is found
    pthread_create(&t1, NULL, find_spot, (void *) &zsocket);
    //pthread_create(&t2, NULL, p_park, (void *) &zsocket);

    //Create threads that check the forward and side sensors
    //CORRECT WHAT THESE THREADS PASS SO THAT IT MATCHES NEW STRUCT
    pthread_create(&uf, NULL, dist_detect, (void *) &IO_forward );
    pthread_create(&us1, NULL, dist_detect, (void *) &IO_right_f );
    pthread_create(&us2, NULL, dis_detect, (void *) &IO_right_b );
    //Create thread that detects open parking spot
    pthread_create(&t2, NULL, open_spot, (void *) &IO_right_b );
    pthread_join(t2,NULL);
    int start_park = pthread_join(t1, NULL);

    if (start_park == 1){
        pthread_create(&t3, NULL, p_park, (void *) &zsocket );
        //CORRECT WHAT THIS THREAD PASSES SO IT MATCHES NEW STRUCT
        pthread_create(&ub, NULL, dist_detect, (void *) &IO_forward );
    }

    pthread_join(t3,NULL);
    pthread_join(ub,NULL);
    pthread_join(uf,NULL);
    pthread_join(us1,NULL);
    pthread_join(us2,NULL);

    stop(&zsocket);
    return 0;
}

//determine if open spot exists with which to park
int find_spot(void *p_socket)
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    int available;
    int safe;
    pthread_mutex_lock(mutex_spot);
        available = spot;
    pthread_mutex_unlock(mutex_spot);

    pthread_mutex_lock(mutex_peds);
        nsafe = peds;
    pthread_mutex_unlock(mutex_peds);

    //Available == 0 when parking spot has not yet been found
    //nsafe == 0 when there is nothing around the car
    while(available == 0 && nsafe == 0){
        //Change function so accepts number less than 1
        forward(20, 1, &zsocket);
        pthread_mutex_lock(mutex_spot);
            available = spot;
        pthread_mutex_unlock(mutex_spot);

        pthread_mutex_lock(mutex_peds);
            nsafe = peds;
        pthread_mutex_unlock(mutex_peds);
    }
    return 1;
}

//Hardcoded parallel park. Modify to dynamically park
void p_park((void *) &p_socket){
    struct zmq_socket *zsocket = (struct zmq_socket *) p_socket;
    //checks ultrasonic sensor in back
    pthread_mutex_lock(mutex_peds_back);
        nsafe_back = peds_back;
    pthread_mutex_unlock(mutex_peds_back);
    //checks ultrasonic sensor in front
    pthread_mutex_lock(mutex_peds);
        nsafe = peds;
    pthread_mutex_unlock(mutex_peds);

    while(/*INSTRUCTIONS LEFT IN LIST*/){
        if(nsafe == 1){
            stop(&zsocket);
        }
        else{
            //READ NEXT INSTRUCTION FROM THE LIST
            /*back_right(30, 2, &zsocket);
            back_left(30, 1, &zsocket);
            forward(30, 1, &zsocket);
            backward(20, 1, &zsocket);
            stop(&zsocket);
            */
        }
        pthread_mutex_lock(mutex_peds_back);
            nsafe_back = peds_back;
        pthread_mutex_unlock(mutex_peds_back);

        pthread_mutex_lock(mutex_peds);
            nsafe = peds;
        pthread_mutex_unlock(mutex_peds);
    }

    return NULL;
}

//MODIFY THIS FUNCTION SO THAT IT TAKES IN AN INT AS WELL AS *P_GPIO
//INT WILL REPRESENT WHICH GLOBAL VARIABLE TO CHANGE IE F
void dist_detect(struct dist_vars *vars){

    struct gpio_pins pointer_gpio = vars->IO;
    sensor_init(&pointer_gpio);

    while (1){
        dist = distance(pointer_gpio->gpio_output, pointer_gpio->gpio_input);
        //writes to shared variable if distance is unsafe
        if(vars->checker == 1 && dist < MIN_DISTANCE){
            pthread_mutex_lock(mutex_peds);
                peds_back = 1;
            pthread_mutex_unlock(mutex_peds);
        }
        //writes to shared variable if distance is unsafe
        else if(vars->checker == 2 && dist < MIN_DISTANCE){
            pthread_mutex_lock(mutex_peds_back);
                peds_backs = 1;
            pthread_mutex_unlock(mutex_peds_back);
        }
        //printf("Distance from sensor is %f \n" , dist);
        //usleep(10000);
}
