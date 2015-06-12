#pragma once

#ifndef __ROSE_LOGIN_SERVER__
#define __ROSE_LOGIN_SERVER__

#include "..\Common\ServerSocket.h"

extern class ConfigA *config;

class LoginServer : public ServerSocket {
	private:
		InternalServerConnection charServer;

		void disconnectClient(ClientSocket* client);
		bool checkForServer(ClientSocket* clients, std::string& ip);
	public:
		LoginServer(WORD port, MYSQL* mysql);
		~LoginServer();
		const static BYTE DEFAULT_ACCESSLEVEL = 0x00;
		ClientSocket* createClient(SOCKET sock);
		void loadEncryption() { ::GenerateLoginServerCryptTables(this->crypt.table); }
		bool isCharServerConnected() { return this->charServer.clientHandle != nullptr; }
		bool notifyCharServer(const WORD packetId, const WORD optionalInfo = std::numeric_limits<WORD>::max());
};

extern LoginServer *mainServer;

#endif //__ROSE_LOGIN_SERVER__