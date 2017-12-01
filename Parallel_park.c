//Assume car is in position
//Car will move straight until open spot is detected
//Determine size of spot
//Initiate parallel parking protocol
#include "Parallel_park.h"

pthread_mutex_t mutex_spot;
pthread_mutex_t mutex_peds;
pthread_mutex_t mutex_peds_back;
pthread_mutex_t mutex_open_found;
pthread_mutex_t mutex_distance;

pthread_cond_t condvar;
pthread_cond_t condvar_peds_back;
pthread_cond_t condvar_spot;
pthread_cond_t condvar_open_found;

//Shared variables representing if there is an open
//parking spot and if there is anything blocking
//the path in the front or behind the car
int spot = 0;
int peds = 0;
int peds_back = 0;
int open_found = 0;


//determine if open spot exists with which to park
void *find_spot(void *p_socket){
    printf("Checking for parking spot \n");

    //Lighting(blue) sets lights to blue while looking for spot
    struct zmq_socket *p_zsocket = (struct zmq_socket *) p_socket;
    int available;

    pthread_mutex_lock(&mutex_spot);
    available = spot;
    pthread_mutex_unlock(&mutex_spot);

    //Available == 0 when parking spot has not yet been found
    //peds == 0 when there is nothing around the car
    while(available == 0){
        pthread_mutex_lock(&mutex_peds);
        if (peds == 1){
            stop(p_zsocket);
            pthread_cond_wait(&condvar, &mutex_peds);

        }
        else{
            forward(20, 1, p_zsocket);
        }
        pthread_mutex_unlock(&mutex_peds);
        pthread_mutex_lock(&mutex_spot);
        available = spot;
        pthread_mutex_unlock(&mutex_spot);
    }
    stop(p_zsocket);
    return (void *)1;
}

double time_remaining(struct timespec *time_start,struct timespec *time_end, double expected){

    double delta_sec = time_end->tv_sec - time_start->tv_sec;
    double delta_nsec = time_end->tv_nsec - time_start->tv_nsec;
    double pulse_duration = delta_sec + (delta_nsec/pow(10, 9));
    double remaining = expected- pulse_duration;
    return remaining;
}

void set_timer(struct timespec *timeout_time, double expected, struct timespec *time_start){
    clock_gettime(CLOCK_REALTIME, time_start);
    timeout_time->tv_sec = time_start->tv_sec + (int)expected;
    expected = (expected - (int)expected) * pow(10,9);
    timeout_time->tv_nsec = time_start->tv_nsec + expected;

}

void *p_park(void *p_socket){           //Hardcoded parallel park. Modify to dynamically park
    printf("Parking \n");

    //Lighting(yellow) sets lights to yellow while parking
    struct zmq_socket *p_zsocket = (struct zmq_socket *) p_socket;

    typedef void *(*FN)(int speed, double t, struct zmq_socket * xsocket);
    FN br = &back_right;
    FN b = &backward;
    FN bl = &back_left;
    FN f = &forward;


    struct timespec time_start, time_end, timeout_time;

    int i = 0;
    int park_complete = 0;
    double time_elapsed = 0;
    int block;


    //User set inputs
    FN motion[PARKING_MOVES]={br,b,bl,f};
    double pause_time[PARKING_MOVES] = {2,1,1,2};
    int speed[PARKING_MOVES] = {30,30,30,20};
    int flags[PARKING_MOVES] = {0,0,0,1};


    while(i < PARKING_MOVES){
        pthread_mutex_lock(&mutex_peds_back);
        if (flags[i] == 0){
            printf("%d", pause_time[i]);
            while(pause_time[i] > 0){
                if(peds_back == 0){
                    set_timer(&timeout_time, pause_time[i], &time_start);
                    motion[i](speed[i],pause_time[i],p_zsocket);
                    pthread_cond_timedwait(&condvar_peds_back, &mutex_peds_back, &timeout_time);
                    clock_gettime(CLOCK_REALTIME, &time_end);
                    pause_time[i] = time_remaining(&time_start, &time_end, pause_time[i]);
                }
                else{
                    stop(p_zsocket);
                    pthread_cond_wait(&condvar_peds_back, &mutex_peds_back);
                }
            }
        }
        pthread_mutex_unlock(&mutex_peds_back);

        pthread_mutex_lock(&mutex_peds);
        if (flags[i] == 1){
            while(pause_time[i] > 0){
                if(peds == 0){
                    set_timer(&timeout_time, pause_time[i], &time_start);
                    motion[i](speed[i],pause_time[i],p_zsocket);
                    pthread_cond_timedwait(&condvar, &mutex_peds, &timeout_time);
                    clock_gettime(CLOCK_REALTIME, &time_end);
                    pause_time[i] = time_remaining(&time_start, &time_end, pause_time[i]);
                }
                else{
                    stop(p_zsocket);
                    pthread_cond_wait(&condvar, &mutex_peds);
                }
            }
        }
        pthread_mutex_unlock(&mutex_peds);
        i++;


    }
    printf("DONE ALL MOVES\n");
    stop(p_zsocket);
    return NULL;
}

void *dist_detect(void *zgpio){
    printf("Beginning distance detection protocol \n");
    double dist;
    struct gpio_pins *p_gpio = (struct gpio_pins *) zgpio;
    double prev_state = 0;
    double time_elapsed = 0;
    struct timespec time_start;
    struct timespec time_stop;//start timer here
    clock_gettime(CLOCK_REALTIME, &time_start);
    clock_gettime(CLOCK_REALTIME, &time_stop);
    double delta_sec;
    double delta_nsec;
    int k= 0;

    sensor_init(p_gpio);
    printf("Initialization complete");

    while(1){
        //pthread_mutex_lock(&mutex_distance);
        dist = distance(p_gpio);

        if (dist < 1) {
            dist = prev_state;
        }

        //pthread_mutex_unlock(&mutex_distance);
        usleep(100000);
        printf("dist: %f\n sensor: %d\n", dist, p_gpio->checker);
        //writes to shared variable if distance is unsafe in front of car
        if(p_gpio->checker == 1){
            if(dist < MIN_DISTANCE){
                pthread_mutex_lock(&mutex_peds);
                peds = 1;
                pthread_cond_broadcast(&condvar);
                pthread_mutex_unlock(&mutex_peds);
            }
            else{
                pthread_mutex_lock(&mutex_peds);
                peds = 0;
                pthread_cond_broadcast(&condvar);
                pthread_mutex_unlock(&mutex_peds);
            }
        }
        //writes to shared variable if distance is unsafe behind car
        else if(p_gpio->checker == 2){
            if(dist < MIN_DISTANCE){
                pthread_mutex_lock(&mutex_peds_back);
                peds_back = 1;
                pthread_cond_broadcast(&condvar_peds_back);
                pthread_mutex_unlock(&mutex_peds_back);
            }
            else{
                pthread_mutex_lock(&mutex_peds);
                peds_back = 0;
                pthread_cond_broadcast(&condvar_peds_back);
                pthread_mutex_unlock(&mutex_peds);
            }
        }
        //writes to shared variable spot if distance has been greater than max distance for
        //specified length of time
        //Corresponds to frontright ultrasonic sensor
        else if (p_gpio->checker == 3){
            if(dist > MAX_DISTANCE && k == 0){
                clock_gettime(CLOCK_REALTIME, &time_start);
                k = 1;
            }
            else if( dist < MIN_DISTANCE_SIDES && k == 1){
                clock_gettime(CLOCK_REALTIME, &time_stop);
                k = 0;

                delta_sec = time_stop.tv_sec - time_start.tv_sec;
                delta_nsec = time_stop.tv_nsec - time_start.tv_nsec;
                time_elapsed = delta_sec + delta_nsec/pow(10, 9);
            }

            printf("TIME ELAPSED: %f \t k: %d\n\n\n\n", time_elapsed, k );
            if(time_elapsed >= OPEN_TIME){
                pthread_mutex_lock(&mutex_open_found);
                open_found = 1;
                pthread_cond_broadcast(&condvar_open_found);
                pthread_mutex_unlock(&mutex_open_found);
                return NULL;
            }

        }
        //Causes car to stop when distance changes from greater than Max Distance
        //to less than max distance
        //Corresponds to backright ultrasonic sensor.
        else if(p_gpio->checker == 4){
            pthread_mutex_lock(&mutex_open_found);
            while(open_found == 0){
                //printf("Waiting\n");
                pthread_cond_wait(&condvar_open_found, &mutex_open_found);
            }
            printf("OPEN WAS FOUND!!!!!!!!!!!!!!!!!!!!");
            pthread_mutex_unlock(&mutex_open_found);
            if (dist < MIN_DISTANCE_SIDES){
                pthread_mutex_lock(&mutex_spot);
                spot = 1;
                printf("SPOT IS NOW 1");
                pthread_cond_broadcast(&condvar_spot);
                pthread_mutex_unlock(&mutex_spot);
            }
        }
        prev_state = dist;

    }
    return NULL;
}

void *User(void *p_socket){
    int BLT_client[2];
    init_BLT_sock(BLT_client);
    char message[1024];
    while(1){
        BLT_Recv(BLT_client[1], message);
        send_instr(message, p_socket);
    }
    BLT_end(BLT_client);

}

int main(){
    //Set main thread priority higher than all other threads
    pthread_t mainid = pthread_self();
    struct sched_param schedule_paramMain;
    schedule_paramMain.sched_priority = 99;
    pthread_setschedparam(mainid,SCHED_FIFO, &schedule_paramMain);
    //initialize sockets and io structs
    static struct zmq_socket zsocket;
    struct gpio_pins IO_forward;
    struct gpio_pins IO_backward;
    struct gpio_pins IO_right_f;
    struct gpio_pins IO_right_b;
    void *start_park;


    //Pin initialization of ultrasonic sensors
    IO_forward.trigger = 66;
    IO_forward.echo = 67;
    IO_forward.checker = 1;

    IO_backward.trigger = 27;
    IO_backward.echo = 65;
    IO_backward.checker = 2;

//SOMETHING WRONG WITH HARDWARE FOR THIS ONE
    IO_right_b.trigger = 69;
    IO_right_b.echo = 68;
    IO_right_b.checker = 4;

    IO_right_f.trigger = 47;
    IO_right_f.echo = 46;
    IO_right_f.checker = 3;
    //initialize socket
    init_socket(&zsocket);

    //create threads
    pthread_t f1, uf, ub, us1, us2, p1;
    pthread_attr_t a_attr;
    struct sched_param schedule_paramA;
    pthread_attr_init(&a_attr);
    pthread_attr_setinheritsched(&a_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&a_attr, SCHED_RR);
    pthread_attr_setschedparam(&a_attr, &schedule_paramA);

    pthread_attr_t user_attr;
    struct sched_param schedule_user;
    pthread_attr_init(&user_attr);
    pthread_attr_setinheritsched(&user_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&user_attr, SCHED_FIFO);
    schedule_user.sched_priority = 50;
    pthread_attr_setschedparam(&user_attr, &schedule_user);



    //Create mutexes and conditional variables
    pthread_mutexattr_t attr;
		// This solves a known problem with locking and unlocking a fast, nonrecursive
		// mutex too often
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&mutex_spot, &attr);
    pthread_mutex_init(&mutex_peds, &attr);
    pthread_mutex_init(&mutex_peds_back, &attr);
    pthread_mutex_init(&mutex_open_found, &attr);
    pthread_mutex_init(&mutex_distance, &attr);
    pthread_mutexattr_destroy(&attr);

    //Create conditional variable attributes
    pthread_condattr_t condattr1;
		pthread_condattr_init(&condattr1);
		pthread_cond_init(&condvar, &condattr1);
    pthread_cond_init(&condvar_peds_back, &condattr1);
    pthread_cond_init(&condvar_spot, &condattr1);
		pthread_cond_init(&condvar_open_found, &condattr1);

    //pthread_create(&us1, &a_attr, dist_detect, (void *) &IO_right_f);
    //pthread_create(&us2, &a_attr, dist_detect, (void *) &IO_right_f);
    //pthread_attr_destroy(&a_attr);
    //pthread_join(us1,NULL);
    //pthread_join(us2,NULL);


    //Create threads that check the forward and side sensors
    pthread_create(&uf, &a_attr, dist_detect, (void *) &IO_forward);
    pthread_create(&us1, &a_attr, dist_detect, (void *) &IO_right_f);
    pthread_create(&us2, &a_attr, dist_detect, (void *) &IO_right_b);
    //pthread_attr_destroy(&a_attr);
		
		printf("After joining threads\n");

    //Create thread to drive forward until open spot is found
    pthread_create(&f1, &a_attr, find_spot, (void *) &zsocket);

    pthread_join(f1, &start_park);

    //Once spot is found stop side sensors and begin parking protocol

    if ((int)start_park == 1){
        pthread_cancel(us1);
        pthread_cancel(us2);
        printf("Beginning Parking Protocol Now");
        usleep(3000000);
        pthread_create(&p1, NULL, p_park, (void *) &zsocket);
        pthread_create(&ub, NULL, dist_detect, (void *) &IO_backward);
    }

    pthread_join(p1,NULL);
    pthread_cancel(uf);
    pthread_cancel(ub);

    stop(&zsocket);
    return 0;
}
