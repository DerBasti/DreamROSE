#pragma once

#ifndef __ROSE_BASE_CLIENT__
#define __ROSE_BASE_CLIENT__

#include "BaseSocket.h"

class ClientSocket : public BaseSocket {
	public:
		Packet packet;
		WORD bytesReceived;
		WORD bytesExpected;
		std::string ip;
		class ServerSocket* serverDelegate;
		virtual bool handlePacket() { return true; }
	public:
		ClientSocket() {
			this->bytesReceived = 0x00; this->bytesExpected = Packet::DEFAULT_HEADER_OFFSET;
			this->socket = SOCKET_ERROR;
			this->serverDelegate = nullptr;
		};
		ClientSocket(SOCKET sock, ServerSocket* pServerDelegate) {
			this->bytesReceived = 0x00; this->bytesExpected = Packet::DEFAULT_HEADER_OFFSET;
			this->socket = sock;
			this->serverDelegate = pServerDelegate;
		};
		~ClientSocket();
		bool receiveData();
		bool sendData( Packet& pak );
};

#endif //__ROSE_BASE_CLIENT__
