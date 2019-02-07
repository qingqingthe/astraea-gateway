FROM ubuntu:16.04
#installing drivers
RUN apt-get update && apt-get install -y git make wget vim
RUN apt-get install -y build-essential ocaml automake autoconf libtool wget python libssl-dev gcc
RUN apt-get install -y libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper
RUN git clone https://github.com/intel/linux-sgx-driver.git
WORKDIR linux-sgx-driver
RUN git checkout tags/sgx_driver_1.9
RUN apt-get install -y linux-headers-$(uname -r)
RUN make
RUN apt-get install kmod
COPY lib/modules/ /lib
RUN rm -r /lib/modules/4.4.0-141-generic
RUN mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
RUN cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
RUN touch /etc/modules
RUN sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
RUN /sbin/depmod
RUN /sbin/modprobe isgx
WORKDIR ..
#building psw/sdk
RUN git clone https://github.com/intel/linux-sgx.git
WORKDIR linux-sgx
RUN git checkout tags/sgx_1.9
RUN ./download_prebuilt.sh
RUN make
RUN make sdk_install_pkg
RUN make psw_install_pkg
#RUN make deb_pkg
#installing sdk
WORKDIR ./linux/installer/bin
RUN yes yes | ./sgx_linux_x64_sdk_1.9.100.39124.bin
RUN echo "LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/opt/Intel/iclsClient/lib" >> ./sgxsdk/environment
###installing psw/platform services
WORKDIR ../../../..
ADD iclsclient_1.45.449.12-2_amd64.deb .
RUN apt-get -y install lib32gcc1 lib32stdc++6 
RUN dpkg -i iclsclient_1.45.449.12-2_amd64.deb
RUN apt install -y uuid-dev libxml2-dev cmake pkg-config libsystemd-dev
RUN git clone https://github.com/intel/dynamic-application-loader-host-interface.git
WORKDIR dynamic-application-loader-host-interface
RUN cmake .;make;make install
##sudo systemctl enable jhi
WORKDIR ../linux-sgx/linux/installer/bin
ADD libsgx-enclave-common_2.4.100.48163-1_amd64.deb .
ADD libsgx-urts_2.4.100.48163-1_amd64.deb .
ENV LD_LIBRARY_PATH=/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/opt/Intel/iclsClient/lib
ADD ./AMCS/monotonic /monotonic
RUN ./sgx_linux_x64_psw_1.9.100.39124.bin
RUN LD_LIBRARY_PATH=/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/opt/Intel/iclsClient/lib dpkg -i libsgx-urts_2.4.100.48163-1_amd64.deb libsgx-enclave-common_2.4.100.48163-1_amd64.deb
WORKDIR ../../../../
#RUN wget https://software.intel.com/sites/default/files/m/b/2/9/0/9/37961-mei_7.1.20.25.zip
#RUN apt-get install -y unzip alien
#RUN unzip 37961-mei_7.1.20.25.zip
#WORKDIR ./outputdir
#RUN alien mei-7.1.20-25.x86_64.rpm
#RUN dpkg -i mei_7.1.20-26_amd64.deb
ADD start_up.sh /
RUN chmod 755 /start_up.sh 
ENTRYPOINT ["/start_up.sh"]
#CMD ./sgx_linux_x64_psw_1.9.100.39124.bin && /bin/bash -c "jhid 2>&1 &" && /bin/bash && /bin/bash -c "service aesmd start"

#CMD /bin/bash
##
