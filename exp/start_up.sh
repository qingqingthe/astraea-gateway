#!/bin/sh
pkill aesm_service 
jhid 2>&1 & 
/opt/intel/libsgx-enclave-common/aesm/aesm_service &
/bin/bash
