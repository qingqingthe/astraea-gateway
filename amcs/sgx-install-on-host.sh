#!/bin/bash
set -e
#installing drivers
debug_mode=false
light_debug=false
for i in "$@"
do
case $i in
    -d)
         debug_mode=true
     ;;
    -l)
         light_debug=true
     ;;
esac
done
if [[ -z $(git --version) ]]; then
    apt install git
fi
#check if driver is installed
if [ ! -d ./linux-sgx-driver ]; then
    git clone https://github.com/intel/linux-sgx-driver.git
fi
sudo apt-get install -y linux-headers-$(uname -r)
if $debug_mode; then
    echo "[DEBUG] making driver binaries"
fi
cd linux-sgx-driver
sudo make
sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
sudo /sbin/depmod
sudo /sbin/modprobe isgx
cd ..
#building psw/sdk
if $debug_mode; then
    echo "[DEBUG] installing dependencies for psw/sdk"
fi
sudo apt-get install -y build-essential ocaml automake autoconf libtool wget python libssl-dev
sudo apt-get install -y libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper
if [ ! -d ./linux-sgx ]; then
    git clone https://github.com/intel/linux-sgx.git
fi
cd linux-sgx
if $debug_mode; then
    echo "[DEBUG] download prebuilt"; read
fi
sudo ./download_prebuilt.sh
if $debug_mode; then
    echo "[DEBUG] making"; read
fi
sudo make
if $debug_mode; then
    echo "[DEBUG] making install sdk pkg"; read
fi
sudo make sdk_install_pkg
if $debug_mode; then
    echo "[DEBUG] making deb pkg"; read
fi
sudo make deb_pkg
#installing sdk
cd linux/installer/bin
if $debug_mode; then
    echo "[DEBUG] running sgx sdk bin"; read
fi
if [ $(ls -l linux-sgx/linux/installer/ | grep bin | cut -d ' ' -f 3) != $USER ]; then 
   if $debug_mode; then
       echo "[DEBUG] Changing directory owner..." 
   fi
   nosudo=$USER                                                     
   sudo chown $nosudo linux-sgx/linux/installer/bin
fi
yes yes | ./sgx_linux_x64_sdk_2.4.100.48163.bin
source /home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/environment
echo $LD_LIBRARY_PATH
echo "LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/opt/Intel/iclsClient/lib" >> /home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/environment
source /home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/environment
echo $LD_LIBRARY_PATH
if $light_debug; then
    echo "[DEBUG] check on LD_LIBRARY_PATH"; read
fi
#installing psw/platform services
cd ../../../..
if $debug_mode; then
    echo "[DEBUG] dpkg'ing iclsclient"; read
fi
sudo dpkg -i iclsclient_1.45.449.12-2_amd64.deb
sudo apt install -y uuid-dev libxml2-dev cmake pkg-config libsystemd-dev
if [ ! -d ./dynamic-application-loader-host-interface ]; then
    git clone https://github.com/intel/dynamic-application-loader-host-interface.git
fi
cd dynamic-application-loader-host-interface
if $debug_mode; then
    echo "[DEBUG] jhi making"; read
fi
cmake .;make;sudo make install
echo "[DEBUG] jhi debug done"
if [ -z "$(pidof /usr/sbin/jhid)" ]; then
    echo "[DEBUG] jhi pid is "$(pidof /usr/sbin/jhid)
    sudo jhid 2>&1 &
fi
#sudo systemctl enable jhi
cd ..
cd linux-sgx/linux/installer/deb
if $light_debug; then
    echo "[DEBUG] dpkg'ing libsgx"; read
    echo $LD_LIBRARY_PATH
fi

sudo LD_LIBRARY_PATH=/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/home/ubuntu/sgx-install/linux-sgx/linux/installer/bin/sgxsdk/sdk_libs:/opt/Intel/iclsClient/lib dpkg -i libsgx-urts_2.4.100.48163-xenial1_amd64.deb libsgx-enclave-common_2.4.100.48163-xenial1_amd64.deb
if $light_debug; then
    echo "[DEBUG] after dpkg"; read
    echo $LD_LIBRARY_PATH
fi

if $light_debug; then
    echo "[DEBUG] start aesmd"; read
fi
sudo service aesmd start



