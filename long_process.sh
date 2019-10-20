#! /bin/bash

COUNTER=0

while true
do
    ((COUNTER+=1))
    echo "#${COUNTER} Working..."
    sleep 2
done