#pragma once

#ifndef __ROSE_CHAR_SERVER__
#define __ROSE_CHAR_SERVER__

#include "..\Common\ServerSocket.h"
#include "CharClient.h"

extern class ConfigA* config;

class CharServer : public ServerSocket {
	private:
		void registerServer();
	public:
		CharServer(WORD port, MYSQL* mysql);
		~CharServer();
		CharClient* createClient(SOCKET sock);
		void loadEncryption();
};

extern CharServer* mainServer;

#endif //__ROSE_CHAR_SERVER__