FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    git \
    doxygen \
    python3-dev

WORKDIR /