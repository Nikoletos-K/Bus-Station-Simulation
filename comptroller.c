#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

int main(int argc,char **argv){

	fprintf(stdout,"~Comptroller just created \n");

	int in=1,statPeriod,statCalc,shmid;
	time_t timenow1,timenow2;
	struct tm* time_info;
	char timeStr1[CUR_TIME],timeStr2[CUR_TIME];
	
	if(in==argc){
		perror("Comptroller process input needs more arguments\n");
		exit(1);			
	}

	while(in<argc){
		switch(comptrollerArgs(argv[in])){
			case TIME:
				statPeriod = atoi(argv[in+1]);
				break;
			case STAT_TIMES:
				 statCalc= atoi(argv[in+1]);
				break;
			case SHMID:
				shmid = atoi(argv[in+1]);
				break;
			case INPUT_ERROR:
				perror("Comptroller process wrong input data \n");	
				exit(1);
		}
		in = in+2;		
	}


	sharedMemory_Data * smData;
	smData =(sharedMemory_Data *) shmat(shmid,(void *)0,0);
	if(*(int * )smData == ERROR){
		perror("Shared memory attachment in bus process failed");
		exit(1);
	}

	sharedMemory_Data * s = smData;
	s++;
	int * temp;

	int waiting_forService=1;
	while(waiting_forService>0){

		temp = (int * ) s;
		sleep(statPeriod);
		sem_wait(&(smData->comptroller_inAction));
		waiting_forService = smData->waiting_forService	;
		time(&timenow1);
		time_info = localtime(&timenow1);
		strftime(timeStr1,sizeof(timeStr1),"%H:%M:%S",time_info);
	

		fprintf(stdout,"\n\t+------------------------------------------------------------------+\n");	
		fprintf(stdout,"\n\t\t Time : %s \n",timeStr1);	

		fprintf(stdout,"\n\t\t\t_STATION_CURRENT_SITUATION_\n\n");
		
		for(int i=0;i<NUM_OF_BAYES;i++){
			char name[BAY_BUFFER];
			get_bayName(i,name);
			fprintf(stdout,"\n\t\t Bay %s",name);
			fprintf(stdout,"\n\n\t\t\tPassengers ,at this moment, in this bay : %d \n",smData->curPassengers_perBay[i]);
			fprintf(stdout,"\t\t\tBuses parked : %d \n",(smData->curCapacity_perBay[i]));
			fprintf(stdout,"\t\t\tAvailable parking positions : %d \n\n ",smData->available_parkingPosition[i]);
			fprintf(stdout,"\t\t\tAvailable positions of this bay: ");

			for(int p=0;p<smData->maxCapacity_perBay[i];p++){
				if(*temp==0)
					fprintf(stdout,"%d ",p);
				temp++;
			}
			fprintf(stdout,"\n\n");

		}
		fprintf(stdout,"\n\t\t Station ");
		fprintf(stdout,"\n\n\t\t\tPassengers ,at this moment in station  : %d \n",smData->curPassengers_perBay[ASK]+smData->curPassengers_perBay[PEL]+smData->curPassengers_perBay[VOR]);
		fprintf(stdout,"\t\t\tBuses parked : %d \n",(smData->curCapacity_perBay[ASK]+smData->curCapacity_perBay[PEL])+smData->curCapacity_perBay[VOR]);
		fprintf(stdout,"\t\t\tAvailable parking positions : %d \n\n ",smData->available_parkingPosition[ASK]+smData->available_parkingPosition[PEL]+smData->available_parkingPosition[VOR]);
		fprintf(stdout,"\n\t+------------------------------------------------------------------+\n\n");	

		sem_post(&(smData->comptroller_inAction));

		sleep(statCalc);

		sem_wait(&(smData->comptroller_inAction));
		waiting_forService = smData->waiting_forService	;
		time(&timenow2);
		time_info = localtime(&timenow2);
		strftime(timeStr2,sizeof(timeStr2),"%H:%M:%S",time_info);

		fprintf(stdout,"\n\t+------------------------------------------------------------------+\n");			
		fprintf(stdout,"\n\t\t Time : %s \n",timeStr2);	

		fprintf(stdout,"\n\t\t\t_STATION_STATISTICS_\n\n");
		
		for(int i=0;i<NUM_OF_BAYES;i++){
			char name[BAY_BUFFER];
			get_bayName(i,name);
			fprintf(stdout,"\n\t\t Bay %s",name);
			fprintf(stdout,"\n\n\t\t\tTotal number of passengers arrived to this bay : %d \n",smData->total_passengersArrived[i]);
			fprintf(stdout,"\t\t\tTotal number of passengers departed from this bay : %d \n",smData->total_passengersDeparted[i]);
			fprintf(stdout,"\t\t\tNumber of buses entered to this bay : %d \n",smData->numOfBusesServed[i]+smData->curCapacity_perBay[i]);
			fprintf(stdout,"\t\t\tNumber of buses served from this bay : %d \n",smData->numOfBusesServed[i]);

			if( smData->numOfBusesServed[i] == 0)
				fprintf(stdout,"\t\t\tAverage time waiting not available yet \n\n");
			else{
				fprintf(stdout,"\t\t\tAverage time waiting to this bay : %.2lf \n\n",(smData->averageTime_perBay[i]/((double) smData->numOfBusesServed[i])));
			}
		}
		fprintf(stdout,"\n\t\t Station ");
		fprintf(stdout,"\n\n\t\t\tTotal number of passengers arrived to station : %d \n",smData->total_passengersArrived[ASK]+smData->total_passengersArrived[VOR]+smData->total_passengersArrived[PEL]);
		fprintf(stdout,"\t\t\tTotal number of passengers departed from station : %d \n",smData->total_passengersDeparted[ASK]+smData->total_passengersDeparted[VOR]+smData->total_passengersDeparted[PEL]);
		int counter1=0;
		for(int i=0;i<NUM_OF_BAYES;i++)
			counter1 += smData->numOfBusesServed[i]+smData->curCapacity_perBay[i];
		fprintf(stdout,"\t\t\tNumber of buses entered to station : %d \n",counter1);
		int counter2=0;
		for(int i=0;i<NUM_OF_BAYES;i++)
			counter2 += smData->numOfBusesServed[i];

		fprintf(stdout,"\t\t\tNumber of buses served from station : %d \n\n",counter2);

		if(counter2 == 0)
			fprintf(stdout,"\t\t\tAverage time waiting to station not available yet \n\n");
		else
			fprintf(stdout,"\t\t\tAverage time waiting for this station : %.2lf \n\n",(smData->averageTime/((double) counter2)));

		fprintf(stdout,"\n\t+------------------------------------------------------------------+\n\n");	

		sem_post(&(smData->comptroller_inAction));

	}

	fprintf(stdout,"Comptroller terminated \n");
	if(shmdt((void*)smData) == ERROR){
		perror("Shared segment detachement failed");
		exit(1);
	}	
	
return 0;
}