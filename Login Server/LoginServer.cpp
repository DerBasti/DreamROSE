#include "LoginServer.h"
#include "LoginClient.h"
#include "D:\Programmieren\ConfigReader\Config.h"

LoginServer* mainServer;

LoginServer::LoginServer(WORD port, MYSQL* mysql) {
	this->port = port;
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;

	std::string workingPath = ""; 
	QuickInfo::getPath( &workingPath );
	this->ipString = config->getValueString("LoginIp");
	std::cout << "[" << config->getValueString("ServerName") << "]: " << config->getValueString("ChannelIp") << ":" << config->getValueString("ChannelPort") << "\n";
}

LoginServer::~LoginServer() {
	this->sqlDataBase.disconnect();
	::closesocket(this->charServer.socket);
	this->charServer.socket = SOCKET_ERROR;
}

bool LoginServer::checkForServer(ClientSocket* client, std::string& ip) {
	if (this->charServer.clientHandle == nullptr && _stricmp(::config->getValueString("ChannelIp"), ip.c_str()) == 0) {
		this->charServer.ip = ip;
		this->charServer.port = ::config->getValueInt("ChannelPort");

		if (!this->establishInternalConnection(&this->charServer))
			return false;

		this->charServer.clientHandle = client;
		std::cout << "CharServer fully connected: " << ip.c_str() << "\n";

		return true;
	}
	return false;
}

ClientSocket* LoginServer::createClient(SOCKET sock) {
	LoginClient* client = new LoginClient(sock, this);
	return client;
}

bool LoginServer::notifyCharServer(const WORD packetId, const WORD optionalInfo) {
	Packet pak(packetId);
	pak.addWord(optionalInfo);
	return this->charServer.clientHandle->sendData(pak);
}