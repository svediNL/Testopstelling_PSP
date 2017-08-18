  

   # Polymer Science Park Testopstelling
   
  Author(s):
  Sven Dicker - sdsdsven@gmail.com - 0639842173
  
  This program is part of a test setup for large scale 3D printing
  The controller architecture consists of 3 different components: a pc, an arduino and a labjack
  The pc is the master of the wholesetup, the arduino and labjack are slaves.
  
  The pc is resonsible for the interaction between the operator and the setup. 
  The user fills in certain parameters and chooses a method of controlling the setup, 
  methods like controling individual motors or running a test print. 
  Based on the control parameters, the program then calculates certain parameters 
  which are relevant for controlling the stepper drivers. Then the parameters are send
  along with an excecution command which will activate a certain mode which are explained below.
     
  The arduino is responsible for controlling the motor drivers and reading the endswitches.
  The parameters and methods/modes of controlling the motor drivers are based on commands received
  over serial communication. There are two types of commands parameter: parameter commands and excecution commands.
  Parameter commands have a relevant parameter whereas excecution commands do not.
  However a parameter has to be given to every send command, 
  in the case of an excecution command this parameter will usually be 0.
 
## Modes
 There are 6 different modes: disabled, break, homing, speed, print and jog.
  
  - DisableAll - disables all motors, no current will be applied the windings so the motrs are free to rotate
  this is the default mode.
  
  - Break - stops pulsing the motor drivers but keeps the motors enable. This mode is used after jog mode because
  of the delay that occurs in the drivers when the motors switch from disabled to enabled.
  
  - Homing - sets the vertical direction upward and then enables the vertical motor. When the top end witch is activated
  the motors stop and move downward until the switch is deactivated. Then the DisableAll mode is activated.
  
  - Speed - motors are driven based on a set pulse period for each motor
  
  - Print - platform moter is driven based on speed, vertical motor is driven so that the platform will drop a certain
  distance for every rotation.
  
  - JogPlatform/JogVertical - will drive a certain motor based on the set pulse period, pretty musch the same as Speed only
 for one specified motor.


