import zmq
import time

from DC_Control import *
def ZMQP():
    port = "5555"
    #initialize motor object with PWM frequency of 1000
    motor = motor_init(1000)

    context = zmq.Context()
    socket = context.socket(zmq.REP)
    print("Binding to socket...")
    socket.bind("tcp://*:5555")

    while True:
        #CHECK DOCUMENTATION ABOUT VALUES
        message = str(socket.recv(0,True,False))
        print ("Received request: %s " % message)
        #parse message
        message = message.replace("\x00", "")

        func,speed,time = map((int),message.split(","))

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
