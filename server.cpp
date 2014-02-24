#ifndef __servcpp__
#define __servcpp__

#include "server.h"

int server::init()
{
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	addr_serv.sin_addr.s_addr = INADDR_ANY;
	isStarted = 0;
	return 1;
}

void server::dispose()
{
	delete(this);
}

server::server()
{
	port = default_port;
	protocol = default_type;
	isStarted = 0;
	bzero(EndStream, sizeof(EndStream));
}

int server::start()
{
	init(); // initialisation
	
	int options=-1;
	struct sockaddr_in infosock;
	int infosock_size = sizeof(infosock);
	
	if (protocol == TCP)
	{
		sockd = socket(AF_INET,SOCK_STREAM,0);
		
		setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (int));

		if (bind(sockd, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) != 0) { perror("Bind"); return(0); }
		if (listen(sockd, 20) != 0) { perror("Listen"); return(0); }
		
		isStarted = 1;
		struct arg_tcp *myarg;
		while(1 && isStarted)
		{
			pthread_t th;
			
			myarg = new struct arg_tcp;
			bzero(myarg, sizeof(struct arg_tcp));
			myarg->m_serv = this;
			
			myarg->sock = accept(sockd, (struct sockaddr*)&(myarg->info_tcp), (socklen_t *)&infosock_size);
			if (! isStarted) { close(myarg->sock); break; }

			pthread_create(&th, NULL, &server::beginconnected, (void*) myarg);
			pthread_detach(th);
		}
	}
	else
	{
		sockd = socket(PF_INET, SOCK_DGRAM, 0);
		
		setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (int));

		if (bind(sockd, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) != 0) { perror("Bind"); return(0); }
		//if (listen(sockd, 20) != 0) { perror("Listen"); return(0); }
		
		isStarted = 1;
		struct arg_udp *myarg;
		while(1 && isStarted)
		{
			pthread_t th;
			
			myarg = new struct arg_udp;
			bzero(myarg, sizeof(struct arg_udp));
			myarg->m_serv = this;
			
			int n  = recvfrom(sockd, &(myarg->msg), sizeof(myarg->msg)-1, 0,  (struct sockaddr *)&(myarg->info_udp), (socklen_t *)&infosock_size);
			if (! isStarted) break;
			
			pthread_create(&th, NULL, &server::beginreceived, (void*) myarg);
			pthread_detach(th);
		}
	}
	isStarted = 0;
	return 1;
}

void server::stop()
{
	close(sockd);
	isStarted = 0;
}

int server::readline(int sock, unsigned char *buf)
{
	// lit une ligne de taille linesize
	// attention un mauvais parametre (sizeof(buf) < linesize) peut entrainer un probleme de securite
	bzero(buf, linesize);
	int size = read(sock, buf, linesize - 1);
	if (size <= 0) return 0;
	else return size;
}

int server::sread(int sock, unsigned char *buf, int limit)
{
	int size = 0;
	int tsize = 0;
	int nread = linesize;
	
	while(size < limit)
	{
		// on delimite le nombre a lire
		if (size + linesize > limit)
			nread = limit - size;
		else
			nread = linesize;
		
		tsize = read(sock, buf+size, nread);
		if (tsize <= 0) return 0;
		
		size += tsize;
		
		// fin de stream ?
		char *posend = strstr((char*)buf, EndStream);
		if (posend)
		{
			*(posend+strlen(EndStream)) = '\0';
			break;
		}
		else if (size >= limit) return 0;
	}
	buf[limit] = '\0';
	return size;
}

// *** SET ***
void server::setProtocol(int proto)
{
	if (proto == TCP or proto == UDP)
		protocol = proto;
}

void server::setPort(int p) { port = p; }

void server::setParam(void *arg)
{
	param = arg;
}

void server::setEndStream(char *mastream)
{
	strncpy(EndStream, mastream, sizeof(EndStream)-1);
}

// *** GET ***
int server::getProtocol() { return protocol; }
int server::getPort() { return port; }
void server::getEndStream(char *mastream)
{
	mastream = EndStream;
}

// *** THREAD ***
void* server::beginconnected(void* data)
{
	struct arg_tcp *myarg;
	server *m_serv;
	myarg = (struct arg_tcp*) data;
	m_serv = myarg->m_serv;
	int mysock = myarg->sock;
	
	struct sockaddr_in info_tcp = myarg->info_tcp;
	delete myarg;
	
	m_serv->connected(mysock, info_tcp);
	close(mysock);
}

void* server::beginreceived(void* data)
{
	struct arg_udp *myarg;
	server *m_serv;
	myarg = (struct arg_udp*) data;
	m_serv = myarg->m_serv;
	
	m_serv->received(m_serv->sockd, myarg->msg, myarg->info_udp);
	delete myarg;
}

void server::received(int sock, char *msg, struct sockaddr_in info_udp) {}

void server::connected(int sock, struct sockaddr_in info_tcp) {}
// info_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");

#endif
