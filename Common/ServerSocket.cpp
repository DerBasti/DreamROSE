#include "ServerSocket.h"
#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

ConfigA* config;
std::string workingPath;

void changeConsoleFont() {
	std::wstring path = L""; QuickInfo::getPath(&path);
	if (!QuickInfo::loadFont((path + L"\\Inconsolata.otf").c_str())) {
		GlobalLogger::debug("Couldn't load font\n");
		return; 
	}
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_FONT_INFOEX font;
	font.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	font.dwFontSize.X = 0;
	font.dwFontSize.Y = 14;
	font.FontFamily = FF_DONTCARE;
	font.FontWeight = FW_DONTCARE;
	wcscpy_s(font.FaceName, L"Consolas");
	font.nFont = 0;

	if (SetCurrentConsoleFontEx(consoleHandle, false, &font) == 0) {
		GlobalLogger::debug("Error while setting font: %i", GetLastError());
	}
}

ServerSocket::ServerSocket() {
	changeConsoleFont();

	/**/
	QuickInfo::getPath(&workingPath, true);

	::config = new ConfigA();
	if (::config)
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
	dword_t errorCode = 0x00;

	if (this->socket == SOCKET_ERROR) {
		errorCode = WSAGetLastError();
		GlobalLogger::fatal("Error while creating Socket. ErrorCode: %i\n", errorCode);
		return false;
	}
	int optVal = 1;
	if (::setsockopt(this->socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optVal, sizeof(optVal)) == SOCKET_ERROR) {
		errorCode = WSAGetLastError();
		GlobalLogger::fatal("Error while changing socket settings: %s (%i)\n", QuickInfo::getErrorDescription(errorCode), errorCode);
		return false;
	}
	::setsockopt(this->socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optVal, sizeof(optVal));
	this->ain.sin_family = AF_INET;
	this->ain.sin_addr.S_un.S_addr = ::inet_addr(this->ipString.c_str());
	this->ain.sin_port = ::htons(this->port); 
	memset(&(this->ain.sin_zero), '\0', 8);
	if (::bind(this->socket, (const sockaddr*)&this->ain, sizeof(struct sockaddr))) {
		errorCode = WSAGetLastError();
		GlobalLogger::fatal("Error while binding socket: %s (%i)\n", QuickInfo::getErrorDescription(errorCode), errorCode);
		this->closeSocket();
		return false;
	}
	if (::listen(this->socket, SOMAXCONN) == SOCKET_ERROR) {
		errorCode = WSAGetLastError();
		GlobalLogger::fatal("Error while trying to listen on socket: %s (%i)\n", QuickInfo::getErrorDescription(errorCode), errorCode);
		this->closeSocket();
		return false;
	}
	GlobalLogger::info("Listening on: %s:%i\n", this->ipString.c_str(), this->port);
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
	GlobalLogger::info("Waiting for clients...\n");
	this->setupBeforeMainLoop();
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
				GlobalLogger::debug("Adding valid client\n");
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
	std::string newIp = std::string(inet_ntoa(info->sin_addr));
	if (!newClient) {
		GlobalLogger::warning("Client wasn't valid (Out of memory?) - IP: %s\n", newIp.c_str());
		::closesocket(sock);
		return;
	}
	newClient->setEncryption(this->crypt.table, this->crypt.status);
	newClient->setIP(newIp);
	newClient->setIsActive(true);

	//Check whether or not a server connected.
	//this->checkForServer(newClient, newIp);
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
	std::string ip = toDisconnect->getIP();
	this->onClientDisconnect(toDisconnect);
	//Deletes the derived classes aswell -> virtual deconstructor in base class
	delete toDisconnect;
	toDisconnect = nullptr;
	
	GlobalLogger::info("Client from IP %s disconnected.\n", ip.c_str());
}


bool ServerSocket::establishInternalConnection(InternalServerConnection* con) {
	sockaddr_in ain;
	ain.sin_family = AF_INET;
	ain.sin_addr.S_un.S_addr = ::inet_addr(con->ip.c_str());
	ain.sin_port = ::htons(con->port);
	con->socket = ::socket(AF_INET, SOCK_STREAM, 0x00);
	if (!con->socket || ::connect(con->socket, reinterpret_cast<sockaddr*>(&ain), sizeof(sockaddr)) != 0x00) {
		std::cout << "Socket couldn't be established.\nReason: " << QuickInfo::getErrorDescription(WSAGetLastError()) << "\n";
		return false;
	}
	return true;
}

bool ServerSocket::sendInternalData(InternalServerConnection* con, const word_t pakId, const dword_t optional) {
	Packet pak(pakId);
	pak.addDWord(optional);
	return con->clientHandle->sendData(pak);
}