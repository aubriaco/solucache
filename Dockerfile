FROM alpine:latest AS compile-image
RUN apk add libressl-dev make cmake g++ libstdc++ git
ARG CPU_COUNT
RUN git clone https://github.com/weidai11/cryptopp.git && cd cryptopp && make -j${CPU_COUNT} && make install && ln -s /usr/local/lib/libcryptopp.a /usr/local/lib/libcrypto++.a && ln -s /usr/local/include/cryptopp/ /usr/local/include/crypto++
RUN git clone https://github.com/aubriaco/solusek.git && mkdir solusek/build && cd solusek/build && cmake ../src && make -j${CPU_COUNT} && make install

RUN mkdir /solucache
COPY src/ /solucache/src/
RUN mkdir /solucache/build && cd /solucache/build && cmake ../src && make -j${CPU_COUNT}

FROM alpine:latest AS runtime-image

RUN apk add libressl libstdc++


COPY --from=compile-image /usr/local/lib/libsolusek.so /usr/local/lib/
RUN mkdir /opt/solucache
COPY --from=compile-image /solucache/build/solucache /opt/solucache/bin/solucache

WORKDIR /opt/solucache/bin

ENTRYPOINT ["./solucache"]
