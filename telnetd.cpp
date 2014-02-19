#ifndef __telnetdcpp__
#define __telnetdcpp__ 

#include "telnetd.h"

void sig_handler(int sig)
{
	if (sig==SIGCHLD)
	{
		int retval;
		wait(&retval);
	}
}

void telnetd::initsigaction()
{
	struct sockaddr_in server;
	struct sigaction act;

	bzero(&act, sizeof(act));
	act.sa_handler = sig_handler;
	act.sa_flags = SA_NOCLDSTOP;
	sigaction(SIGCHLD,&act,0);
}

telnetd::telnetd()
{
	int passwdmode = 0;
	initsigaction();
	pthread_mutex_init(&locklog, NULL);
	
	// opening logs
	mylog = open((char*)"./telnetlog.txt", O_WRONLY, O_APPEND);
	if (mylog < 0)
	{
		printf("Can't open the logs file, terminating\n");
		exit(0);
	}
	lseek(mylog, 0, SEEK_END);
	
	if (chroot("chroot") < 0)
	{
		printf("Can't chroot in ./chroot, terminating\n");
		exit(0);
	}
	
}

telnetd::telnetd(char *pass)
{
	bzero(passwd, sizeof(passwd));
	strncpy(passwd, pass, sizeof(passwd)-1);
	int passwdmode = 1;
	initsigaction();
}

void telnetd::setPassTelnet(char *pass)
{
	memcpy(passwd, pass, sizeof(passwd));
}

void telnetd::getPassTelnet(char *pass)
{
	memcpy(pass, passwd, sizeof(passwd));
}

int telnetd::degradation(int uid)
{
	environ = NULL;
	gid_t listgroup[1];
	listgroup[0]=uid;
	if (setgroups(1, listgroup) == -1) perror("SetGroups");
	if (setregid(uid,uid) == -1) perror("SetGid");
	if (setreuid(uid,uid) == -1) perror("SetUid");
}

void telnetd::connected(int sock, struct sockaddr_in info_tcp)
{
	char login[256], passenter[256];
	int connected = 1;
	
	// drop privileges
	degradation(1000);
	
	chdir("/");
	
	char buffer[1024], buff2[1024];
	int nblu;
		
	while (connected)
	{
		// write a prompt
		write(sock, prompt_string, strlen(prompt_string));
		bzero(buffer, sizeof(buffer));
		nblu = read(sock, buffer, sizeof(buffer)-1);
		if (nblu <= 0 )
		{
			connected = 0;
			break;
		}
		
		// log result
		// set a mutex
		pthread_mutex_lock(&locklog);
			printf("cmd : %s", buffer);
			if (write(mylog, (char*)"[cmd]\n", 6) <= 0)
			{
				printf("write error ...");
			}
			write(mylog, buffer, nblu);
		pthread_mutex_unlock(&locklog);
		
		// purge the command
		int i = 0;
		while(buffer[i])
		{
			if (buffer[i] == '\n' || buffer[i] == '\r')
				buffer[i] = '\0';
			i++;
		}
		
		// execute the command
		FILE *ptr;
		int nlu2;

		if ((ptr = popen(buffer, "r")) != NULL)
		{
			while (1)
			{
				bzero(buff2, sizeof(buff2));
				nlu2 = fread(buff2, 1, sizeof(buff2) - 1, ptr);
				printf("res : %s", buff2);
				if (nlu2 <= 0) break;
				// echo socket
				if (write(sock, buff2, nlu2) < 0)
				{
					connected = 0;
					break;
				}
				// log result
				// set a mutex
				pthread_mutex_lock(&locklog);
					write(mylog, (char*)"[result]\n", 9);
					write(mylog, buff2, nlu2);
				pthread_mutex_unlock(&locklog);
			}
			pclose(ptr);
		}
		else
		{
			//fprintf(stderr, "Echec de popen\n");
			printf("Error popen !\n");
			exit(1);
		}

	}
	
	close(sock);
}

#endif 
