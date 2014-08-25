#pragma once

#ifndef __ROSE_SERVER_SOCKET__
#define __ROSE_SERVER_SOCKET__

#include "BaseSocket.h"
#include "ClientSocket.h"
#include "PacketIDs.h"
#include <vector>

extern class ConfigA* config;
extern std::string workingPath;

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
		WORD port;
		std::string ipString;
		std::vector<ClientSocket*> clients;
		DataBase sqlDataBase;
		virtual void registerServer() { };
		virtual void executeRequests() { };
	public:
		virtual ClientSocket* createClient(SOCKET sock) {
			return new ClientSocket(sock, this);
		}
		virtual void loadEncryption() { GenerateCryptTables(this->crypt.table, 0x87654321); }
		
		bool sqlInsert(const char* fmt, ...) {
			ArgConverterA(result, fmt);
			return this->sqlDataBase.put(result.c_str());
		}
		bool sqlRequest(const char* fmt, ...) {
			ArgConverterA(result, fmt);
			return this->sqlDataBase.get(result.c_str()) != nullptr;
		}
		__inline MYSQL_ROW sqlGetNextRow() {
			return this->sqlDataBase.getNextEntry();
		}
		__inline DWORD sqlGetRowCount() {
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