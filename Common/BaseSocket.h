#pragma once

#ifndef __ROSE_BASE_SOCKET__
#define __ROSE_BASE_SOCKET__

#include "datatypes.h"
#include "..\Common\rosecrypt.h"

class BaseSocket {
	protected:
		SOCKET socket;
		bool active;
		std::string IP;
		struct {
			CryptTable* table;
			CryptStatus	status;
		} crypt;
	public:
		BaseSocket() {
			this->socket = SOCKET_ERROR;
			this->active = false;
			this->IP = "localhost";
		}
		virtual ~BaseSocket() {
			this->closeSocket();
		}
		__inline bool isActive() const { return this->active; }
		__inline void setIsActive(bool value) { this->active = value; }

		__inline SOCKET getSocket() const { return this->socket; }
		__inline void closeSocket() {
			if (!this->isActive() || this->getSocket() == SOCKET_ERROR) return;
			closesocket(this->socket);

			this->socket = SOCKET_ERROR;
			this->active = false;
		}

		__inline std::string getIP() const { return this->IP; }
		__inline void setIP(std::string& newIp) { this->IP = newIp; }

		void setEncryption(CryptTable* table, CryptStatus& status) {
			this->crypt.table = table; 
			this->crypt.status.currentValue = status.currentValue;
			this->crypt.status.additionalValuePerStep = 0;
		}
};

bool initWinSocket();
void closeWinSocket();

#endif //__ROSE_BASE_SOCKET__