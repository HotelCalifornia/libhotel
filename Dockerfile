FROM ubuntu:latest

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    doxygen \
    python3-dev \
    python3-pip \
    texlive-base \
    texlive \
    texlive-latex-extra \
    texlive-fonts-extra

RUN pip3 install jinja2 pygments wheel

WORKDIR /