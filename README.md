# Autonomous-Parallel-Parking-Car

Software required to make a battery-powered car that is capable of being remotely controlled over Bluetooth
via an Android app and able to autonomously parallel park itself when commanded.

The Python code includes a motor control library written by the developers of the motor control board that
was used. The other two Python files map the lower-level motor control commands in the library to commands
to move the car in certain directions and communicate with the C code via a client-server model.

The C code includes many threads doing tasks such as checking to see if the car will collide with any
objects going forward or backward, checking to see if there is an open parking space, and checking to see if there
are any new commands coming from the Android app over Bluetooth. The C code communicates with the Python code via a
client-server model, where the C code requests the Python code to execute certain motor control commands to drive
the car.

The Android app allows the user to make the car move forward and backward, turn, and start the parallel parking
procedure, which will work as long as the parking space (which is just an open space with obstructions on either side
of it) is parallel and to the right of the car.
