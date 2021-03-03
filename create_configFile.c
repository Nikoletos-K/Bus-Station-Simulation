#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc,char **argv){


	char * bayes[3] = {"ASK","VOR","PEL"};
	int bayesCapacity[3];
	int bayCapacity;
	int i=1,passengers,numOfBayes=3,maxCapacity,maxPassengers,maxParkTime,numOfBuses,manuverTime;
	int stattistics_Time,curent_Situation,stationManager_WorkingTime ;
	srand(time(NULL));
	while(i<argc){

		if(!strcmp("-c",argv[i]))
			maxCapacity = atoi(argv[i+1]);
		else if(!strcmp("-p",argv[i]))
			maxPassengers = atoi(argv[i+1]);
		else if(!strcmp("-t",argv[i]))
			maxParkTime = atoi(argv[i+1]);
		else if(!strcmp("-b",argv[i]))
			numOfBuses = atoi(argv[i+1]);
		else if(!strcmp("-m",argv[i]))
			manuverTime = atoi(argv[i+1]);
		else if(!strcmp("-d",argv[i]))
			curent_Situation= atoi(argv[i+1]);
		else if(!strcmp("-st",argv[i]))
			stattistics_Time = atoi(argv[i+1]);
		else if(!strcmp("-wt",argv[i]))
			stationManager_WorkingTime = atoi(argv[i+1]);		
		else{			
			perror("Wrong input");
			exit(1);
		}
		i=i+2;
	}

	FILE * configfile = fopen("configfile.txt","w");
 	
	fprintf(configfile,"%d %d \n",numOfBayes,numOfBuses);
	for(int i=0;i<numOfBayes;i++){
		bayCapacity = rand()%maxCapacity + 1;
		bayesCapacity[i] = bayCapacity;
		fprintf(configfile,"%d ",bayCapacity);
	}fprintf(configfile,"\n");

	for(int i=0;i<numOfBuses;i++){

		manuverTime = rand()%manuverTime + 2;
		maxPassengers = rand()%maxPassengers + 5;
		maxParkTime = rand()%maxParkTime + 2;
		passengers =  rand()%maxPassengers + 5;
		
		fprintf(configfile,"%s %d %d %d %d\n",bayes[rand()%3],passengers,maxPassengers,maxParkTime,manuverTime);
	}

	fprintf(configfile,"%d %d\n",curent_Situation,stattistics_Time );
	
	fprintf(configfile,"%d\n",stationManager_WorkingTime );	
	fclose(configfile);
return 0;	
}