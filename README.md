# Polymer Science Park Testopstelling
   
  Author(s):
  Sven Dicker - sdsdsven@gmail.com - 0639842173
  
  This program is part of a test setup for large scale 3D printing. The system consists of 3 different controlling components: a pc, an arduino and a labjack. The pc is the master of the wholesetup, the arduino and labjack are slaves.
  
  The pc is responsible for the interaction between the system and the setup. The user fills in certain parameters and chooses a method of controlling the setup, methods like controling individual motors or running a test print. Based on the control parameters, the program then calculates certain parameters which are relevant for controlling the stepper drivers. Then the parameters are send to the arduino along with an excecution command which will activate a certain mode which are explained below.
  
  The labjack is used to read analog signals from other control units in the system like the extruder frequency driver, barrel pressure sensor display and temperature controllers which could other wise not be connected. 
     
  The arduino is responsible for controlling the motor drivers and reading the endswitches. The parameters and methods/modes of controlling the motor drivers are based on commands received over serial communication. There are two types of commands parameter: parameter commands and excecution commands.Parameter commands have a relevant parameter whereas excecution commands do not. However a parameter has to be given to every send command,in the case of an excecution command this parameter will usually be 0.
 
## Interface
The interface consists of three layout arrangements, control mode, system setting and sensor display. 

### Control mode layout
The layout control mode is used to send certain commands to the arduino which will drive the motor drivers. The layout constist of an initiate button and a tab widget. The tab widget has the tabs, each tab is a different method of controlling the system. The initiate button will send an initiate command to the arduino which will trigger the homing sequence. 

#### Print tab
The tab "print" is used for actual testing. The user has the ability to run the motors based on continuous movement or steps. The user can configure the speed of the platform along with the layer height. When the user presses start the platform will start rotating and a message box pops up which asks if the extruder is in position. When the user clicks ready the vertical motor will start as well. When the user clicks cancel the system will stop and return to the disabled mode. Parameters can be edited while system is running and will be applied directly.

#### Motor tab
The tab "motor" is used to control the motors individually (or together) based on speed. The user can select what motor they want to be enabled and the directions of the system. The user can also set speed based parameters.

#### Manual tab
The tab "manual" is used to send commands directly to the arduino. This is basically the same as using the serial monitor in the arduino IDE.

### System settings layout
The layout "system sttings" is used to set certain (static) parameters of the system like step resolution or motor reductions. The layout consists of only a tab widget which has three tabs, general, motors and control parameters.

#### General tab
The general tab is used for system parameters which could not be classified. The parameters are set here: the ramp time, the com port of the arduino and the path to log files. The ramp time is used when the motors have to speed up/slow down.

#### Motors tab 
The motors tab is used to set the stepping resolution of each stepper driver, the reduction between motor and load, and the spindle pitch.

#### Control Parameters
Not yet used, but could contain parameters for a PID controller or something.

### Sensor layout
The sensor layout displays all the relevant sesnor data received using the labjack. For now only pressure and set extruder speed are displayed as numerical values. In the future more sensor data will be added along with a graphical representation like a graph.

The sensor data should also have an export button which will export the sensor data in a csv-file. 
