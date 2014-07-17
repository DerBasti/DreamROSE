#pragma once

#ifndef __ROSE_LOGIN_SERVER__
#define __ROSE_LOGIN_SERVER__

#include "..\Common\ServerSocket.h"

extern class ConfigA *config;

class LoginServer : public ServerSocket {
	private:
		void disconnectClient(ClientSocket* client);
	public:
		LoginServer(WORD port, MYSQL* mysql);
		~LoginServer();
		const static BYTE DEFAULT_ACCESSLEVEL = 0x00;
		ClientSocket* createClient(SOCKET sock);
		void loadEncryption() { ::GenerateLoginServerCryptTables(this->crypt.table); }
};

extern LoginServer *mainServer;

#endif //__ROSE_LOGIN_SERVER__