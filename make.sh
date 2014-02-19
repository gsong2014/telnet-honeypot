#!/bin/bash
g++ serveur.cpp telnetd.cpp main.cpp -o telnetd -lpthread
g++ serveur.cpp smtpd.cpp main2.cpp -o smtpd -lpthread
mkdir chroot
