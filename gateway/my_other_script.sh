#!/bin/bash

for (( i=0; i<=10; i++ ))
do
    curl -G -X POST "127.0.0.1:4000/read_request_from_client" --data-urlencode 'request=1' --data-urlencode 'ip_client=client:12346'
done
