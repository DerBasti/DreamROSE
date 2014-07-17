#include "LoginClient.h"
#include "LoginServer.h"
#include "D:\Programmieren\ConfigReader\Config.h"
#include <iostream>

void LoginClient::setBaseInfo(const WORD userId, const DWORD accessLevel, std::string& userNam, std::string& pw) {
	this->accountInfo.userId = userId;
	this->accountInfo.accessLevel = accessLevel;
	this->accountInfo.userName = userNam;
	this->accountInfo.password = pw;
}

bool LoginClient::pakEncryptionRequest() {
	this->crypt.status.additionalValuePerStep = rand() | (rand() << 16);

	Packet pak(PacketID::Login::Response::ENCRYPTION);
	pak.addByte(0x02);
	pak.addDWord(this->crypt.status.additionalValuePerStep);

	return this->sendData(pak);
}

bool LoginClient::pakRequestChannelList() {
	DWORD requestedServer = this->packet.getDWord(0x00);
	
	Packet pak(PacketID::Login::Response::GET_SERVERLIST);
	pak.addDWord(requestedServer);
	pak.addByte(0x01);

	pak.addWord(0x01); //ID
	pak.addByte(0x00); //?
	pak.addWord(0x00); //STATUS
	pak.addString(::config->getValueString("ChannelName")); //NAME
	pak.addByte(0x00);
	/*
	Iterate through all the channel
	LoginServer* server = dynamic_cast<LoginServer*>(this->serverDelegate);

	BYTE serverStatus = 0x00;
	for (unsigned int i = 0; i < server->getChannelAmount(); i++) {

	Channel* channel = server->getChannel(i);
	pak.addWord( channel->getID() );
	pak.addByte( 0x00 );

	serverStatus = (channel->getPlayerAmount() * 100) / (channel->getMaxConnections() == 0 ? 1 : channel->getMaxConnections());
	pak.addWord( serverStatus );

	pak.addWord( channel->getName() );
	pak.addByte( 0x00 );
	}
	*/
	return this->sendData(pak);
}
bool LoginClient::pakUserLogin() {

	this->accountInfo.password = std::string(this->packet.getData(), 0x00, 0x20);
	this->accountInfo.userName = std::string(this->packet.getData(), 0x20, this->packet.getLength() - Packet::DEFAULT_HEADER_OFFSET - 0x20);

	if(!mainServer->sqlRequest("SELECT id, accesslevel, isonline FROM accounts WHERE name='%s'", this->accountInfo.userName.c_str(), this->accountInfo.password.c_str())) {
		if (config->getValueString("CreateOnLogin") && mainServer->sqlGetRowCount() == 0) {
			mainServer->sqlInsert("INSERT INTO accounts (name, password, accesslevel, isonline) VALUES('%s', '%s', 1, 1)", this->accountInfo.userName.c_str(), this->accountInfo.password.c_str());
			Packet pak(PacketID::Login::Response::USER_LOGIN);
			pak.addByte(0x02);
			pak.addDWord(0x00);
			return this->sendData(pak);
		} else {
			if(!mainServer->sqlRequest("SELECT password FROM accounts WHERE name='%s'", this->accountInfo.userName)) {
				Packet pak(PacketID::Login::Response::USER_LOGIN);
				pak.addByte(0x02);
				pak.addDWord(0x00);
				return this->sendData(pak);
			}
			if(_stricmp(mainServer->sqlGetNextRow()[0], this->accountInfo.password.c_str())!=0) {
				Packet pak(PacketID::Login::Response::USER_LOGIN);
				pak.addByte(0x02);
				pak.addDWord(0x00);
				return this->sendData(pak);
			}
		}
	}
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	this->accountInfo.userId = static_cast<DWORD>(atol(row[0]));
	this->accountInfo.accessLevel = static_cast<DWORD>(atol(row[1]));
	mainServer->sqlFinishQuery();

	Packet pak(PacketID::Login::Response::USER_LOGIN);
	//ADDDWORD( pak, 0x6f000000 );

	//OKAY
	pak.addDWord(0x0c000000);
	pak.addByte(0x00);
	
	//0x30 + ID
	pak.addByte(0x31);
	pak.addString(config->getValueString("ServerName"));
	pak.addByte(0x00);

	//ChannelId
	pak.addDWord(0x01);

	return this->sendData(pak);
}

bool LoginClient::pakRequestChannelIP() {

	DWORD serverId = this->packet.getDWord(0x00);
	BYTE channel = this->packet.getByte(0x04);

	Packet pak(PacketID::Login::Response::GET_CHANNEL_IP);
	pak.addByte(0x00); //Channel status
	pak.addDWord(this->accountInfo.userId);
	pak.addDWord(0x87654321); //Encryption
	pak.addString(config->getValueString("ChannelIp"));
	pak.addByte(0x00);
	pak.addWord(config->getValueInt("ChannelPort")); //PORT

	return this->sendData(pak);
}

bool LoginClient::handlePacket() {
	std::cout << "New Packet: " << std::hex << this->packet.getCommand() << " with Length " << std::dec << this->packet.getLength() << "\n";
	switch (this->packet.getCommand()) {
		case PacketID::Login::Request::ENCRYPTION:
			return this->pakEncryptionRequest();

		case PacketID::Login::Request::GET_SERVER_LIST:
			return this->pakRequestChannelList();

		case PacketID::Login::Request::USER_LOGIN:
			return this->pakUserLogin();

		case PacketID::Login::Request::GET_CHANNEL_IP:
			return this->pakRequestChannelIP();

		case PacketID::Login::Request::GAME_GUARD:
			return true;
	}
	return true;
}