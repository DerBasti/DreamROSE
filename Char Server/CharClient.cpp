#include "CharClient.h"
#include "CharServer.h"
#include "..\Common\PacketIDs.h"

CharClient::CharClient(SOCKET sock, ServerSocket* newServer) {
	this->accountInfo.isLoggedIn = false;
	this->accountInfo.password = "";
	this->accountInfo.userName = "";

	this->selectedCharacter.id = 0x00;
	this->selectedCharacter.name = "";

	this->socket = sock;
	this->serverDelegate = newServer;

	this->characters.reserve(5);
}

CharClient::~CharClient() {
	this->accountInfo.isLoggedIn = false;
	this->accountInfo.password = "";
	this->accountInfo.userName = "";

	this->selectedCharacter.id = 0x00;
	this->selectedCharacter.name = "";
}

bool CharClient::pakIdentifyAccount() {
	Packet pak(PacketID::Character::Response::IDENTIFY);
	pak.addByte(0x00);
	pak.addDWord(0x87654321); //Encryption
	pak.addDWord(0x00);

	this->accountInfo.id = this->packet.getDWord(0x00);

	if (!mainServer->sqlRequest("SELECT name from accounts WHERE id=%i", this->accountInfo.id) || mainServer->sqlGetRowCount() != 1)
		return false;
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	this->accountInfo.userName = std::string(row[0]);
	this->accountInfo.isLoggedIn = true;
	mainServer->sqlFinishQuery();

	return this->sendData(pak);
}

bool CharClient::pakGetCharacters() {
	BYTE itemAmount = 10;

	Packet pak(PacketID::Character::Response::GET_CHARACTERS);

	if (!mainServer->sqlRequest("SELECT * from characters WHERE acc_id=%i", this->accountInfo.id))
		return false;
	
	//Character Amount - will never be bigger than BYTE (255)
	pak.addByte(static_cast<BYTE>(mainServer->sqlGetRowCount()));
	for (unsigned int i = 0; i < mainServer->sqlGetRowCount(); i++) {
		MYSQL_ROW row = mainServer->sqlGetNextRow();

		Character newChar;
		newChar.id = atoi(row[0]);
		newChar.name = std::string(row[2]);
		newChar.level = atoi(row[3]);
		newChar.classId = atoi(row[5]);
		newChar.faceStyle = atoi(row[6]);
		newChar.hairStyle = atoi(row[7]);
		newChar.sex = atoi(row[8]);
		newChar.deleteTime = 0x00;
		this->characters.addValue(newChar);
	}
	mainServer->sqlFinishQuery();

	for (unsigned int i = 0; i < this->characters.size(); i++) {
		Character& newChar = this->characters.getValue(i);
		if (!mainServer->sqlRequest("SELECT slot, itemId, refine from inventory WHERE charId=%i AND slot < 9", newChar.id))
			return false;
		DWORD itemCnt = mainServer->sqlGetRowCount();
		for (unsigned int j = 0; j < itemCnt; j++) {
			MYSQL_ROW row = mainServer->sqlGetNextRow();
			Item& curItem = newChar.equipment[atoi(row[0])];
			curItem.itemId = atoi(row[1]) % 10000;
			curItem.refineLevel = atoi(row[2]);
		}
		mainServer->sqlFinishQuery();
		pak.addString(newChar.name.c_str());
		pak.addByte(0x00);
		pak.addByte(newChar.sex);
		pak.addWord(newChar.level);
		pak.addWord(newChar.classId);
		pak.addDWord(newChar.deleteTime);
		pak.addByte(0x00); //Platinum ?
		pak.addDWord(newChar.faceStyle);
		pak.addDWord(newChar.hairStyle);
#define ITEM_SENDING(slot) pak.addWord(newChar.equipment[slot].itemId); pak.addWord(newChar.equipment[slot].refineLevel);
		//#define ITEM_SENDING(slot) pak.addWord(newChar.equipment.at(slot).itemId); pak.addDWord(newChar.equipment.at(slot).refineLevel);

		ITEM_SENDING(2);
		ITEM_SENDING(3);
		ITEM_SENDING(5);
		ITEM_SENDING(6);
		ITEM_SENDING(1);
		ITEM_SENDING(4);
		ITEM_SENDING(7);
		ITEM_SENDING(8);
	}
	return this->sendData(pak);
}

bool CharClient::pakCreateCharacter() {
	Packet pak(PacketID::Character::Response::CREATE_CHARACTER);
	if (this->characters.size() == 5) { //MAX CHARS REACHED
		pak.addWord(0x04);
		return this->sendData(pak);
	}
	Character newChar;

	newChar.classId = 0x00;
	newChar.deleteTime = 0x00;
	newChar.faceStyle = this->packet.getByte(0x03);
	newChar.id = this->characters.size() + 1;
	newChar.hairStyle = this->packet.getByte(0x02);
	newChar.level = 150;
	newChar.name = this->packet.getString(0x07);
	newChar.sex = this->packet.getByte(0x00) % 2;

	if (!mainServer->sqlInsert("INSERT INTO characters (acc_id, name, level, job, face, hair, sex) VALUES(%i, '%s', %i, %i, %i, %i, %i)", this->accountInfo.id, newChar.name.c_str(),
		newChar.level, newChar.classId, newChar.faceStyle, newChar.hairStyle, newChar.sex))
		return false;

	if (!mainServer->sqlRequest("SELECT id FROM characters WHERE acc_id=%i AND name='%s'", this->accountInfo.id, newChar.name.c_str()))
		return false;
	newChar.id = atoi(mainServer->sqlGetNextRow()[0]);
	mainServer->sqlFinishQuery();

	if (!mainServer->sqlInsert("INSERT INTO character_stats(id) VALUES(%i)", newChar.id))
		return false;

	if (!mainServer->sqlInsert("INSERT INTO character_skills(id, basicSkills, passiveSkills, activeSkills) VALUES(%i, '%s', '%s', '%s')", newChar.id,
		"11,12,16,20,41,42,43,181,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
		"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
		"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"))
		return false;
	pak.addWord(0x00);

	this->characters.addValue(newChar);
	return this->sendData(pak);
}

bool CharClient::pakDeleteCharacter() {
	return true;
}

bool CharClient::pakGetWorldserverIp() {
	const char *data = &this->packet.getData()[0x03];
	this->selectedCharacter.name = std::string(data);

	DWORD charId = 0x00;
	for (unsigned int i = 0; i < this->characters.size(); i++) {
		if (_stricmp(this->characters.getValue(i).name.c_str(), this->selectedCharacter.name.c_str()) == 0) {
			charId = this->characters.getValue(i).id;
		}
	}

	if (!mainServer->sqlInsert("UPDATE accounts SET lastChar_Id=%i WHERE id=%i AND name='%s'", charId, this->accountInfo.id, this->accountInfo.userName.c_str())) {
		return false;
	}

	Packet pak(PacketID::Character::Response::GET_WORLDSERVER_IP);
	pak.addWord(config->getValueInt("WorldPort")); //PORT
	pak.addDWord(this->accountInfo.id);
	pak.addDWord(0x87654321);
	pak.addString(config->getValueString("WorldIp")); //IP
	pak.addByte(0x00);
	if (!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::Character::Response::MESSAGE_MANAGER);
	pak.addByte(0x06); //Send Friendlist
	pak.addByte(0x00); //NUMBER OF FRIENDS
	return this->sendData(pak);
}

bool CharClient::handlePacket() {
	std::cout << "New Packet: " << std::hex << this->packet.getCommand() << " with Length " << std::dec << this->packet.getLength() << "\n";
	switch (this->packet.getCommand()) {
		case PacketID::Character::Request::IDENTIFY:
			return this->pakIdentifyAccount();

		case PacketID::Character::Request::GET_CHARACTERS:
			return this->pakGetCharacters();

		case PacketID::Character::Request::CREATE_CHARACTER:
			return this->pakCreateCharacter();

		case PacketID::Character::Request::DELETE_CHARACTER:
			return this->pakDeleteCharacter();

		case PacketID::Character::Request::GET_WORLDSERVER_IP:
			return this->pakGetWorldserverIp();

		case PacketID::Character::Request::CLAN_MANAGER:
			return true;

		case PacketID::Character::Request::MESSAGE_MANAGER:
			return true;
	}
	return false;
}