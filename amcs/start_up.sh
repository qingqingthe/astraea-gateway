#!/bin/sh
#pkill aesm_service 
jhid 2>&1 & 
sleep 5
/opt/intel/libsgx-enclave-common/aesm/aesm_service &
sleep 5
/bin/bash
