FROM archlinux:latest

RUN mkdir /report

VOLUME /report

# Install cmaker
COPY cmaker /cmaker

#Copy index file
COPY index.json /index.json

WORKDIR /cmaker

# Install cmake
RUN pacman -Syu --noconfirm && pacman -S cmake --noconfirm
RUN pacman -S base-devel --noconfirm
RUN pacman -S git --noconfirm

#RUN git config --global init.defaultBranch deeznuts

# Install cmake-generator
RUN cmake -DCMAKE_BUILD_TYPE=Release ./ && make -j20 && make install

COPY . /compatibility-report

WORKDIR /compatibility-report



