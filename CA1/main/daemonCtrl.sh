#!/bin/bash

stopDaemon() {
    if [ -e "facDaemon.pid" ]; then
        echo "Stopping daemon"
        cat facDaemon.pid | xargs kill -TERM

        echo "Removing pid file"
        rm -f facDaemon.pid

        echo "Daemon is now stopped"
    else 
        echo "Daemon not currently running"
    fi
}

startDaemon() {
    #Stop daemon if already running
    
    stopDaemon
    
    echo "Making daemon" 
    make  

    echo "Running daemon"
    ./facDaemon  

    echo "Cleaning executable"
    make removeObjects
    
    echo "Daemon is now started"
}

getDaemonStatus() {
    if [ -e "facDaemon.pid" ]; then
        echo "Daemon is running"
    else 
        echo "Deamon is not running"
    fi
}

sendTransferSignal() {
    kill -SIGUSR1 $(cat facDaemon.pid)
}

sendBackupSignal() {
    kill -SIGUSR2 $(cat facDaemon.pid)
}

sendRestartSignal() {
    kill -SIGHUP $(cat facDaemon.pid)
}

if [ "$#" -lt 1 ]; then
    echo "Invalid number of arguments"
    exit 1
fi

if [ "$1" = "start" ]; then
    startDaemon
elif [ "$1" = "stop" ]; then    
    stopDaemon
elif [ "$1" = "status" ]; then
    getDaemonStatus
elif [ "$1" = "transfer" ]; then
    sendTransferSignal
elif [ "$1" = "backup" ]; then
    sendBackupSignal
elif [ "$1" = "restart" ]; then
    sendRestartSignal
fi




