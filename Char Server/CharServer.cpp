#include "CharServer.h"

CharServer *mainServer;

CharServer::CharServer(WORD port, MYSQL* mysql) {
	this->port = config->getValueInt("ChannelPort");
	this->ipString = config->getValueString("ChannelIp");
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;
	std::cout << "[" << config->getValueString("WorldName") << "]: " << config->getValueString("WorldIp") << ":" << config->getValueString("WorldPort") << "\n";
}

CharServer::~CharServer() {
	this->sqlDataBase.disconnect();
}

CharClient* CharServer::createClient(SOCKET sock) {
	CharClient* newChar = new CharClient(sock, this);
	return newChar;
}

void CharServer::loadEncryption() {
	GenerateCryptTables(this->crypt.table, 0x87654321);
}

void CharServer::registerServer() {
}

