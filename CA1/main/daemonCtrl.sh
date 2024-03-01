#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Invalid number of arguments"
    exit 1
fi

if [ "$1" = "start" ]; then
    echo "Making daemon" 
    make  

    echo "Running daemon"
    ./facDaemon  

    echo "Cleaning executable"
    rm -f facDaemon
elif [ "$1" = "stop" ]; then    
    echo "Stopping daemon"
    cat facDaemon.pid | xargs kill -TERM

    echo "Removing pid file"
    rm -f facDaemon.pid
fi