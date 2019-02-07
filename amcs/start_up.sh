#!/bin/sh
#pkill aesm_service 
jhid 2>&1 & 
echo "[DEBUG] entering sleep"
#sleep 5
/opt/intel/libsgx-enclave-common/aesm/aesm_service &
sleep 2 
cd /monotonic/bin/
result=2
while [ $result -eq 2 ]
do
    eval ./amcs
    result=$?
    sleep 1
done
/bin/bash
while true; do sleep 10; done
