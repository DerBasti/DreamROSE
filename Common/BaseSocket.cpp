#include "BaseSocket.h"

bool initWinSocket() {
	WSADATA wsa;
	return (WSAStartup(MAKEWORD(2, 0), &wsa) == 0);
}

void closeWinSocket() {
	WSACleanup();
}

void cryptPacket(char *packet, char *crypttable)
{
	unsigned short paksize = (*((unsigned short*)&packet[0])) - 2;

	for (int i = 0; i<paksize; i++) {
		unsigned tmp1 = (*((unsigned*)&crypttable[0]) + 1) & 0xff;
		unsigned tmp2 = (unsigned)crypttable[tmp1 + 8];
		unsigned tmp3 = ((*((unsigned*)&crypttable[4]) + tmp2) & 0xff);
		unsigned tmp4 = crypttable[tmp3 + 8];
		*((unsigned*)&crypttable[0]) = tmp1;
		*((unsigned*)&crypttable[4]) = tmp3;
		crypttable[tmp3 + 8] = tmp2;
		crypttable[tmp1 + 8] = tmp4;
		packet[i + 2] = crypttable[((tmp4 + tmp2) & 0xff) + 8] ^ packet[i + 2];
	}
}