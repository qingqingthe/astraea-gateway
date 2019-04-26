FROM sconecuratedimages/osdi18:sconecli-alpine3.7
MAINTAINER Dana Kuban

#RUN echo "http://dl-cdn.alpinelinux.org/alpine/v3.9/main/" >> /etc/apk/repositories
#RUN echo "http://dl-cdn.alpinelinux.org/alpine/v3.9/community/" >> /etc/apk/repositories
#RUN echo "@edge http://nl.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories
#RUN echo "@edgecommunity http://nl.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories
#RUN echo "@testing http://nl.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories
#RUN apk update

#RUN apk add apk-tools@edge
RUN apk update \
    && apk upgrade \
    && apk add g++ \
	ca-certificates wget \
	git \
	cmake \
	make \
	libmicrohttpd-dev \
	boost-dev \
	nano \
	clang \
	curl-dev \
	curl \
    openssl-dev \
    jsoncpp-dev



	
# clone silicon
RUN git clone https://github.com/matt-42/silicon.git
RUN  sed -i "s|cmake\ \.\.\ -DCMAKE_PREFIX_PATH\=\$1\ -DCMAKE_INSTALL_PREFIX\=\$1|cmake\ \.\.\ -G\"Unix\ Makefiles\"\ -DCMAKE_PREFIX_PATH\=\$1\ -DCMAKE_INSTALL_PREFIX\=\$1|g" /silicon/install.sh
# install silicon
RUN cd silicon && ls && cat install.sh && ./install.sh /

# install websocketpp
RUN git clone https://github.com/zaphoyd/websocketpp.git
RUN cd websocketpp && cmake . && make install

CMD tail -f /dev/null
