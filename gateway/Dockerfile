FROM silicon_base
MAINTAINER Dana Kuban


RUN apk del openssl-dev
RUN apk add libressl-dev
RUN apk add mariadb-dev
RUN apk update && apk add mysql-client gcc make cmake curl-dev openssl-dev

RUN git clone https://github.com/MariaDB/mariadb-connector-c.git

WORKDIR mariadb-connector-c
RUN mkdir build
RUN cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
RUN make


#RUN git clone https://github.com/mysql/mysql-connector-cpp.git
#WORKDIR mysql-connector-cpp
#ugly hack incoming. Brace yourselves;
#ADD endian.h /sys
#ADD byteorder.h /sys

#RUN cmake .
#RUN make clean && make && make install


# add gateway.cpp
ADD gateway.cpp http_request_helper.h cJSON.c cJSON.h /

# generate symbols.hh
RUN /bin/iod_generate_symbols gateway.cpp symbols_gateway.hh

# compile app
RUN clang++ -std=c++14 -I /include gateway.cpp cJSON.c -lmicrohttpd -lcurl -pthread -o gateway


RUN mkdir /data

# create script for entering container -> starting the app
RUN echo "/gateway" > run_example.sh && chmod +x run_example.sh

EXPOSE 4000

ENTRYPOINT ["sh", "-c", "/run_example.sh"]
#CMD tail -f /dev/null
