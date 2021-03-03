#include <sys/wait.h>

#include "utils.h"

int main(int argc,char **argv){

	int numOfBayes,busesCreated;
	char bayName[BAY_BUFFER];
	pid_t pid;
	int stationManager_workingTime;
	
	if(argc>3 || argc<=1){
		perror("Execution flags of myprocess - wrong input ");
		exit(1);
	}
	printf("\n\t~ Simulation of a station starts now ~ \n");

	/*------------------- Reading configuration file -------------------*/
	
	FILE * configFile = fopen(argv[2],"r");
	if(configFile==NULL){
		perror("Can't read configuration file");
		exit(1);
	}
	char ch_pid[INT_BUFFER],busPassengers[INT_BUFFER],maxBusPassengers[INT_BUFFER],maxParking[INT_BUFFER],manuverTime[INT_BUFFER];
	fscanf(configFile,"%d %d\n",&numOfBayes,&busesCreated);
	int maxCapacity[numOfBayes];
	fscanf(configFile,"%d %d %d\n ",&maxCapacity[ASK],&maxCapacity[VOR],&maxCapacity[PEL]);

	int max_busesStation=0;
	for(int i=0;i<NUM_OF_BAYES;i++)
		max_busesStation += maxCapacity[i];
	
	int offset=0;
	for(int i=0;i<NUM_OF_BAYES;i++)
		offset = offset + maxCapacity[i];


	/* -------------- Initialing semaphores ------------------*/
	sharedMemory_Data data,*temp;
	
	sem_init(&data.station_isFull,NON_ZERO,max_busesStation);
	sem_init(&data.station_isActive,NON_ZERO,0);
	sem_init(&data.bus_Request,NON_ZERO,0);
	sem_init(&data.waiting_inEntrance,NON_ZERO,0);
	sem_init(&data.waiting_inExit,NON_ZERO,0);
	sem_init(&data.bus_inStation,NON_ZERO,0);
	sem_init(&data.bus_inExit,NON_ZERO,0);
	sem_init(&data.bus_justFinished,NON_ZERO,1);
	sem_init(&data.bay_Stop[ASK],NON_ZERO,1);
	sem_init(&data.bay_Stop[VOR],NON_ZERO,1);
	sem_init(&data.bay_Stop[PEL],NON_ZERO,1);
	sem_init(&data.comptroller_inAction,NON_ZERO,1);

	data.bus_toExit = false;
	data.bus_toEntrance = false;

	data.current_busType[IN] = -1;
	data.current_busType[OUT] = -1;
	data.current_busID[IN] = -1;
	data.current_busID[OUT] = -1;

	for(int i=0;i<NUM_OF_BAYES;i++){

		data.available_parkingPosition[i]= maxCapacity[i];
		data.curPassengers_perBay[i] = 0;
		data.curCapacity_perBay[i]=0;
		data.maxCapacity_perBay[i] = maxCapacity[i];
		data.isfull[i]=false;
		data.total_passengersDeparted[i]=0;
		data.total_passengersArrived[i]=0;
		data.averageTime_perBay[i]=0.0;
		data.numOfBusesServed[i]=0;

	}

	data.waiting_forService = busesCreated;
	data.max_numOfBuses = offset;
	data.averageTime=0.0;

	/*------------- Creating shared memory ----------------*/
	unsigned int shmid=0;

	
	shmid = shmget(IPC_PRIVATE,(sizeof(sharedMemory_Data)+sizeof(Position)+offset*sizeof(int)),SEGMENTPERM);

	if(shmid == -1){
		perror("Creation of shared memory failed");
		exit(1);
	}
	char ch_shmid[SHMID_BUFFER];
	sprintf(ch_shmid,"%d",shmid);

	temp  = (sharedMemory_Data * ) shmat(shmid,(void*)0,0);
	memcpy(temp,&data,sizeof(sharedMemory_Data));
	temp = temp + 1;
	memset(temp,0,(offset*sizeof(int)+sizeof(Position)));
	temp--;

	int choise;
	printf("\n> Type 1 for automatic creation of processes or 0 for manual creation : ");
	scanf("%d",&choise);
	printf("\n");

	if(choise){
		/* _______________________ Processes generations __________________________________*/



		/* --------------- Station-manager creation -----------------------*/
		
		switch(fork()){
			case ERROR:
				perror("Fork failed");
				exit(1);
			case 0: 	/* Child process */
				execlp("./station-manager","./station-manager","-s",ch_shmid,NULL);			
			default:	/* Parent process */
				break;
		}


		/* -------------- Buses creation ----------------------- */
		
		for(int b=0;b<busesCreated;b++){
			fscanf(configFile,"%s %s %s %s %s\n",bayName,busPassengers,maxBusPassengers,maxParking,manuverTime);		
			switch(fork()){
				case ERROR:
					perror("Fork failed");
					exit(1);
				case 0: 	/* Child process */
					pid = getpid();
					sprintf(ch_pid,"%d",pid);
					execlp("./bus","./bus","-t",bayName,"-n",busPassengers,"-c",maxBusPassengers,"-p",maxParking,"-m",manuverTime,"-s",ch_shmid,"-id",ch_pid,NULL);			
				default:	/* Parent process */
					break;
			}
		}

		/* -------------- Comptroller creation ----------------------- */

		int curent_Situation,statistics_Time;
		char ch_cur[INT_BUFFER],ch_st[INT_BUFFER];
		fscanf(configFile,"%d %d\n ",&curent_Situation,&statistics_Time);
		sprintf(ch_cur,"%d",curent_Situation);
		sprintf(ch_st,"%d",statistics_Time);

		switch(fork()){
			case ERROR:
				perror("Fork failed");
				exit(1);
			case 0: 	/* Child process */
				execlp("./comptroller","./comptroller","-s",ch_shmid,"-d",ch_cur,"-t",ch_st,NULL);			
			default:	/* Parent process */
				break;
		}

		while(wait(NULL)>0);

	}else{

		stationManager_workingTime = 100;
		printf("Shared segment id is : %d \n",shmid );
		int choise2;
		printf("\n>> How many seconds station manager will work? (press 0 for default value)\n");
		printf("Seconds: ");
		scanf("%d",&choise2);
		printf("\n");
		printf("This programm will finish after serving %d buses (informed from configuration file)\n",busesCreated );
		if(choise2 == 0){
			printf("Alright, time will be fixed to default value that is %d seconds\n",stationManager_workingTime );
			printf("Ready to serve buses! Start!\n");
			sleep(stationManager_workingTime);
		}else{
			printf("Ready to serve buses! Start!\n");
			sleep(choise2);
		}
		

	}





	/* ---------------- Semaphores destruction ---------------*/

	if(sem_destroy(&data.station_isFull) != 0){
		perror("Semaphore failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.station_isActive) != 0){
		perror("Semaphore station failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.bus_Request) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.waiting_inEntrance) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.waiting_inExit) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.bus_inStation) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.bus_inExit) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.bus_justFinished) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}
	if(sem_destroy(&data.comptroller_inAction) != 0){
		perror("Semaphore mutex failed to destroy ");
		exit(1);
	}

	for(int i=0;i<NUM_OF_BAYES;i++){
		if(sem_destroy(&data.bay_Stop[i]) != 0){
			perror("Semaphore mutex failed to destroy ");
			exit(1);
		}		
	}

	/* ------------- Shared memory destruction --------------*/
	
	if(shmdt((void*)temp) == ERROR){
		perror("Shared segment detachement failed");
		exit(1);
	}	

	if(shmctl(shmid,IPC_RMID,0) == ERROR){
		perror("Shared segment removal failed");
		exit(1);
	}

	fclose(configFile);

	printf("\n\t~ End of simulation , Bye! ~ \n");

return 0;	
}