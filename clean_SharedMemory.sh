#!/bin/bash

m=`ipcs -m | grep -w 666 | cut -d' ' -f2`

for i in $m 
do
	echo Removing shm with id: $i
	ipcrm -m $i
done 


