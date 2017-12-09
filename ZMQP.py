#!/usr/bin/python
import zmq
import time

# Import all of the motor control functions
from DC_Control import *
<<<<<<< HEAD
=======
def ZMQP():
    port = "5555 "
    #initialize motor object with PWM frequency of 1000
    motor = motor_init(1000)
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc

#This function creates a ZMQ reply server and connects to something on port 5555. It receives instructions
#from the request client and controls the motor in the desired way given the instruction
def ZMQP():
    #initialize motor object with PWM frequency of 1000
    motor = motor_init(1000)

<<<<<<< HEAD
		# Connecting to anything on port 5555 using ZMQ. This program will initialize a
		# reply server that will communicate with the main C process.
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    print("Binding to socket...")
    socket.bind("tcp://*:5555")

    while True:
        message = socket.recv(0,True,False)
        message = str(message)
        print ("Received request: %s " % message)

				# Removing any trailing null characters from the received message
        message = message.replace("\x00", "")

				# The fourth variable, garbage is whatever extra bytes were sent along with the
				# intended message because the message size is fixed and can be longer than the
				# actual instruction
        func,speed,time,garbage = message.split(",")
        func = int(func)
        speed = int(speed)
        time = float(time)
        #Control motors based on message
        if (func == 0):
            Forward(speed, motor, time)
        elif (func == 1):
            Backward(speed, motor, time)
        elif (func == 2):
            ForwardLeft(speed, motor, time)
        elif (func == 3):
            BackLeft(speed, motor, time)
        elif (func == 4):
            ForwardRight(speed, motor, time)
        elif (func == 5):
            BackRight(speed, motor, time)
        elif (func == 6):
            Stop(motor)

        #send acknowledge of motor motion
        socket.send("Movement complete %s" % port)
ZMQP() # This ensures that the program will run when it is called from the command line
=======
    while True:
        message = socket.recv(0,True,False)
        message = str(message)
        print ("Received request: %s " % message)
        #parse message

        message = message.replace("\x00", "")


        #func,speed,time,garbage = map((int),message.split(","))
        func,speed,time,garbage = message.split(",")
        func = int(func)
        speed = int(speed)
        time = float(time)
        #Control motors based on message
        if (func == 0):
            Forward(speed, motor, time)
        elif (func == 1):
            Backward(speed, motor, time)
        elif (func == 2):
            ForwardLeft(speed, motor, time)
        elif (func == 3):
            BackLeft(speed, motor, time)
        elif (func == 4):
            ForwardRight(speed, motor, time)
        elif (func == 5):
            BackRight(speed, motor, time)
        elif (func == 6):
            Stop(motor)

        #send acknowledge of motor motion
        socket.send("Movement complete %s" % port)
ZMQP()
>>>>>>> 8886df0b96ecb8f11d6e39ca52627fc30126b4dc
