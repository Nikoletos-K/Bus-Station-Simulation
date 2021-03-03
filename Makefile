# in order to execute this "Makefile" just type "make"

OBJS   = mystation.o bus.o utils.o station-manager.o comptroller.o create_configFile.o
SOURCE = mystation.c bus.c utils.c station-manager.c comptroller.c create_configFile.c
HEADER = utils.h
OUT    = mystation bus station-manager create_configFile clean_sharedMemory comptroller
CC     = gcc
FLAGS  = -g -c

all: mystation bus station-manager comptroller create_configFile

mystation: mystation.o utils.o
	$(CC) -g mystation.o utils.o -o mystation -lpthread 

station-manager: station-manager.o utils.o
	$(CC) -g station-manager.o utils.o -o station-manager -lpthread

bus: bus.o utils.o
	$(CC) -g bus.o utils.o -o bus -lpthread

comptroller: comptroller.o utils.o
	$(CC) -g comptroller.o utils.o -o comptroller -lpthread


create_configFile: create_configFile.o
	$(CC) -g create_configFile.o -o create_configFile


# create/compile the individual files >>seperetaly<<

mystation.o: mystation.c
	$(CC) $(FLAGS)  mystation.c 

bus.o: bus.c
	$(CC) $(FLAGS) bus.c

station-manager.o: station-manager.c
	$(CC) $(FLAGS) station-manager.c

comptroller.o: comptroller.c
	$(CC) $(FLAGS) comptroller.c

utils.o: utils.c
	$(CC) $(FLAGS) utils.c

create_configFile.o: create_configFile.c
	$(CC) $(FLAGS) create_configFile.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

# clear shared memory if something failed
clean_sharedMemory:
	./clean_SharedMemory.sh

list_OfSharedMemory:
	ipcs -m
