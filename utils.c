#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int busArgs(char * input){

	if(!strcmp(input,"-t"))
		return TYPE;
	else if (!strcmp(input,"-n"))
		return NUM_OF_PASSENGERS;
	else if (!strcmp(input,"-c"))
		return CAPACITY;
	else if (!strcmp(input,"-p"))
		return PARKPERIOD;
	else if (!strcmp(input,"-m"))
		return MANTIME;
	else if (!strcmp(input,"-s"))
		return SHMID;
	else if (!strcmp(input,"-id"))
		return PID;
	else
		return INPUT_ERROR;

}

int comptrollerArgs(char * input){

	if(!strcmp(input,"-d"))
		return TIME;
	else if (!strcmp(input,"-t"))
		return STAT_TIMES;
	else if (!strcmp(input,"-s"))
		return SHMID;
	else
		return INPUT_ERROR;
}


int bay_hasSpace(sharedMemory_Data * smData,int * bay_Parked,int * position){

	
	// In shared segment I store one integer per position that it is 0 if empty and 1 if full



	sharedMemory_Data * s = smData; 	// pointer to shared segment 
	s = s + 1;		// so I move pointer +1 because if the struct
	int * temp = (int *) s;		// cast to int
	int one=1;	
	

	/* --------------- 3 types ------------------------*/

	switch(smData->current_busType[IN]){

		case ASK:

			if(smData->curCapacity_perBay[ASK] < smData->maxCapacity_perBay[ASK]){

				*bay_Parked = ASK;

				int * curPointer = temp;
				int pos = *curPointer;
				int counter=0;

				while(pos!=0 && counter<smData->maxCapacity_perBay[ASK]){		// loop to traverse untill finding a free position
					
					curPointer++;
					pos = *curPointer;
					counter++;
				}


				memcpy(curPointer,&one,sizeof(int));		// mark as unavailable
				*position = counter;


				smData->curCapacity_perBay[ASK]++;
				smData->available_parkingPosition[ASK]--;
		
				return true;
			
			}else if(smData->curCapacity_perBay[PEL] < smData->maxCapacity_perBay[PEL]){	// check if PEL has space

				temp = temp + smData->maxCapacity_perBay[ASK];

				*bay_Parked = PEL;

				int pos = *temp;
				int counter=0;

				while(pos!=0 && counter<smData->maxCapacity_perBay[PEL]){
					
					temp ++;
					pos = *temp;
					counter++;
				}
				memcpy(temp,&one,sizeof(int));
				*position = counter;

				smData->curCapacity_perBay[PEL]++;
				smData->available_parkingPosition[PEL]--;
				
				return true;
			}else
				return false;	

			break;
		
		case PEL:		// same
			
			temp += smData->maxCapacity_perBay[ASK];

			if(smData->curCapacity_perBay[PEL] < smData->maxCapacity_perBay[PEL]){

				*bay_Parked = PEL;

				int pos = *temp;
				int counter=0;

				while(pos!=0 && counter<smData->maxCapacity_perBay[PEL]){
					
					temp ++;
					pos = *temp;
					counter++;
				}
				memcpy(temp,&one,sizeof(int));
				*position = counter;

				smData->available_parkingPosition[PEL]--;
				smData->curCapacity_perBay[PEL]++;
				// printf("~~~~~~~~~~~~~~~~~~~~~PEL %d \n",smData->curCapacity_perBay[PEL] );
				return true;
			}else				
				return false;	
			

			break;

		case VOR:	// same

		
			if(smData->curCapacity_perBay[VOR] < smData->maxCapacity_perBay[VOR]){

				temp += smData->maxCapacity_perBay[ASK]+smData->maxCapacity_perBay[PEL];
				
				*bay_Parked = VOR;

				int * curPointer = temp;
				int pos = *curPointer;
				int counter=0;

				while(pos!=0 && counter<smData->maxCapacity_perBay[VOR]){
					
					curPointer ++;
					pos = *curPointer;
					counter++;
				}
				memcpy(curPointer,&one,sizeof(int));
				*position = counter;

				smData->curCapacity_perBay[VOR]++;
				smData->available_parkingPosition[VOR]--;

				return true;
			}else if(smData->curCapacity_perBay[PEL] < smData->maxCapacity_perBay[PEL]){

				temp += smData->maxCapacity_perBay[ASK];

				*bay_Parked = PEL;

				int pos = *temp;
				int counter=0;

				while(pos!=0 && counter<smData->maxCapacity_perBay[PEL]){
					
					temp ++;
					pos = *temp;
					counter++;
				}
				memcpy(temp,&one,sizeof(int));
				*position = counter;

				smData->curCapacity_perBay[PEL]++;
				smData->available_parkingPosition[PEL]--;

				return true;
				
			}else				
				return false;	
			break;

	}

	return ERROR;

}

void get_bayName(int bay,char * bayName){

	switch(bay){
		case ASK:
			strcpy(bayName,"ASK");
			break;
		case VOR:
			strcpy(bayName,"VOR");
			break;
		case PEL:
			strcpy(bayName,"PEL");
			break;
	}
}

int get_bayID(char * bayName){

	if(!strcmp("ASK",bayName))
		return ASK;
	else if(!strcmp("PEL",bayName))
		return PEL;
	else if(!strcmp("VOR",bayName))
		return VOR;
	else
		return ERROR;
}

int position_Available(sharedMemory_Data * smData,int position,int bay){	// function that marks available a position

	sharedMemory_Data * startOfPositions = smData;
	startOfPositions=startOfPositions+1;

	int * temp = (int *) startOfPositions;
	int counter=0;

	switch(bay){
		case ASK:
			break;
		case PEL:
			temp = temp + smData->maxCapacity_perBay[ASK];
			break;
		case VOR:
			temp = temp + smData->maxCapacity_perBay[PEL] + smData->maxCapacity_perBay[ASK];
			break;
	}

	while(counter<position){
		temp++;
		counter++;
	}


	memset(temp,0,sizeof(int));		// mark it as free
	smData->curCapacity_perBay[bay]--;	// inform counters
	smData->available_parkingPosition[bay]++;

	return true;

}

void print_toLedger(Position * p,FILE * ledger,int numOfBus){
	
	fprintf(ledger,"%d. Bus %d \n",numOfBus,p->busid);
	char bay[BAY_BUFFER];
	get_bayName(p->type,bay);
	fprintf(ledger,"\tType: %s \n",bay);
	char bayPark[BAY_BUFFER];
	get_bayName(p->bayParked,bayPark);
	fprintf(ledger,"\tBay parked: %s at position: %d \n",bayPark,p->position);
	fprintf(ledger,"\tLeaved %d passengers \n",p->passengers_toLeave);
	fprintf(ledger,"\tLoaded %d passengers \n",p->passengers_toLoad);
	fprintf(ledger,"\tArrival time: %s \n",p->timeOfArrival);
	fprintf(ledger,"\tDeparture time: %s \n\n",p->timeOfDeparture);
}