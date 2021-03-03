# Bus-Station-Simulation
Standalone programs that run at the same time and simulate the operation of a Bus Station. Project tsk is to get familiar with multiprocess programs, shared memory, signals and semaphores. This project was part of the subject Operational Systems in my bachelor.

## Files included
1. ```create_configFile.c``` : Program that creates the configuration file \ Run with ```./create_configFile``` , with the following options-flags:
   - c bay_maxCapacity (upper bound-random)
   - p max_number_of_passengers (upper bound-random)
   - t max_parking_time (upper bound-random)
   - b number_of_buses that will be created
   - m time in seconds that manuver will last
   - d seconds for comptroller to print current situation of station
   - st .. for statistics
   - wt time that program mystation will need in order to end ,in case of multiple ttys
2. ```utils.c``` : useful functions for bus.c,stationmanager.c,comtroller.c
3. ``` utils.h ```: include file
4. ```bus.c```

## Program execution
__Compile:__ ```make``` \
__Execution cmds:__
 - Bus: ```./bus -t type -n incpassengers -c capacity -p parkperiod -m mantime -s shmid ```
 - Station manager: ``` ./station-manager -s shmid ```
 - Comptroller: ``` ./comptroller -d time -t stattimes -s shmid ```
 - System: ``` ./mystation -l configfile ```
 
Implemented 3 of types processes where each has a different role. The three types of processes are:
1. __Buses__ wishing to enter the station, disembark passengers, stay for someone finite period of time and finally after receiving passengers to leave. The buses travel to different parts of the country which are: 
    - Attica, Central & Central Greece (ASK),
    - Peloponnese (PEL), and 
    - Northern Greece (VOR).
2. __Station manager__. 
3. __Station operation monitor (comptroller)__.

## Project functionality
The above types of processes are standalone programs that can run at the same time
to implement collaboratively what is happening in the area of a bus station. 

Every bus that arrives at a random time wants to enter the station, to park on the islet
instructed to disembark passengers, to wait for a period of time, to board the public
who has bought tickets, and leave. 

Before entering the station, every bus should
consult with the station-manager in order to get the relevant ok but also to be informed about the islet
who will eventually be able to park. \
After the station-manager makes sure it does not there is too much traffic in the station area, it gives ok to one of the many buses waiting  off station to proceed. This vehicle enters, parks, and stays in the specific location
has been given for as long as it takes to complete the unloading / loading process
passengers. \
The role of the station – manager is to supervise the safe operation of the infrastructure and to record in
a log (reference ledger) all the activity that takes place at the station. 

Depending on the arrival time and the travel area of the bus, for each vehicle, the reference ledger lists the
relevant information such as time of arrival, number plate, bus type, final parking location (or island),
number of passengers disembarking and status of the vehicle that has arrived (ie vehicle present at the station).
When a bus has to depart and after a number of passengers have boarded, it follows the
Expected sequence of events: comes to an agreement and waits to get ok from the station–
manager. 

When the latter issues a departure order (after clarifying that there is no other outgoing / incoming traffic), the ledger shall indicate the time of the said departure of the vehicle, number
passenger status, and changed status (ie vehicle has left a station).
The comptroller is an independent program that regularly provides the status to
Station (ie which and how many buses are present). 
