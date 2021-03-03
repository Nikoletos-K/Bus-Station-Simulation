#include "utils.h"


int main(int argc,char **argv){

	double t1,t2;
	struct tms tb1,tb2;
	double ticspersec;
	time_t timenow1,timenow2;
	struct tm* time_info;
	char timeStr1[CUR_TIME],timeStr2[CUR_TIME];
	int position,bayParked;
	int in=1,typeID,numOfPassengers,capacity,parkPeriod,manuverTime,shmid;
	char type[BAY_BUFFER];
	pid_t id;
	
	if(in==argc){
		perror("Bus process input needs more arguments\n");
		exit(1);			
	}

	while(in<argc){
		switch(busArgs(argv[in])){
			case TYPE:
				strcpy(type,argv[in+1]);
				typeID = get_bayID(type);
				break;
			case NUM_OF_PASSENGERS:
				numOfPassengers = atoi(argv[in+1]);
				break;
			case CAPACITY:
				capacity = atoi(argv[in+1]);
				break;
			case PARKPERIOD:
				parkPeriod = atoi(argv[in+1]);
				break;
			case MANTIME:
				manuverTime = atoi(argv[in+1]);
				break;
			case SHMID:
				shmid = atoi(argv[in+1]);
				break;
			case PID:
				id = atoi(argv[in+1]);
				break;
			case INPUT_ERROR:
				perror("Bus process wrong input data \n");	
				exit(1);
		}
		in = in+2;		
	}

	fprintf(stdout,"~Bus created with data : %d %s %d %d %d %d %d\n",id,type,numOfPassengers,capacity,parkPeriod,manuverTime,shmid );

	sharedMemory_Data * smData;
	smData =(sharedMemory_Data *) shmat(shmid,(void *)0,0);
	if(*(int * )smData == ERROR){
		perror("Shared memory attachment in bus process failed");
		exit(1);
	}

	int offset = smData->max_numOfBuses;
	sharedMemory_Data * s = smData;
	s = s + 1;
	int * temp = (int *) s;
	temp = temp + offset;



	/*----- Capture current time ------------*/
	time(&timenow1);
	time_info = localtime(&timenow1);
	strftime(timeStr1,sizeof(timeStr1),"%H:%M:%S",time_info);
	

	fprintf(stdout,"Bus %d: Arrival at the station \n",id);

	/*------- Calculate current time ------------*/
	ticspersec = (double) sysconf(_SC_CLK_TCK);
	t1 = (double) times(&tb1);

	sem_wait(&(smData->station_isFull));	
	sem_wait(&(smData->station_isActive));	// block if station hasn't created

	/*----------------- Write to shared segment data , in order to be read from station manager ------------------*/
	fprintf(stdout,"Bus %d: Is now in the entrance and communicates with the manager (paperwork) \n",id);	
	smData->bus_toEntrance = true;
	smData->current_busType[IN] = typeID;
	smData->current_busID[IN] = id;

	sem_post(&(smData->bus_Request));	// Bus sends a request to manager in order to read that data and answer

	fprintf(stdout,"Bus %d: Waits for manager responce\n",id);	

	sem_wait(&(smData->bus_inStation));	// wait untill station manager reads them and ansewrs

	fprintf(stdout,"Bus %d: Take's the okey from the station manager to manuver \n",id);	

	t2 = (double) times(&tb2);	// time waiting ends

	smData->bus_toEntrance = false;	// no longer wants to enter , change flag

	sleep(manuverTime);		// do manuver without any one else

	// reads bay and position that will park 
	position = smData->busPosition;
	bayParked = smData->bayParked;
	char bayPark[BAY_BUFFER];
	get_bayName(bayParked,bayPark);  
	fprintf(stdout,"Bus %d: Going to parking area in bay %s and position %d ,leaving manuver area \n",id,bayPark,position);	

	sem_post(&(smData->waiting_inEntrance));	// inform station manager that bus left from entrance and manuver area

	sem_wait(&(smData->bay_Stop[bayParked]));		// stop in your bay because maybe another bus loads passengers and must not be concurrently
	sem_wait(&(smData->comptroller_inAction));	// comptroller data

	fprintf(stdout,"Bus %d: Parked and it's now ready to leave and load passengres \n",id);	
	smData->curPassengers_perBay[bayParked] += numOfPassengers;
	smData->total_passengersArrived[bayParked] += numOfPassengers;
	fprintf(stdout,"Bus %d: Left passengers \n",id);	

	sem_post(&(smData->comptroller_inAction));	// reacticate 

	sleep(parkPeriod);	

	sem_post(&(smData->bay_Stop[bayParked]));	// passengers movement to station ended ,another bus can now leave passengers

	sem_wait(&(smData->bus_justFinished));	// wait in order to start communicating to load passengers and getting ready to leave
	smData->bus_toExit = true;	// mark that bus wants to leave

	sem_post(&(smData->bus_Request));	// sends bus request for starting preparing to leave
	sem_wait(&(smData->bus_inExit));	// waits until an answer
	
	// communicates with station manager
	fprintf(stdout,"Bus %d: Communicates with station manager in order to leave passengers and prepare for manuver \n",id);	
	smData->current_busType[OUT] = bayParked;
	smData->current_busID[OUT] = id;
	smData->bus_toExit = false;
	smData->positionOut = position;

	sem_wait(&(smData->comptroller_inAction));	// comproller data
	smData->total_passengersDeparted[bayParked] += capacity;	// leave passengers
	smData->curPassengers_perBay[bayParked] -= numOfPassengers;	
	position_Available(smData,position,bayParked);
	sem_post(&(smData->comptroller_inAction));
	fprintf(stdout,"Bus %d: Starts manuver to exit \n",id);	

	sleep(manuverTime);		// does manuver
	fprintf(stdout,"Bus %d: Ended manuver at exiting \n",id);	

	/*---------- currentTime -----------------*/
	time(&timenow2);
	time_info = localtime(&timenow2);
	strftime(timeStr2,sizeof(timeStr2),"%H:%M:%S",time_info);

	/*------------- sends data to station manager --------------*/

	Position * p = malloc(sizeof(Position));
	p->position = position;
	p->busid = id;
	p->type = typeID;
	p->bayParked = bayParked;
	p->passengers_toLoad = capacity;
	p->passengers_toLeave = numOfPassengers;
	strcpy(p->timeOfArrival,timeStr1);
	strcpy(p->timeOfDeparture,timeStr2);

	memcpy(temp,p,sizeof(Position));
	

	sem_post(&(smData->station_isFull)); // bus leaving

	sem_post(&(smData->waiting_inExit));	// leaves from exit

	fprintf(stdout,"-Bus %d: Exiting the station \n",id );

	
	sem_wait(&(smData->comptroller_inAction));		// informing data that comptroller needs
	double timePassed = ((double) (t2-t1))/((double) ticspersec);
	smData->averageTime_perBay[bayParked] += timePassed;
	smData->averageTime += timePassed;
	smData->numOfBusesServed[bayParked]++;	
	sem_post(&(smData->comptroller_inAction));

	
	if(shmdt((void*)smData) == ERROR){
		perror("Shared segment detachement failed");
		exit(1);
	}	

	free(p);
return 0;
}