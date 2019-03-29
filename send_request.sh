#!/bin/bash
enumerate=$1
no_write=false
if [[ $enumerate -eq -1 ]]; then
   no_write=true
fi
if [[ $enumerate -lt 10 ]]; then
    enumerate="0$enumerate"
fi

if [[ $enumerate -lt 100 ]]; then
    enumerate="0$enumerate"
fi
m=1000000
time1=$(($(date +%s%N/$m)))
#curl -o /dev/null -H "Content-Type: application/json" -X POST -d '{"user":"Notorious)","query":"SELECT * FROM Table"}' localhost:5000/write
#req="{'user': 'Notorious', 'query': 'SELECT * FROM TABLE'}"
#req="%7B%27user%27%3A+%27Notorious%27%2C+%27query%27%3A+%27SELECT+%2A+FROM+TABLE%27%7D"
req="SELECT+%2A+FROM+SomeTable"
#ip="client:6000"
ip="client%3A6000"
curl -X POST "localhost:4000/read_request_from_client?request=$req&ip_client=$ip"
#time2=$(($(date +%s%N/$m)))
#latency=$(($time2 - $time1))
if ! $no_write ; then
    echo $enumerate","$time1 >> "./latencies/$enumerate"
fi



