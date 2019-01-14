FROM ubuntu:16.04

RUN apt-get update && apt-get install -y ca-certificates build-essential ocaml automake autoconf libtool wget python libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev alien cmake uuid-dev libxml2-dev pkg-config
RUN wget https://github.com/01org/linux-sgx-driver/archive/sgx_driver_2.1.tar.gz
RUN tar -xzf sgx_driver_2.1.tar.gz
WORKDIR linux-sgx-driver-sgx_driver_2.1
RUN apt-get update && apt-get install -y linux-headers-$(uname -r) kmod make
RUN make
RUN cp isgx.ko /lib/modules/4.4.0-140-generic
#RUN modprobe isgx.ko
#RUN rm -rf linux-sgx-driver-sgx_driver_2.1
#RUN wget --progress=dot:mega -O iclsclient.rpm http://registrationcenter-download.intel.com/akdlm/irc_nas/11414/iclsClient-1.45.449.12-1.x86_64.rpm
#RUN alien --scripts -i iclsclient.rpm
#RUN wget --progress=dot:mega -O - https://github.com/01org/dynamic-application-loader-host-interface/archive/7b49da96ee2395909d867234c937c7726550c82f.tar.gz
#RUN tar -xz dynamic-application-loader-host-interface-7b49da96ee2395909d867234c937c7726550c82f.tar.gz
#WORKDIR dynamic-application-loader-host-interface-7b49da96ee2395909d867234c937c7726550c82f
#RUN cmake . -DCMAKE_BUILD_TYPE=Release
#RUN make -j$(nproc)
#RUN make install
#WORKDIR .. 
#RUN rm -rf dynamic-application-loader-host-interface-7b49da96ee2395909d867234c937c7726550c82f
#RUN wget --progress=dot:mega -O - https://github.com/01org/linux-sgx/archive/sgx_2.3.1.tar.gz
#RUN tar -xz sgx_2.3.1.tar.gz
#WORKDIR linux-sgx-sgx_2.3.1
#RUN ./download_prebuilt.sh 2> /dev/null
#RUN make -s -j$(nproc) sdk_install_pkg psw_install_pkg
#RUN ./linux/installer/bin/sgx_linux_x64_sdk_2.3.1*.bin --prefix=/opt/intel 
#RUN ./linux/installer/bin/sgx_linux_x64_psw_2.3.1*.bin
#WORKDIR .. 
#RUN rm -rf linux-sgx-sgx_2.3.1
