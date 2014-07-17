#pragma once

#ifndef __ROSE_CHAR_CLIENT__
#define __ROSE_CHAR_CLIENT__

#include "..\Common\ClientSocket.h"

struct Item {
	DWORD owner;
	BYTE itemType;
	WORD itemId;
	BYTE refineLevel;
	BYTE durability;
	BYTE lifespan;

	Item() {
		this->owner = 0x00;
		this->itemId = 0x00;
		this->itemType = this->refineLevel = this->durability = this->lifespan = 0x00;
	}

	bool operator==(const Item& rhs) {
		if (owner == rhs.owner &&
			itemType == rhs.itemType &&
			itemId == rhs.itemId &&
			refineLevel == rhs.refineLevel &&
			durability == rhs.durability &&
			lifespan == rhs.lifespan)
			return true;
		return false;
	}
	bool operator!=(const Item& rhs) {
		return !(operator==(rhs));
	}
};

struct Character {
	DWORD id;
	std::string name;
	WORD level;
	WORD classId;
	DWORD faceStyle;
	DWORD hairStyle;
	BYTE sex;
	DWORD deleteTime;

	Item equipment[10];

	Character& operator=(const Character& rhs) {
		this->id = rhs.id;
		this->name = rhs.name;
		this->level = rhs.level;
		this->classId = rhs.classId;
		this->faceStyle = rhs.faceStyle;
		this->hairStyle = rhs.hairStyle;
		this->sex = rhs.sex;
		this->deleteTime = rhs.deleteTime;
		for (unsigned int i = 0; i < 10; i++)
			this->equipment[i] = rhs.equipment[i];
		
		return (*this);
	}

	bool operator==(const Character& rhs) {
		if (this->id == rhs.id &&
			_stricmp(this->name.c_str(), rhs.name.c_str()) == 0 &&
			this->level == rhs.level &&
			this->classId == rhs.classId &&
			this->faceStyle == rhs.faceStyle &&
			this->hairStyle == rhs.hairStyle &&
			this->sex == rhs.sex &&
			this->deleteTime == rhs.deleteTime) {
				
			for (unsigned int j = 0; j < 10; j++) {
				if (this->equipment[j] != rhs.equipment[j])
					return false;
			}
			return true;
		}
		return false;
	}
	bool operator!=(const Character& rhs) {
		return !(operator==(rhs));
	}
};


class CharClient : public ClientSocket {
	private:
		struct _accountInfo {
			DWORD id;
			std::string userName;
			std::string password;
			bool isLoggedIn;
		} accountInfo;
		struct _characterInfo {
			DWORD id;
			std::string name;
		} selectedCharacter;
		FixedArray<Character> characters;

		bool pakIdentifyAccount();
		bool pakGetCharacters();
		bool pakCreateCharacter();
		bool pakDeleteCharacter();
		bool pakGetWorldserverIp();
	public:
		CharClient(SOCKET sock, ServerSocket* newServer);
		~CharClient();

		bool handlePacket();
};

#endif //__ROSE_CHAR_CLIENT__