#!/bin/bash

if [ -z "$1" ]; then
    echo -e "Error: Target FQDN or IP-address must be provided!"
    exit 1
fi

URL="http://$1/temperatures"

for i in $(seq 1 500); do
    echo "Request #$i"
    curl -s "$URL"
    echo -e "\n"
    sleep 2
done
