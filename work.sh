#! /bin/bash

COUNTER=0

while true
do
    ((COUNTER+=1))
    echo "#${COUNTER} Working..."
    sleep 1

    if [ $COUNTER -eq 20 ]; then
        exit 0
    fi
done