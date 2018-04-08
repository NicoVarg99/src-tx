#!/bin/bash

#Generate audio
./src-tx

echo "Waiting for second '50' to start playing..."

#Sleep until next 50th second
#sleep $(( $((60 + $((60 - $(date +%S) - 10)) )) % 60 ))

#Play audio
aplay ./src.wav -q &

sleep 2

echo "SIG1"
sleep 1

echo "SIG2"
sleep 1

echo -n "==="
sleep 1

echo -n "==="
sleep 1

echo -n "==="
sleep 1

echo -n "==="
sleep 1

echo -n "==="
sleep 2

echo -ne "\n* SYNC *   "
date
