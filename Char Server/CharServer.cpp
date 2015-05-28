#include "CharServer.h"
#include "D:\Programmieren\ConfigReader\Config.h"
#include "D:\Programmieren\QuickInfos\QuickInfo.h"

CharServer *mainServer;

CharServer::CharServer(WORD port, MYSQL* mysql) {
	this->port = config->getValueInt("ChannelPort");
	this->ipString = config->getValueString("ChannelIp");
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;
	std::cout << "[" << config->getValueString("ChannelName") << "]: " << config->getValueString("WorldIp") << ":" << config->getValueString("WorldPort") << "\n";
}

CharServer::~CharServer() {
	this->sqlDataBase.disconnect();

	::closesocket(this->loginServer.socket);
	this->loginServer.socket = SOCKET_ERROR;
	this->worldServer.socket = SOCKET_ERROR;
}

CharClient* CharServer::createClient(SOCKET sock) {
	CharClient* newChar = new CharClient(sock, this);
	return newChar;
}

void CharServer::loadEncryption() {
	GenerateCryptTables(this->crypt.table, 0x87654321);
}

bool CharServer::checkForServer(ClientSocket* client, std::string& ip) {
	if (this->loginServer.clientHandle == nullptr && _stricmp(::config->getValueString("LoginIp"), ip.c_str()) == 0) {
		this->loginServer.clientHandle = client;
		std::cout << "LoginServer fully connected: " << ip.c_str() << "\n";
		return true;
	}
	if (this->worldServer.clientHandle == nullptr && _stricmp(::config->getValueString("WorldIp"), ip.c_str()) == 0) {
		this->worldServer.clientHandle = client;

		this->worldServer.ip = ip;
		this->worldServer.port = ::config->getValueInt("WorldPort"); 

		if (!this->establishInternalConnection(&this->worldServer))
			return false;

		std::cout << "WorldServer fully connected: " << ip.c_str() << "\n";
		return true;
	}
	return false;
}

void CharServer::registerServer() {
	this->loginServer.port = ::config->getValueInt("LoginPort");
	this->loginServer.ip = std::string(::config->getValueString("LoginIp"));

	sockaddr_in ain;
	ain.sin_family = AF_INET;
	ain.sin_addr.S_un.S_addr = ::inet_addr(this->loginServer.ip.c_str());
	ain.sin_port = ::htons(this->loginServer.port);

	this->loginServer.socket = ::socket(AF_INET, SOCK_STREAM, 0x00);
	if (this->loginServer.socket == SOCKET_ERROR)
		throw TraceableException("Socket couldn't be established.\nReason: %s\n", QuickInfo::getErrorDescription(WSAGetLastError()));
	while (::connect(this->loginServer.socket, (const sockaddr*)&ain, sizeof(sockaddr)) != 0) {
		Sleep(1);
	}
	std::cout << "Connection to LoginServer established! Awaiting permission...\n";
}
