//Assume car is in position
//Car will move straight until open spot is detected
//Determine size of spot
//Initiate parallel parking protocol
pthread_mutex_t mutex_spot;
pthread_mutex_t mutex_peds;
pthread_mutex_t mutex_peds_back;
pthread_mutex_t mutex_open_found;

pthread_cond_t condvar;
pthread_cond_t condvar_peds_back;
pthread_cond_t condvar_spot;

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
    struct zmq_socket *p_zsocket = (struct zmq_socket *) p_socket;
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
    while(available == 0){
        pthread_mutex_lock(&mutex_peds);
        if (peds == 1){
            pthread_cond_wait(&condvar, &mutex_peds);
            stop(p_zsocket);
        }
        else{
            forward(20, 1, p_zsocket);
        }
        pthread_mutex_unlock(&mutex_peds);
        pthread_mutex_lock(&mutex_spot);
        available = spot;
        pthread_mutex_unlock(&mutex_spot);

        printf("Available: %d\t Not Safe: %d\n",available,nsafe);
    }
    return (void *)1;
}

void *p_park(void *p_socket){           //Hardcoded parallel park. Modify to dynamically park
    printf("Parking \n");
    struct zmq_socket *p_zsocket = (struct zmq_socket *) p_socket;
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
    /*
    while(1){
        pthread_mutex_lock(&mutex_peds);
        if (peds == 1){
            pthread_cond_wait(&condvar, &mutex_peds);
            stop(p_zsocket);
        }
        else{
            //READ NEXT INSTRUCTION FROM THE LIST
            back_right(30, 2, p_zsocket);
            back_left(30, 1, p_zsocket);

            forward(30, 1, p_zsocket);
            backward(20, 1, p_zsocket);
            stop(p_zsocket);

        }

        pthread_mutex_lock(&mutex_peds_back);
            nsafe_back = peds_back;
        pthread_mutex_unlock(&mutex_peds_back);

        pthread_mutex_lock(&mutex_peds);
            nsafe = peds;
        pthread_mutex_unlock(&mutex_peds);
    }
    */
    return NULL;
}

void *dist_detect(void *zgpio){
    printf("Beginning distance detection protocol \n");
    double dist;
    struct gpio_pins *p_gpio = (struct gpio_pins *) zgpio;
    double prev_state;
    double time_elapsed;
    struct timespec time_start;
    struct timespec time_stop;//start timer here

    sensor_init(p_gpio);

    while(1){
        dist = distance(p_gpio);
        printf("dist: %f\n", dist);
        //writes to shared variable if distance is unsafe
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
        //writes to shared variable if distance is unsafe
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
            if(dist > MAX_DISTANCE){
                clock_gettime(CLOCK_REALTIME, &time_start);
            }
            else if( dist < MIN_DISTANCE_SIDES){
                clock_gettime(CLOCK_REALTIME, &time_stop);
            }

            delta_sec = time_stop.tv_sec - time_start.tv_sec;
            delta_nsec = time_stop.tv_nsec - time_start.tv_nsec;
            time_elapsed = delta_sec + delta_nsec/pow(10, 9);

            if(time_elapsed > OPEN_TIME){
                pthread_mutex_lock(&mutex_open_found);
                open_found = 1;
                pthread_mutex_unlock(&mutex_open_found);
            }

        }
        //Causes car to stop when distance changes from greater than Max Distance
        //to less than max distance
        //Corresponds to backright ultrasonic sensor.
        else if(p_gpio->checker == 4){
            if (prev_state > MAX_DISTANCE && dist < MIN_DISTANCE_SIDES && open_found == 1){
                pthread_mutex_lock(&mutex_spot);
                spot = 1;
                pthread_cond_broadcast(&condvar_spot);
                pthread_mutex_unlock(&mutex_spot);

            }
            prev_state = dist;
        }
    }
    return NULL;
}



int main(){
    //initialize sockets and io structs
    static struct zmq_socket zsocket;
    struct gpio_pins IO_forward;
    struct gpio_pins IO_backward;
    struct gpio_pins IO_right_f;
    struct gpio_pins IO_right_b;
    void *start_park;


    //Pin initialization of ultrasonic sensors
    IO_forward.trigger = 45;
    IO_forward.echo = 44;
    IO_forward.checker = 1;

    //IO_backward.trigger = 45;
    //IO_backward.echo = 44;
    //IO_backward.checker = 1;

    //IO_right_f.trigger = 45;
    //IO_right_f.echo = 44;

    //IO_right_b.trigger = 45;
    //IO_right_b.echo = 44;

    //initialize socket
    init_socket(&zsocket);

    //create threads
    pthread_t f1, uf, ub, us1, us2, p1;

    //Create mutexes and conditional variables
    pthread_mutexattr_t attr;
    pthread_mutex_init(&mutex_spot, &attr);
    pthread_mutex_init(&mutex_peds, &attr);
    pthread_mutex_init(&mutex_peds_back, &attr);
    pthread_mutexattr_destroy(&attr);

    pthread_condattr_t condattr1;
	pthread_condattr_init(&condattr1);
	pthread_cond_init(&condvar, &condattr1);

    //Create threads that check the forward and side sensors
    pthread_create(&uf, NULL, dist_detect, (void *) &IO_forward);
    //pthread_create(&us1, NULL, dist_detect, (void *) &IO_right_f);
    //pthread_create(&us2, NULL, dist_detect, (void *) &IO_right_b);

    //Create thread to drive forward until open spot is found
    pthread_create(&f1, NULL, find_spot, (void *) &zsocket);

    //Side sensors and find spot threads no longer needed during park
    pthread_join(f1, &start_park);
    //pthread_join(us1,NULL);
    //pthread_join(us2,NULL);

    /*if ((int)start_park == 1){
        pthread_create(&p1, NULL, p_park, (void *) &zsocket);
        //pthread_create(&ub, NULL, dist_detect, (void *) &IO_backward);
    }*/

    pthread_join(p1,NULL);
    pthread_join(ub,NULL);
    pthread_join(uf,NULL);

    stop(&zsocket);
    return 0;
}
