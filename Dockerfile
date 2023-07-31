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

# Install spot
RUN apt-get -y update && apt -y install spot \
    libspot-dev \
    spot-doc

WORKDIR /root/grapefruit

# Make the build directory if it doesnt exist, then remove it to make sure a no previous build files exist:
RUN mkdir -p build && rm -r build
RUN mkdir build 
#RUN cd build && cmake ./.. && make

