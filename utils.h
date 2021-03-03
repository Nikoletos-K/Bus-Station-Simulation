#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/times.h>
#include <time.h>


#define SEGMENT_SIZE sizeof(sharedMemory_Data)
#define CUR_TIME sizeof("HH:MM:SS")
#define SEGMENTPERM 0666
#define NON_ZERO 1
#define NUM_OF_BAYES 3
#define IN_AND_OUT 2
#define IN 1
#define OUT 0 
#define ACTIVE 1

#define true 1
#define false 0

typedef enum buffer_size {

	BAY_BUFFER = 4,SHMID_BUFFER=20,INT_BUFFER=10

} Buffer;

typedef enum busArguments {

	TYPE,NUM_OF_PASSENGERS,CAPACITY,PARKPERIOD,MANTIME,SHMID,PID

} busArguments;

typedef enum comptrollerArguments {

	TIME,STAT_TIMES

} comptrollerArguments;


typedef enum errors {

	INPUT_ERROR = -2,ERROR = -1,EMPTY = -1

} Error;

typedef enum Bayes {

	ASK=0,PEL=1,VOR=2

} Bayes;

typedef struct Position {

	int position;

	int busid;
	int type;
	
	int bayParked;

	int passengers_toLoad;
	int passengers_toLeave;
	
	char timeOfArrival[CUR_TIME];
	char timeOfDeparture[CUR_TIME];
	

} Position ;

typedef struct sharedMemory_Data {

	/* ---------- Semaphores ---------*/
	sem_t station_isFull;
	sem_t station_isActive;
	sem_t bus_Request;
	sem_t waiting_inEntrance;
	sem_t waiting_inExit;
	sem_t bus_inStation;
	sem_t bus_inExit;
	sem_t bus_justFinished;
	sem_t bay_Stop[NUM_OF_BAYES];
	sem_t comptroller_inAction;

	/*------------ Counters and flags ---------------*/
	
	int maxCapacity_perBay[NUM_OF_BAYES];
	int curCapacity_perBay[NUM_OF_BAYES];

	int bus_toExit;
	int bus_toEntrance;

	int current_busType[IN_AND_OUT];
	int current_busID[IN_AND_OUT];

	int available_parkingPosition[NUM_OF_BAYES];
	
	int waiting_forService;
	
	int busPosition;
	int bayParked;
	
	int isfull[3];

	int max_numOfBuses;

	/*---------- Statistics info --------*/

	int total_passengersDeparted[NUM_OF_BAYES];
	int total_passengersArrived[NUM_OF_BAYES];
	int curPassengers_perBay[NUM_OF_BAYES];

	int numOfBusesServed[NUM_OF_BAYES];
	int positionOut;
	double averageTime_perBay[NUM_OF_BAYES];
	double averageTime;


} sharedMemory_Data;



int busArgs(char * input);
int comptrollerArgs(char * input);
int bay_hasSpace(sharedMemory_Data * smData,int * bay_Parked,int * position);
void get_bayName(int bay,char * bayName);
int get_bayID(char * bayName);
int position_Available(sharedMemory_Data * smData,int position,int bay);
void print_toLedger(Position * p,FILE * ledger,int numOfBus);
