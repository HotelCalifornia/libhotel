FROM ubuntu:latest

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    wget \
    cmake \
    flex \
    bison \
    python3-dev \
    python3-pip \
    texlive-base \
    texlive \
    texlive-latex-extra \
    texlive-fonts-extra

RUN pip3 install jinja2 pygments wheel

RUN wget https://www.doxygen.nl/files/doxygen-1.9.3.src.tar.gz -O /tmp/doxygen-1.9.3.src.tar.gz
RUN tar -C /tmp -xzvf /tmp/doxygen-1.9.3.src.tar.gz

WORKDIR /tmp/doxygen-1.9.3

RUN mkdir build
WORKDIR /tmp/doxygen-1.9.3/build

RUN cmake -G "Unix Makefiles" ..
RUN make
RUN make install

WORKDIR /