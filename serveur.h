#ifndef __servh__
#define __servh__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <pthread.h>

#define TCP 1
#define UDP 2
#define default_port 2222
#define default_type TCP
#define linesize 256
#define streamsize 256

using namespace std;

class serveur
{
	private :
		static void* beginconnected(void*);
		static void* beginreceived(void*);
	
	protected :
		int port;
		struct sockaddr_in addr_serv;
		int sockd;
		int protocol;
		void *param;
		int isStarted;
		char EndStream[streamsize];

	public :
	
		serveur();
		int init();
		int start();
		void stop();
		void dispose();
		
		void setProtocol(int);
		int getProtocol();
		
		void setPort(int);
		int getPort();
		
		void setParam(void *arg);
		
		void setEndStream(char*);
		void getEndStream(char*);
		
		int readline(int sock, unsigned char *buf);
		int sread(int sock, unsigned char *buf, int limit);
		
		virtual void connected(int sock, struct sockaddr_in info_tcp);
		virtual void received(int sock, char *msg, struct sockaddr_in info_udp);
		// inet_ntoa(info_tcp.sin_addr)
		// ntohs(info_tcp.sin_port)
};

struct arg_udp
{
	serveur *m_serv;
	struct sockaddr_in info_udp;
	char msg[256];
};

struct arg_tcp
{
	serveur *m_serv;
	int sock;
	struct sockaddr_in info_tcp;
};
#endif
