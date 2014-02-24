#!/bin/bash
g++ serveur.cpp telnetd.cpp main.cpp -o telnetd -lpthread
mkdir -p chroot
