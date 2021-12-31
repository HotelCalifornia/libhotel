FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    git \
    doxygen \
    python3-dev \
    python3-pip

RUN pip3 install jinja2 pygments wheel

WORKDIR /