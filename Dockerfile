FROM alpine:latest AS compile-image
RUN apk add libressl-dev make cmake g++ libstdc++ git
ARG CPU_COUNT
RUN git clone https://github.com/aubriaco/libsolunet.git && mkdir libsolunet/build && cd libsolunet/build && cmake .. && make -j && make install

RUN mkdir /solucache
COPY src/ /solucache/src/
RUN mkdir /solucache/build && cd /solucache/build && cmake ../src && make

FROM alpine:latest AS runtime-image

RUN apk add libressl libstdc++


COPY --from=compile-image /usr/local/lib/libsolunet.so /usr/local/lib/
RUN mkdir /opt/solucache
COPY --from=compile-image /solucache/build/solucache /opt/solucache/bin/solucache

WORKDIR /opt/solucache/bin

ENTRYPOINT ["./solucache"]
