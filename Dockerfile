#!/bin/bash
set -e
#installing drivers
RUN git clone https://github.com/intel/linux-sgx-driver.git
RUN sudo apt-get install -y linux-headers-$(uname -r)
WORKDIR linux-sgx-driver
RUN make
RUN mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
RUN cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
RUN sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
RUN /sbin/depmod
RUN /sbin/modprobe isgx
WORKDIR ..
#building psw/sdk
RUN apt-get install -y build-essential ocaml automake autoconf libtool wget python libssl-dev
RUN apt-get install -y libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper
RUN git clone https://github.com/intel/linux-sgx.git
WORKDIR linux-sgx
RUN ./download_prebuilt.sh
RUN make
RUN make sdk_install_pkg
RUN make deb_pkg
#installing sdk
WORKDIR ./linux/installer/bin
#if [ $(ls -l linux-sgx/linux/installer/ | grep bin | cut -d ' ' -f 3) != $USER ]; then 
#   if $debug_mode; then
#       echo "[DEBUG] Changing directory owner..." 
#   fi
#   nosudo=$USER                                                     
#   sudo chown $nosudo linux-sgx/linux/installer/bin
#fi
RUN yes yes | ./sgx_linux_x64_sdk_2.4.100.48163.bin
echo "LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/opt/Intel/iclsClient/lib" >> ./sgxsdk/environment
#installing psw/platform services
WORKDIR ../../../..
RUN dpkg -i iclsclient_1.45.449.12-2_amd64.deb
RUN apt install -y uuid-dev libxml2-dev cmake pkg-config libsystemd-dev
RUN git clone https://github.com/intel/dynamic-application-loader-host-interface.git
WORKDIR dynamic-application-loader-host-interface
RUN cmake .;make;sudo make install
#sudo systemctl enable jhi
WORKDIR linux-sgx/linux/installer/deb
RUN LD_LIBRARY_PATH=/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/opt/Intel/iclsClient/lib dpkg -i libsgx-urts_2.4.100.48163-xenial1_amd64.deb libsgx-enclave-common_2.4.100.48163-xenial1_amd64.deb
CMD /bin/bash -c "sudo jhid 2>&1 &" && /bin/bash -c "service aesmd start"
