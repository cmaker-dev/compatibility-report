FROM archlinux:latest

RUN mkdir /report

VOLUME /report

# Install cmaker
COPY cmaker /cmake-generator

#Copy index file
COPY index.json /index.json

WORKDIR /cmake-generator

# Install cmake
RUN pacman -Syu --noconfirm && pacman -S cmake --noconfirm
RUN pacman -S base-devel --noconfirm
RUN pacman -S git --noconfirm

RUN git config --global init.defaultBranch deeznuts

# Install cmake-generator
RUN cmake -DCMAKE_BUILD_TYPE=Release ./ && make -j20 && make install

COPY compatibility-report /compatibility-report

WORKDIR /compatibility-report



