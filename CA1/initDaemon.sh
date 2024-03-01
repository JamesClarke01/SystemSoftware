!#/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Invalid number of arguments\n"
    exit 1
fi

if ["$0" = "start"]; then
    echo "Starting daemon..."
elif ["$0" = "stop"]; then    
    echo "Stopping daemon..."
fi