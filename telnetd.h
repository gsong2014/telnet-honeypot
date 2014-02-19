#ifndef __telnetdh__
#define __telnetdh__ 

#include "serveur.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>

#define login_message_telnetd "Login: "
#define pass_message_telnetd "Password: "
#define prompt_string "[root@localhost]# "
#define mode_in 0
#define mode_out 1
#define mode_err 2

using namespace std;

class telnetd : public serveur
{
	private :
		char passwd[256];
		int passwdmode;
		pthread_mutex_t locklog;
		int mylog;
		
	protected :
		void initsigaction();
		int degradation(int uid);

	public :
		telnetd();
		telnetd(char *pass);
		
		void setPassTelnet(char *pass);
		void getPassTelnet(char *pass);
		
		void connected(int sock, struct sockaddr_in info_tcp);

};


#endif
