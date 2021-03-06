
/** distance.c
 *
 *  Created on: Oct 29, 2017
 *  Author: Samvrutha Tumuluru, Michael Byrd
 *
 **/
 #include "distance.h"
/**
This function initializes the ultrasonic sensor
**/

void sensor_init(struct gpio_pins *IO){
    printf("Initializing sensors please wait .... \n");

    //Request gpio pins
    IO->gpio_output = libsoc_gpio_request(IO->trigger, LS_SHARED);
    IO->gpio_input = libsoc_gpio_request(IO->echo, LS_SHARED);
    //Set GPIO pins
    libsoc_gpio_set_direction(IO->gpio_output, OUTPUT);
    libsoc_gpio_set_direction(IO->gpio_input, INPUT);


}

/**
 * Calulate distance to object in front of sensor
**/

double distance(struct gpio_pins *IO){
    //struct gpio_pins *IO = (struct gpio_pins *) p_gpio;
    struct timespec pulse_start;
    struct timespec pulse_end;
    double delta_sec;
    double delta_nsec;
    double pulse_duration;
    double dist;
    //Emit 10 usec pulse

    int set = libsoc_gpio_set_level(IO->gpio_output, LOW);
    usleep(10);
    int set_in = libsoc_gpio_set_level(IO->gpio_output, HIGH);

    int i = 0;
    int level = libsoc_gpio_get_level(IO->gpio_input);
    //perror("Input error: ");
    //Detect beginning and end of pulse
    //printf("Level: %d " , level);
    while ((level == 1) && (i < 10000)){
        level = libsoc_gpio_get_level(IO->gpio_input);
    	i++;
    }
    clock_gettime(CLOCK_REALTIME, &pulse_start);
    while (level == 0){
        level = libsoc_gpio_get_level(IO->gpio_input);
    }
    clock_gettime(CLOCK_REALTIME, &pulse_end);

    //Calculate time elapsed
    delta_sec = pulse_end.tv_sec - pulse_start.tv_sec;
    delta_nsec = pulse_end.tv_nsec - pulse_start.tv_nsec;
    pulse_duration = delta_sec + delta_nsec/pow(10, 9);

    //Calculate distance
    //printf("Pulse duration: %f \t Delta_sec: %f \t Delta_nsec %f\n", pulse_duration, delta_sec, delta_nsec);

    dist = pulse_duration * 17150;
    dist = roundf(dist * 100) / 100;

    return dist;

}

/**
  *Sleeps for user specified number of nanoseconds.
**/
/*
void wait(int nanosec){
    struct timespec ts;
    double curr_nsec;
    double curr_sec;
    double start_time;

    //Determine current time
    clock_gettime(CLOCK_REALTIME, &ts);
    curr_nsec = ts.tv_nsec;
    curr_sec = ts.tv_sec;

    //Increment current time by specified number of nanoseconds
    start_time = curr_sec + curr_nsec/pow(10, 9);
    start_time += ((double)nanosec)/pow(10, 9);

    ts.tv_sec = (int)start_time;
    ts.tv_nsec = (start_time - ts.tv_sec) * pow(10, 9);

    //Begin delay
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);

}
*/

/*main(){
    double dist;
    struct gpio_pins IO_forward;
    IO_forward.trigger = 45;
    IO_forward.echo = 44;
    IO_forward.checker = 1;
    printf("Main method is beginning\n");

    sensor_init(&IO_forward);
    printf("Initialization has completed. Sensing will now begin...\n");

    while (1){
        dist = distance(&IO_forward);
        printf("Distance from sensor is %f \n" , dist);
        usleep(100000);
    }

}*/
