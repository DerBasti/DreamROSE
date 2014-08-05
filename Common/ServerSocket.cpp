#include "ServerSocket.h"

Logger logger;
ConfigA* config;
std::string workingPath;
ServerSocket::ServerSocket() {
	QuickInfo::getPath(&workingPath, true);

	logger.setFile( (workingPath + "\\logger.log").c_str() );

	::config = new ConfigA();
	::config->init((workingPath + std::string("\\config.conf")).c_str());

	::initWinSocket();
	this->clients.clear();
}

ServerSocket::~ServerSocket() {
	for (unsigned int i = 0; i < this->clients.size(); i++) {
		ClientSocket* client = this->clients.at(i);
		if (client) {
			this->disconnectClient(client);
		}
	}
	delete ::config;
	::config = nullptr;
}

bool ServerSocket::start() {
	this->socket = ::socket(AF_INET, SOCK_STREAM, 0x00);
	DWORD errorCode = 0x00;
	if (this->socket == SOCKET_ERROR) {
		errorCode = WSAGetLastError();
		return false;
	}
	int optVal = 1;
	if (::setsockopt(this->socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optVal, sizeof(optVal)) == SOCKET_ERROR) {
		errorCode = WSAGetLastError();
		return false;
	}
	::setsockopt(this->socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optVal, sizeof(optVal));
	this->ain.sin_family = AF_INET;
	this->ain.sin_addr.S_un.S_addr = ::inet_addr(this->ipString.c_str());
	this->ain.sin_port = ::htons(this->port); 
	memset(&(this->ain.sin_zero), '\0', 8);
	if (::bind(this->socket, (const sockaddr*)&this->ain, sizeof(struct sockaddr))) {
		errorCode = WSAGetLastError();
		this->closeSocket();
		return false;
	}
	if (::listen(this->socket, SOMAXCONN) == SOCKET_ERROR) {
		this->closeSocket();
		return false;
	}
	std::cout << "Listening on port " << this->port << "\n";
	this->active = true;
	this->registerServer();
	this->mainLoop();
	return true;
}

bool ServerSocket::mainLoop() {
	fd_set		fds;
	int			activity;
	sockaddr_in	clientInfo;
	SOCKET		newClientSocket;

	timeval		timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;

	this->maxFd = this->getSocket();
	std::cout << "Waiting for clients...\n";
	do {
		newClientSocket = SOCKET_ERROR;
		FD_ZERO(&fds);
		this->fillFDS(&fds);
		FD_SET(this->socket, &fds);
		activity = select(this->maxFd + 1, &fds, nullptr, nullptr, &timeout);
		if (activity == 0 && this->clients.empty()) {
			continue;
		}
		if (FD_ISSET(this->socket, &fds))
		{
			int clientinfolen = sizeof(sockaddr_in);
			newClientSocket = accept(this->socket, (sockaddr*)&clientInfo, (int*)&clientinfolen);
			if (newClientSocket != INVALID_SOCKET)
			{
				std::cout << "Adding client!\n";
				this->addClient(newClientSocket, &clientInfo);
			}
		}
		this->handleClients(&fds);
		this->executeRequests();
	} while (this->isActive());
	return true;
}

bool ServerSocket::fillFDS(fd_set* fds) {
	for (unsigned int i = 0; i < this->clients.size(); i++) {
		ClientSocket* client = this->clients.at(i);
		if (client && client->isActive()) {
			FD_SET(client->getSocket(), fds);
			if (client->getSocket() > this->maxFd)
				this->maxFd = client->getSocket();
		}
		else {
			this->disconnectClient(client);
		}
	}
	return true;
}

void ServerSocket::handleClients(fd_set* fds) {
	for (unsigned int i = 0; i < this->clients.size(); i++) {
		ClientSocket* client = this->clients.at(i);
		if (!client->isActive())
			continue;
		if (FD_ISSET(client->getSocket(), fds)) {
			if (!client->receiveData()) {
				this->disconnectClient(client);
			}
		}
	}
}

void ServerSocket::addClient(SOCKET sock, sockaddr_in* info) {
	ClientSocket* newClient = this->createClient(sock);
	if (!newClient) {
		std::cout << "newClient invalid!\n";
		::closesocket(sock);
		return;
	}
	newClient->setEncryption(this->crypt.table, this->crypt.status);
	std::string newIp = std::string(inet_ntoa(info->sin_addr));
	newClient->setIP(newIp);
	newClient->setIsActive(true);
	this->clients.push_back(newClient);
}

void ServerSocket::disconnectClient(ClientSocket* toDisconnect) {
	for (unsigned int i = 0; i < this->clients.size(); i++) {
		ClientSocket* client = this->clients.at(i);
		if (client == toDisconnect) {
			this->clients.erase(this->clients.begin() + i);
			break;
		}
	}
	//Deletes the derived classes aswell -> virtual deconstructor in base class
	delete toDisconnect;
	toDisconnect = nullptr;
	
	std::cout << "Client disconnected\n";
}