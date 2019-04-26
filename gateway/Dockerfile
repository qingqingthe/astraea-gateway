FROM silicon_base
MAINTAINER Dana Kuban

RUN apk update
RUN apk del openssl-dev
RUN apk add libressl-dev
#RUN apk add mariadb-client mariadb-dev
RUN apk update && apk add mysql-client gcc make cmake curl-dev musl-dev libstdc++ sed
RUN apk add mariadb-dev
#RUN git clone https://github.com/MariaDB/mariadb-connector-c.git
#WORKDIR mariadb-connector-c
#RUN mkdir build
#WORKDIR build
#RUN cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
#ENV PATH="/usr/local/:${PATH}"
#RUN make
#WORKDIR /
#RUN git clone https://github.com/mysql/mysql-connector-cpp.git
#WORKDIR mysql-connector-cpp
##ugly hack incoming. Brace yourselves;
#ADD endian.h /usr/include/sys
#RUN cp /usr/include/asm/byteorder.h /usr/include/sys
#ADD remove_tests.sh /mysql-connector-cpp
##RUN rm /usr/lib/libssl*
#RUN sh remove_tests.sh
#RUN apk add linux-headers
#RUN mkdir build
#WORKDIR build
#RUN git submodule update --init
##RUN apk add openssl
#RUN git clone https://github.com/wolfSSL/wolfssl.git
#RUN sed -i "s/MYCPPCONN_STATIC_MYSQL_VERSION_ID  @MYSQL_NUM_VERSION@/ MYCPPCONN_STATIC_MYSQL_VERSION_ID  80000/" /mysql-connector-cpp/jdbc/cppconn/version_info.h.cmake
#RUN cmake ../ -DWITH_SSL=/mysql-connector-cpp/build/wolfssl
##RUN sed -i "325,337d" /mysql-connector-cpp/jdbc/driver/nativeapi/libmysql_static_proxy.cpp
##RUN sed -i "96,118d" /mysql-connector-cpp/jdbc/driver/nativeapi/mysql_native_connection_wrapper.cpp
##RUN sed -i "s/ MYCPPCONN_STATIC_MYSQL_VERSION_ID  100138/ MYCPPCONN_STATIC_MYSQL_VERSION_ID  5000/" /mysql-connector-cpp/build/jdbc/cppconn/version_info.h
##RUN sed -i "s/ > / < /" /mysql-connector-cpp/jdbc/driver/nativeapi/native_statement_wrapper.h
##RUN sed -i "s/ > / < /" /mysql-connector-cpp/jdbc/driver/nativeapi/native_resultset_wrapper.h
##RUN sed -i "s/ > 80004/ < 80004/" /mysql-connector-cpp/jdbc/driver/mysql_resultset_metadata.h
##RUN sed -i "s/ >= 50/ <= 50/g" /mysql-connector-cpp/jdbc/driver/nativeapi/libmysql_static_proxy.cpp
##RUN sed -i "s/ > 80004/ < 80004/" /mysql-connector-cpp/jdbc/driver/nativeapi/mysql_native_connection_wrapper.h
##RUN sed -i "s/ >= 80000/ <= 80000/" /mysql-connector-cpp/jdbc/driver/mysql_connection.cpp
##RUN sed -i "s/ >= 80000/ <= 80000/" /mysql-connector-cpp/jdbc/driver/nativeapi/mysql_native_connection_wrapper.cpp
##RUN sed -i "s/ >= 80000/ <= 80000/" /mysql-connector-cpp/jdbc/driver/mysql_connection_options.h
#
#
#RUN make
#RUN make install
#
#ENV LIBRARY_PATH=$LIBRARY_PATH:/usr/local/mysql/lib64/debug
RUN echo "http://dl-cdn.alpinelinux.org/alpine/v3.8/main/" >> /etc/apk/repositories
#RUN echo "@edge http://nl.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories
RUN apk update
#RUN apk add mariadb-connector-c@edge
RUN apk add mariadb-connector-c

#WORKDIR /

# add gateway.cpp
ADD gateway.cpp http_request_helper.h cJSON.c cJSON.h /

# generate symbols.hh
RUN /bin/iod_generate_symbols gateway.cpp symbols_gateway.hh


#removing problematic symbolds
RUN sed -i "3,7d" symbols_gateway.hh

#moving shared library files to lib directory (tracked by path)
#RUN cp /usr/local/mysql/connector-c++-8.0/lib64/debug/* /usr/lib

RUN ln -s /usr/lib/libmariadb.so.3 /usr/lib/libmariadb.so
# compile app
RUN g++ -std=c++14 -I /include -I /usr/local/mysql/connector-c++-8.0/include gateway.cpp cJSON.c -lmariadb -lmicrohttpd -lcurl -pthread -o gateway

CMD /bin/sh -c "SCONE_HEAP=1024M ./gateway"
#CMD /bin/sh

#RUN mkdir /data

# create script for entering container -> starting the app
#RUN echo "/gateway" > run_example.sh && chmod +x run_example.sh

EXPOSE 4000

#ENTRYPOINT ["sh", "-c", "/run_example.sh"]
#CMD tail -f /dev/null
