FROM ubuntu:focal
#FROM conda/miniconda3:latest
#FROM kitware/cmake:ci-debian10-aarch64-2022-08-30

ARG DEBIAN_FRONTEND=noninteractive

ENV PATH="/root/miniconda3/bin:${PATH}"
ARG PATH="/root/miniconda3/bin:${PATH}"
RUN apt-get update

RUN apt-get install -y wget && rm -rf /var/lib/apt/lists/*

RUN wget \
    https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
    && mkdir /root/.conda \
    && bash Miniconda3-latest-Linux-x86_64.sh -b \
    && rm -f Miniconda3-latest-Linux-x86_64.sh 
RUN conda --version

COPY . /root/task_planner

RUN apt-get update && apt purge --auto-remove cmake \
    && apt install -y gpg \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
    && apt-get install -y software-properties-common \
    && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main' \
    && apt update \
    && apt install -y cmake

RUN apt-get update && apt-get install -y build-essential && apt-get -y install cmake
RUN conda init bash
RUN conda create --name tpenv python=3.8
RUN conda install -n tpenv -c conda-forge spot
RUN /root/task_planner/python_scripts_up.sh

# Make the build directory if it doesnt exist, then remove it to make sure a no previous build files exist:
RUN mkdir -p /root/task_planner/build && rm -r /root/task_planner/build
RUN mkdir -p /root/task_planner/benchmark/benchmark_data && mkdir /root/task_planner/build && mkdir -p /root/task_planner/spot_automaton_file_dump/dfas
#RUN cmake -S /root/task_planner -B /root/task_planner/build && make --directory /root/task_planner/build
RUN cd /root/task_planner/build && cmake ./.. && make