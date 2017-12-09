import MotorBridge
import time


ClockWise = 1
CounterClockWise = 2

#Initialize motor object with user defined frequency - normally 1000
#@param PWM frequency
#@return initialized motor object
def motor_init(frequency):
    motor  = MotorBridge.MotorBridgeCape()

    motor.DCMotorInit(1, frequency)
    motor.DCMotorInit(2, frequency)
    motor.DCMotorInit(3, frequency)
    motor.DCMotorInit(4, frequency)

    return motor
		
#Move car forward
#@param duty cycle of motor to set speed
#@param initialized motor object
#@param amount of time to move in direction minimum time = .001
def Forward(pwm, motor,t):
    motor.DCMotorMove(1, ClockWise, pwm)
    motor.DCMotorMove(2,ClockWise,pwm)
    motor.DCMotorMove(3,ClockWise,pwm)
    motor.DCMotorMove(4,ClockWise,pwm)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Move car backward
#@param duty cycle of motor to set speed
#@param initialized motor object
#@param amount of time to move in direction
def Backward(pwm, motor,t):
    motor.DCMotorMove(1,CounterClockWise, pwm)
    motor.DCMotorMove(2,CounterClockWise,pwm)
    motor.DCMotorMove(3,CounterClockWise,pwm)
    motor.DCMotorMove(4,CounterClockWise,pwm)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Move car backward and right
#@param duty cycle of motor to set speed of the two faster wheels must be greater than 10
#@param initialized motor object
#@param amount of time to move in direction
def BackRight(pwm,motor,t):
    motor.DCMotorMove(3,CounterClockWise,pwm)
    motor.DCMotorMove(4,CounterClockWise,pwm)
    motor.DCMotorMove(1,CounterClockWise, 10)
    motor.DCMotorMove(2,CounterClockWise, 10)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Move car forward and right
#@param duty cycle of motor to set speed of the two faster wheels must be greater than 10
#@param initialized motor object
#@param amount of time to move in direction
def ForwardRight(pwm, motor,t):
    motor.DCMotorMove(3,ClockWise,pwm)
    motor.DCMotorMove(4,ClockWise,pwm)
    motor.DCMotorMove(1,ClockWise, 10)
    motor.DCMotorMove(2,ClockWise, 10)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Move car backward and left
#@param duty cycle of motor to set speed of the two faster wheels must be greater than 10
#@param initialized motor object
#@param amount of time to move in direction
def BackLeft(pwm,motor,t):
    motor.DCMotorMove(1,CounterClockWise,pwm)
    motor.DCMotorMove(2,CounterClockWise,pwm)
    motor.DCMotorMove(3,CounterClockWise, 10)
    motor.DCMotorMove(4,CounterClockWise, 10)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Move car forward and left
#@param duty cycle of motor to set speed of the two faster wheels must be greater than 10
#@param initialized motor object
#@param amount of time to move in direction
def ForwardLeft(pwm,motor,t):
    motor.DCMotorMove(1,ClockWise,pwm)
    motor.DCMotorMove(2,ClockWise,pwm)
    motor.DCMotorMove(3,ClockWise, 10)
    motor.DCMotorMove(4,ClockWise, 10)
<<<<<<< HEAD
		
=======
    #time.sleep(t)
    #Stop(motor)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
#Stops all motors
#@param initialized motor object
def Stop(motor):
    motor.DCMotorStop(1)
    motor.DCMotorStop(2)
    motor.DCMotorStop(3)
    motor.DCMotorStop(4)


#Potential Parallel Parking Movement Ratios
if __name__ == "__main__":
    motors = motor_init(1000)
<<<<<<< HEAD
    Stop(motors) # This ensures that the motors will stop after the program is run
=======
#    BackRight(30, motors, 1)
#    BackLeft(25, motors, 2)
    #Forward(20, motors, 1)
#    Backward(20, motors, 1)
    #sleep(2)
    Stop(motors)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
