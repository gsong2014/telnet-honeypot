#!/bin/bash
g++ server.cpp telnetd.cpp main.cpp -o telnetd -lpthread
mkdir -p chroot
