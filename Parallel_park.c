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

//Global shared variables, mutex protected
int spot = 0;                  //Indicates spot is found and parking protocol should begin
int peds = 0;                  //Indicates if it is safe to move forward
int peds_back = 0;             //Indiacates if it is safe to move in reverse
int open_found = 0;            //Indicates if spot large enough for car is located (not yet time to park)
char last_user_move = '0';     //The last move sent by android app
int temp_dist = MIN_DISTANCE;  //Minimum distance allowed before car should stop moving in reverse

int check_forward = 1;         // Flag representing if back or forward sensor safety should be checked


//Move forward until open spot detected. If path blocked car will halt.
//Once spot is detected function will terminate
void *find_spot(void *p_socket){
    printf("Checking for parking spot \n");

    struct zmq_socket *p_zsocket = (struct zmq_socket *) p_socket;
    int available;

    pthread_mutex_lock(&mutex_spot);
    available = spot;
    pthread_mutex_unlock(&mutex_spot);

    while(available == 0){
        pthread_mutex_lock(&mutex_peds);
        if (peds == 1){
            stop(p_zsocket);
            pthread_cond_wait(&condvar, &mutex_peds);

        }
        else{
            forward(20, 1, p_zsocket);
            pthread_cond_wait(&condvar, &mutex_peds);
        }
        pthread_mutex_unlock(&mutex_peds);
        pthread_mutex_lock(&mutex_spot);
        available = spot;
        pthread_mutex_unlock(&mutex_spot);
    }

    stop(p_zsocket);
    return (void *) 1;
}

//Determine time remaining given expected amount of time to pass and the pulse duration
double time_remaining(struct timespec *time_start,struct timespec *time_end, double expected){

    double delta_sec = time_end->tv_sec - time_start->tv_sec;
    double delta_nsec = time_end->tv_nsec - time_start->tv_nsec;
    double pulse_duration = delta_sec + (delta_nsec/pow(10, 9));
    double remaining = expected - pulse_duration;
    return remaining;
}

//Set timeout time to some time in the future based on the amount of runntime remaining
void set_timer(struct timespec *timeout_time, double expected, struct timespec *time_start){
    clock_gettime(CLOCK_REALTIME, time_start);
    timeout_time->tv_sec = time_start->tv_sec + (int)expected;
    expected = (expected - (int)expected) * pow(10,9);
    timeout_time->tv_nsec = time_start->tv_nsec + expected;

}

//Performs parking protocol based on preset instructions. If obstacles detected
//in path will halt and resume once obstacles are removed.
void *p_park(void *p_socket){
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

    //User set inputs
    FN motion[PARKING_MOVES]={br,b,bl,f};                 //Array of function pointers representing motions
    double pause_time[PARKING_MOVES] = {2,.75,.75,.5};    //Array of pause times correlating to motions
    int speed[PARKING_MOVES] = {40,20,80,20};             //Array of speeds correlating to motions
    int flags[PARKING_MOVES] = {0,0,0,1};                 //Sensor flag array if 0 check back sensor if 1 check front
                                                          //corresponds with motion
    //All moves in motion array are executed
    while(i < PARKING_MOVES){
        //Checks if it is safe to continue moving in reverse. If not recalculates amount of time that
        //parking maneuver should execute for, while accounting for pause time.
        pthread_mutex_lock(&mutex_peds_back);
        if (flags[i] == 0){
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
        //Same as reverse function above, but checks the forward sensor
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

//Runs distance protocols for ultrasonic sensors based on sensor location.
void *dist_detect(void *zgpio){
    printf("Beginning distance detection protocol \n");
    double dist = 2000;
    struct gpio_pins *p_gpio = (struct gpio_pins *) zgpio;
    double prev_state = 0;                               //Last distance read by sensor
    double time_elapsed = 0;                             //Time elapsed from pulse start to pulse end
    double time_stopped = 0;                             //Time that car was halted due to safety concerns
    double tot_time_elapsed = 0;                         //
    struct timespec time_start;                         //Records time to calculate pulse length
    struct timespec time_stop;
    struct timespec mid_start;                           //Records time to calculate time halted
    struct timespec mid_stop;
    clock_gettime(CLOCK_REALTIME, &time_start);
    clock_gettime(CLOCK_REALTIME, &time_stop);
    double delta_sec, delta_nsec;
    double delta_msec, delta_mnsec;
    int k = 0;                                           //Used to create edge detection
    int y = 0;                                           //Used to create edge detection

    sensor_init(p_gpio);
    printf("Initialization complete\n");

    while(1){
        dist = distance(p_gpio);

        if (dist < 1) {
            dist = prev_state;
        }

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
            if(dist < temp_dist){
                pthread_mutex_lock(&mutex_peds_back);
                peds_back = 1;
                pthread_cond_broadcast(&condvar_peds_back);
                pthread_mutex_unlock(&mutex_peds_back);
            }
            else{
                pthread_mutex_lock(&mutex_peds_back);
                peds_back = 0;
                pthread_cond_broadcast(&condvar_peds_back);
                pthread_mutex_unlock(&mutex_peds_back);
            }
						//usleep(1000);
        }
        //writes to shared variable, spot, if distance has been greater than max distance for
        //specified length of time. Corresponds to frontright ultrasonic sensor
        else if (p_gpio->checker == 3){
            pthread_mutex_lock(&mutex_peds);
            if(peds == 1 && y == 0 && k == 1){
                clock_gettime(CLOCK_REALTIME, &mid_start);
                y = 1;
            }
            else if (peds == 0 && y == 1 && k == 1){
                clock_gettime(CLOCK_REALTIME, &mid_stop);
                y = 0;
                //calculating time car has been halted due to safety concerns
                delta_msec = mid_stop.tv_sec - mid_start.tv_sec;
                delta_mnsec = mid_stop.tv_nsec - mid_start.tv_nsec;
                time_stopped = delta_msec + delta_mnsec/pow(10, 9);
                tot_time_elapsed = tot_time_elapsed - time_stopped;
            }
            pthread_mutex_unlock(&mutex_peds);

            if(dist > MAX_DISTANCE && k == 0){
                clock_gettime(CLOCK_REALTIME, &time_start);
                k = 1;
				tot_time_elapsed = 0;
            }
            //Calculate size of spot based on time that it took for car to detect edges of parking spot
            else if( dist < MIN_DISTANCE_SIDES && k == 1){
                clock_gettime(CLOCK_REALTIME, &time_stop);
                k = 0;

                delta_sec = time_stop.tv_sec - time_start.tv_sec;
                delta_nsec = time_stop.tv_nsec - time_start.tv_nsec;
                time_elapsed = (delta_sec + delta_nsec/pow(10, 9));
                tot_time_elapsed = tot_time_elapsed + time_elapsed;

                //Writes to shared variable open found, when spot is big enough to park
                if(tot_time_elapsed >= OPEN_TIME){
                    pthread_mutex_lock(&mutex_open_found);
                    open_found = 1;
                    pthread_cond_broadcast(&condvar_open_found);
                    pthread_mutex_unlock(&mutex_open_found);
                    return NULL;
                }
                else{
                    tot_time_elapsed = 0;
                }
            }
            printf("TIME ELAPSED: %f\n\n\n\n", tot_time_elapsed);
        }
        //Causes car to stop when distance changes from greater than Max Distance
        //to less than max distance. This alerts other functions that it is time to begin parking
        //Corresponds to backright ultrasonic sensor.
        else if(p_gpio->checker == 4){
            pthread_mutex_lock(&mutex_open_found);
            while(open_found == 0){
                pthread_cond_wait(&condvar_open_found, &mutex_open_found);
            }
            pthread_mutex_unlock(&mutex_open_found);

            dist = distance(p_gpio);

            if (dist < MIN_DISTANCE_SIDES){
                pthread_mutex_lock(&mutex_spot);
                spot = 1;
                pthread_cond_broadcast(&condvar_spot);
                pthread_mutex_unlock(&mutex_spot);
            }
        }
        prev_state = dist;
    }
    return NULL;
}

//Reads in user input from Android App. Determines which sensors need to be checked for safety
void *User(void *p_socket){
    int BLT_client[2];
    init_BLT_sock(BLT_client);
    char message[1024];
    while(1){
        BLT_Recv(BLT_client[1], message);
		send_instr(message, p_socket);
        if(message[0] == '7'){
            break;
        }

        last_user_move = message[0];
		if (last_user_move == '1' || last_user_move == '3' || last_user_move == '5') {
            check_forward = 0;
		}
        else {
            check_forward = 1;
		}
    }
    BLT_end(BLT_client);
}

//Checks if it is safe to move forward. If not overrides user input and halts car
void *User_Override(void *p_socket){
    while(1){
        last_user_move = (int)(last_user_move - '0');
		if (check_forward) {
            pthread_mutex_lock(&mutex_peds);
            if (peds == 1){
                stop(p_socket);
                pthread_cond_wait(&condvar, &mutex_peds);
            }
			else{
                pthread_cond_wait(&condvar, &mutex_peds);
			}
            pthread_mutex_unlock(&mutex_peds);
        }
        usleep(100000);
    }
}
//Checks if safe to move in reverse. If not overrides user input and halts car.
void *User_Override2(void *p_socket){
    while(1){
        if(!check_forward){
            pthread_mutex_lock(&mutex_peds_back);
            if (peds_back == 1){
                stop(p_socket);
                pthread_cond_wait(&condvar_peds_back, &mutex_peds_back);
            }
		/*	else{
			     pthread_cond_wait(&condvar_peds_back, &mutex_peds_back);
			} */
            pthread_mutex_unlock(&mutex_peds_back);
        }
        usleep(100000);
    }
    return NULL;
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

    IO_right_b.trigger = 69;
    IO_right_b.echo = 68;
    IO_right_b.checker = 4;

    IO_right_f.trigger = 47;
    IO_right_f.echo = 46;
    IO_right_f.checker = 3;

    //initialize socket
    init_socket(&zsocket);

    //create threads
    pthread_t f1, uf, ub, us1, us2, p1, user, user_o, user_o2;
    pthread_attr_t a_attr;
    struct sched_param schedule_paramA;
    pthread_attr_init(&a_attr);
    pthread_attr_setinheritsched(&a_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&a_attr, SCHED_RR);
    pthread_attr_setschedparam(&a_attr, &schedule_paramA);

    //Create mutexes and conditional variables
    pthread_mutexattr_t attr;
	// This solves a known problem with locking and unlocking a fast, nonrecursive mutex too often
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

    //Phase I: User controlled car with safety halt features
    pthread_create(&user, &a_attr, User, (void *) &zsocket);
    pthread_create(&user_o, &a_attr, User_Override, (void *) &zsocket);
	pthread_create(&user_o2, &a_attr, User_Override2, (void *) &zsocket);
    pthread_create(&uf, &a_attr, dist_detect, (void *) &IO_forward);
    pthread_create(&ub, &a_attr, dist_detect, (void *) &IO_backward);
    pthread_join(user, NULL);
    pthread_cancel(user_o);
	pthread_cancel(user_o2);
    pthread_cancel(ub);
    pthread_cancel(uf);

    //Phase II: Search for parking spot
    pthread_create(&uf, &a_attr, dist_detect, (void *) &IO_forward);
    pthread_create(&us1, &a_attr, dist_detect, (void *) &IO_right_f);
    pthread_create(&us2, &a_attr, dist_detect, (void *) &IO_right_b);
    pthread_create(&f1, &a_attr, find_spot, (void *) &zsocket);
    pthread_join(f1, &start_park);

    //Phase III: Halt car and begin parking protocol
    if ((int)start_park == 1){
        pthread_cancel(us1);
        pthread_cancel(us2);
        temp_dist = 10;
        printf("Beginning Parking Protocol Now\n");
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
