FROM silicon_base
MAINTAINER Dana Kuban

# add client.cpp
ADD client.cpp http_request_helper.h cJSON.c cJSON.h /

# generate symbols.hh
RUN /bin/iod_generate_symbols client.cpp symbols_client.hh

# compile app
RUN clang++ -std=c++14 -I /include client.cpp cJSON.c -lmicrohttpd -lcurl -pthread -o client

RUN mkdir /received

# create script for entering container -> starting the app
RUN echo "/client" > run_example.sh && chmod +x run_example.sh

EXPOSE 6000

CMD /bin/sh -c "/client"


#ENTRYPOINT ["sh", "-c", "/run_example.sh"]
#CMD /bin/sh
#CMD tail -f /dev/null
