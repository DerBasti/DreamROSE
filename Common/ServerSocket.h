#pragma once

#ifndef __ROSE_SERVER_SOCKET__
#define __ROSE_SERVER_SOCKET__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "BaseSocket.h"
#include "ClientSocket.h"
#include "PacketIDs.h"
#include <vector>

extern class ConfigA* config;
extern std::string workingPath;

struct InternalServerConnection {
	std::string ip;
	word_t port;
	SOCKET socket;

	//Necessary in order to send/receive data
	ClientSocket* clientHandle; 
	InternalServerConnection() {
		this->ip = std::string("");
		this->port = 0x00;
		this->socket = SOCKET_ERROR;
		this->clientHandle = nullptr;
	}
};

class ServerSocket : public BaseSocket {
	private:
		SOCKET maxFd;
		struct sockaddr_in ain;
		struct sockaddr_in sain;
		bool mainLoop();

		bool fillFDS(fd_set* fds);
		void addClient(SOCKET sock, sockaddr_in* info);
		void handleClients(fd_set* fds);
		void disconnectClient(ClientSocket* client);
	protected:
		word_t port;
		std::string ipString;
		std::vector<ClientSocket*> clients;
		DataBase sqlDataBase;
		virtual void registerServer() { };
		virtual void executeRequests() { };
		virtual void setupBeforeMainLoop() { };
		virtual void onClientDisconnect(ClientSocket* client) { };
		virtual bool checkForServer(ClientSocket* client, std::string& ip) { return false; };
		bool establishInternalConnection(InternalServerConnection* con);
#undef max
		bool sendInternalData(InternalServerConnection* con, const word_t pakId, const dword_t optional = std::numeric_limits<dword_t>::max());
	public:
		virtual ClientSocket* createClient(SOCKET sock) {
			return new ClientSocket(sock, this);
		}
		virtual void loadEncryption() { GenerateCryptTables(this->crypt.table, 0x87654321); }
		
		bool sqlInsert(const char* fmt, ...) {
			va_list args; va_start(args, fmt);
			std::string result = QuickInfo::convertVarsToString(fmt, args);
			va_end(args);
			return this->sqlDataBase.put(result.c_str());
		}
		bool sqlRequest(const char* fmt, ...) {
			va_list args; va_start(args, fmt);
			std::string result = QuickInfo::convertVarsToString(fmt, args);
			va_end(args);
			return this->sqlDataBase.get(result.c_str()) != nullptr;
		}
		__inline MYSQL_ROW sqlGetNextRow() {
			return this->sqlDataBase.getNextEntry();
		}
		__inline dword_t sqlGetRowCount() {
			return this->sqlDataBase.getRowCount();
		}
		__inline void sqlFinishQuery() { return this->sqlDataBase.finishQuery(); }

		__inline const char* sqlLastError() { return this->sqlDataBase.lastError(); }
		__inline void sqlFreeResult() {
			this->sqlDataBase.freeResult();
		}

		bool start();
		ServerSocket();
		virtual ~ServerSocket();
};

#endif //__ROSE_SERVER_SOCKET__