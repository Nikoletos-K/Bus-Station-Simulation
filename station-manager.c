#include "utils.h"

int main(int argc,char **argv){

	int in=1;
	unsigned int shmid;
	int bayParked;
	int busCounter=1;
	time_t timenow1,timenow2;
	struct tm* time_info;
	char timeStr1[CUR_TIME],timeStr2[CUR_TIME];
	char bayExit[BAY_BUFFER];
	int positionOut;

	

	FILE * ledger = fopen("referenceLedger.txt","w+");
	FILE * history = fopen("stationHistory.txt","w+");

	fprintf(history,"\n+-------------------------- Station information ---------------------------+\n\n");
	fprintf(history,"Station manager information for all the buses he served : \n\n");

	fprintf(ledger,"\n----------------------------- Reference ledger ----------------------------\n\n");


	
	if(in==argc){
		perror("> Station-manager process input needs more arguments\n");
		exit(1);			
	}


	while(in<argc){
		if(!strcmp("-s",argv[in])){
			shmid = atoi(argv[in+1]);
			break;
		}
		in++;
	}

	fprintf(stdout , "> Station manager just created with shared segment id : %u \n",shmid );

	sharedMemory_Data * smData;
	smData =(sharedMemory_Data *) shmat(shmid,(void *)0,0);
	if(*(int * )smData == ERROR){
		perror("Shared memory attachment in station manager failed");
		exit(1);
	}


	int offset = smData->max_numOfBuses;
	sharedMemory_Data * s = smData;		// moving pointer to the segment where integer are stored 
	s = s + 1;
	int * temp = (int *) s;
	temp = temp + offset;		// moving the pointer offset value


	Position * p = malloc(sizeof(Position));
	int space=0;


	sem_post(&(smData->station_isActive));	// Semaphore for unblocking buses that created before station manager

	while(smData->waiting_forService>0){	// while there's no bus waiting for servive

		sem_wait(&(smData->bus_Request));	// Semaphore that activates station manager , if a bus wants him to enter or exit
		int bay,position;
		char bayName[BAY_BUFFER];

		sem_wait(&(smData->comptroller_inAction));		// Semaphore to protect data from comptroller (mutex) , only one process can read that data
		

		/*--------------------- IF A BUS WANTS TO ENTER -----------------------------*/
		if(smData->bus_toEntrance && (space=bay_hasSpace(smData,&bay,&position))){

			sem_post(&(smData->comptroller_inAction));		// reactivate comptroler

			/*---------- Capture current time ------------*/
			time(&timenow1);
			time_info = localtime(&timenow1);
			strftime(timeStr1,sizeof(timeStr1),"%H:%M:%S",time_info);

			/*------------------- Writting the values that the bus will read and informing ledger ---------------------*/
			get_bayName(bay,bayName);
			fprintf(stdout,"> Station-manager: Bus %d ,can enter the station to bay %s and in position %d (%s)\n",smData->current_busID[IN],bayName,position,timeStr1);
			fprintf(ledger,"%s | -> Bus %d enters the station to bay %s and in position %d \n",timeStr1,smData->current_busID[IN],bayName,position);
			smData->bayParked = bay;
			smData->busPosition = position;
			fprintf(stdout,"> Station-manager: Bus %d can now do a manuver \n",smData->current_busID[IN]);

			/* --------------------- Letting bus enter the station ---------------------------*/
			sem_post(&(smData->bus_inStation));

			/* ------ Wait untill bus reads that information ------------*/
			sem_wait(&(smData->waiting_inEntrance));
			fprintf(stdout,"> Station-manager: Bus %d ended manuver, please next bus come in entrance\n",smData->current_busID[IN]);			

			sem_post(&(smData->station_isActive));		// Let next bus come to entrance

		}else if(smData->bus_toEntrance && space==0 && smData->isfull[bay] == false){	// if a bus wants to enter and there's no space -> A flag activates
			get_bayName(bay,bayName);
			fprintf(stdout,"> Station-manager: Bus %d you can't enter the station , bay %s is full , wait untill a bus leaves\n",smData->current_busID[IN],bayName);
			sem_post(&(smData->comptroller_inAction));	
			smData->isfull[bay] = true;
		}else	//for restoring comptroller semaphore
			sem_post(&(smData->comptroller_inAction));	
			
		
		/*--------------------- IF A BUS WANTS TO EXIT -----------------------------*/
		if(smData->bus_toExit){

			sem_post(&(smData->bus_inExit));	// Let bus come in station manager		
			fprintf(stdout,"> Station-manager: Ready to serve a bus that wants to exit\n");

			sem_wait(&(smData->waiting_inExit));	// Wait untill bus comes
	
			bayParked = smData->current_busType[OUT];	// Read what bus want to tell
			positionOut  = smData->positionOut;
			get_bayName(bayParked,bayExit);

			/*---------- currentTime -----------------*/
			time(&timenow2);
			time_info = localtime(&timenow2);
			strftime(timeStr2,sizeof(timeStr2),"%H:%M:%S",time_info);
			fprintf(ledger,"%s | <- Bus %d exits the station (Bay: %s , Position: %d)\n",timeStr2,smData->current_busID[OUT],bayExit,positionOut);

			memcpy(p,temp,sizeof(Position));
			print_toLedger(p,history,busCounter);		// inform history and ledger
			busCounter++;	
			


			fprintf(stdout,"> Station-manager: Bus %d has been served,departing from the station (%s)\n",smData->current_busID[OUT],timeStr2);
			sem_post(&(smData->bus_justFinished));		// Make station manager available to another bus that wants to exit 

			
			smData->waiting_forService--;		// decrement counter -> one bus left

			/* In case a bus of the same bay ,with the one left, is stacked in the entrance because there was no space when it came,check it and let it in */
			if(smData->isfull[bayParked] && smData->bus_toEntrance){
				sem_post(&(smData->bus_Request));
				smData->isfull[bayParked]=false;	

			}
		}
	}

	fprintf(stdout,"> Station manager ended his work and now going home! \n");
	if(shmdt((void*)smData) == ERROR){
		perror("Shared segment detachement failed");
		exit(1);
	}	
	fclose(ledger);
	fclose(history);
	free(p);

return 0;
}
