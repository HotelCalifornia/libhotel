FROM hotelcalifornia/doxygen-base:1.0.2

WORKDIR /usr/src/docs

RUN ls -la .

RUN ./build-docs