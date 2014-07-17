#include "LoginServer.h"
#include "LoginClient.h"
#include "D:\Programmieren\ConfigReader\Config.h"

LoginServer* mainServer;

LoginServer::LoginServer(WORD port, MYSQL* mysql) {
	this->port = port;
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;

	config = new ConfigA();
	std::string workingPath = ""; 
	QuickInfo::getPath( &workingPath );
	config->init( (workingPath + "\\config.conf").c_str() );
	this->ipString = config->getValueString("LoginIp");
	std::cout << "[" << config->getValueString("ChannelName") << "]: " << config->getValueString("ChannelIp") << ":" << config->getValueString("ChannelPort") << "\n";
}

LoginServer::~LoginServer() {
	this->sqlDataBase.disconnect();
	delete config;
}

ClientSocket* LoginServer::createClient(SOCKET sock) {
	LoginClient* client = new LoginClient(sock, this);
	return client;
}