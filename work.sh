#! /bin/bash

trap exit SIGINT

COUNTER=0

while true
do
    ((COUNTER+=1))
    echo "#${COUNTER} Working..."
    sleep 2
    
    if [ $COUNTER -eq 50 ]; then
        exit 0
    fi
done