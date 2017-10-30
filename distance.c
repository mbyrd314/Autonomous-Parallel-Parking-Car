
/**
Calulate distance to object in front of sensor
@param integer representing pin used by BBB for trigger, integer representing pin used by BBB for echo
@return Returns distance as a float
**/
double distance(int trigger, int echo){

gpio *gpio_output, *gpio_input;
struct timespec pulse_start;
struct timespec pulse_end;
double delta_sec;
double delta_nsec;
double pulse_duration;
double dist;


//Request gpio pins
gpio_output = libsoc_gpio_request(trigger, LS_SHARED);
gpio_input = libsoc_gpio_request(echo, LS_SHARED);
//Set GPIO pins
libsoc_gpio_set_direction(gpio_output, OUTPUT);
libsoc_gpio_set_direction(gpio_input, INPUT);

//Emit 10 usec pulse
libsoc_gpio_set_level(gpio_output, HIGH);
wait(10000);
libsoc_gpio_set_level(gpio_output, LOW);

//Check for beginning and end of pulse
while (libsoc_gpio_get_level(gpio_input) == LOW){
    clock_gettime(CLOCK_REALTIME, &pulse_start);
}
while (libsoc_gpio_get_level(gpio_input) == HIGH){
    clock_gettime(CLOCK_REALTIME, &pulse_end);
}

//Compute time elapsed
delta_sec = pulse_end.t_sec - pulse_start.t_sec;
delta_nsec = pulse_end.t_nsec - pulse_start.t_nsec;
pulse_duration = delta_sec + delta_nsec/pow(10, 9);

//Compute distance
dist = pulse_duration * 17150;
dist = roundf(dist * 100) / 100;
//printf("%f", distance)
return dist;

}

/**Sleeps for user specified number of nanoseconds.
@param integer number representing nanoseconds
@return no output
**/

void wait(int nanosec){
    struct timespec ts;
    double curr_nsec;
    double curr_sec;
    double start_time;

    //Determine current time
    clock_gettime(CLOCK_REALTIME, &ts)
    curr_nsec = ts.tv_nsec;
    curr_sec = ts.tv_sec;

    //Increment current time by specified number of nanoseconds
    start_time = curr_sec + curr_nsec/pow(10, 9);
    start_time += ((double)nanosec)/pow(10, 9);

    ts.tv_sec = (int)curr_sec;
    ts.tv_nsec = (start_time - ts.tv_sec) * pow(10, 9);

    //Begin delay
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);

}
main(){
    while (1){
        dist = distance(10, 9); //PIN NUMBER USED ON BBB
        printf("Distance from sensor is %f \n" , dist);
        delay(1000);
    }
}
