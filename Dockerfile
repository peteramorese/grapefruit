FROM ubuntu:jammy

#

ARG DEBIAN_FRONTEND=noninteractive 

RUN apt-get -y update && apt -y install \
    gcc git \
    wget vim curl \
    python3-pip cmake automake \
    build-essential \
    apt-utils flex bison mona \
    libeigen3-dev

# installing spot using apt
RUN wget -q -O - https://www.lrde.epita.fr/repo/debian.gpg | apt-key add -

SHELL ["/bin/bash", "-c"] 

RUN pip3 install pyyaml numpy bidict networkx graphviz ply pybullet pyperplan==1.3 cython IPython svgwrite matplotlib imageio lark-parser==0.9.0 sympy==1.6.1

RUN echo 'deb http://www.lrde.epita.fr/repo/debian/ stable/' >> /etc/apt/sources.list

RUN apt-get -y update && apt -y install spot \
    libspot-dev \
    spot-doc
#

COPY . /root/task_planner

#RUN apt-get update && apt purge --auto-remove cmake \
#    && apt install -y gpg \
#    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
#    && apt-get install -y software-properties-common \
#    && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main' \
#    && apt update \
#    && apt install -y cmake
#
#RUN apt-get update && apt-get install -y build-essential && apt-get -y install cmake

# Make the build directory if it doesnt exist, then remove it to make sure a no previous build files exist:
RUN mkdir -p /root/task_planner/build && rm -r /root/task_planner/build
RUN mkdir /root/task_planner/build 
RUN cd /root/task_planner/build && cmake ./.. && make

