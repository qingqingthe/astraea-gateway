#!/bin/bash

sudo apt-get update && sudo apt-get install -yq --no-install-recommends ca-certificates build-essential ocaml automake autoconf libtool wget python libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev alien cmake uuid-dev libxml2-dev 
#pkg-config
wget -O - https://github.com/01org/linux-sgx-driver/archive/sgx_driver_2.1.tar.gz | tar -xz && \
    cd linux-sgx-driver-sgx_driver_2.1 && \
    make && \
    sudo insmod isgx.ko && \
    rm -rf linux-sgx-driver-sgx_driver_2.1
wget --progress=dot:mega -O iclsclient.rpm http://registrationcenter-download.intel.com/akdlm/irc_nas/11414/iclsClient-1.45.449.12-1.x86_64.rpm
sudo alien --scripts -i iclsclient.rpm
wget --progress=dot:mega -O - https://github.com/01org/dynamic-application-loader-host-interface/archive/7b49da96ee2395909d867234c937c7726550c82f.tar.gz | tar -xz
cd dynamic-application-loader-host-interface-7b49da96ee2395909d867234c937c7726550c82f && \
    cmake . -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    sudo make install && \
    cd .. && rm -rf dynamic-application-loader-host-interface-7b49da96ee2395909d867234c937c7726550c82f
wget --progress=dot:mega -O - https://github.com/01org/linux-sgx/archive/sgx_2.3.1.tar.gz | tar -xz && \
    cd linux-sgx-sgx_2.3.1 && \
    ./download_prebuilt.sh 2> /dev/null && \
    make -s -j$(nproc) sdk_install_pkg psw_install_pkg && \
    sudo ./linux/installer/bin/sgx_linux_x64_sdk_2.3.1*.bin --prefix=/opt/intel && \
    sudo ./linux/installer/bin/sgx_linux_x64_psw_2.3.1*.bin && \
    cd .. && rm -rf linux-sgx-sgx_2.3.1
