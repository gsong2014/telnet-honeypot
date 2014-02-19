#include "telnetd.h"

int main()
{
	telnetd *S = new telnetd;
	S->setPort(23);
	S->start();
	return 1;
}
