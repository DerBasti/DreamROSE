#pragma once

#ifndef __ROSE_PACKET_IDS__
#define __ROSE_PACKET_IDS__

#include "datatypes.h"

class PacketID {
	private:
		PacketID() {}
		~PacketID() {}
	public:
		class Login {
			private:
				Login() {};
				~Login() {};
			public:
				class Request {
				private:
					Request() {};
					~Request() {};
				public:
					const static WORD ENCRYPTION = 0x703;
					const static WORD GET_SERVER_LIST = 0x704;
					const static WORD USER_LOGIN = 0x708;
					const static WORD GET_CHANNEL_IP = 0x70A;
					const static WORD GAME_GUARD = 0x808;
				};
				class Response {
				private:
					Response() {};
					~Response() {};
				public:
					const static WORD GET_SERVERLIST = 0x704;
					const static WORD USER_LOGIN = 0x708;
					const static WORD GET_CHANNEL_IP = 0x70A;
					const static WORD ENCRYPTION = 0x7FF;
					class UserLoginMessages {
					private:
						UserLoginMessages() {};
						~UserLoginMessages() {};
					public:
						const static BYTE SERVER_MAINTAINANCE = 0x00;
						const static BYTE GENERAL_ERROR = 0x01;
						const static BYTE BAD_PASSWORD = 0x03;
						const static BYTE ALREADY_LOGGED_IN = 0x04;
						const static BYTE BANNED = 0x05;
						const static BYTE TOPUP_ACCOUNT = 0x06;
						const static BYTE RETRY_CONNECTION = 0x07;
					};
				};
		};
		class Character {
			private:
				Character() {};
				~Character() {};
			public:
				class Request {
					private:
						Request() {};
						~Request() {};
					public:
						const static WORD IDENTIFY = 0x70B;
						const static WORD GET_CHARACTERS = 0x712;
						const static WORD CREATE_CHARACTER = 0x713;
						const static WORD DELETE_CHARACTER = 0x714;
						const static WORD GET_WORLDSERVER_IP = 0x715;
						const static WORD CLAN_MANAGER = 0x7E0;
						const static WORD MESSAGE_MANAGER = 0x7E1;
				};
				class Response {
					private:
						Response() {};
						~Response() {};
					public:
						const static WORD IDENTIFY = 0x70C;
						const static WORD GET_CHARACTERS = 0x712;
						const static WORD CREATE_CHARACTER = 0x713;
						const static WORD DELETE_CHARACTER = 0x714;
						const static WORD GET_WORLDSERVER_IP = 0x711;
						const static WORD RETURN_TO_CHARSERVER = 0x71C;

						const static WORD CLAN_MANAGER = 0x7E0;
						const static WORD MESSAGE_MANAGER = 0x7E1;
				};
			};
		class World {
			private:
				World() { }
				~World() {}
			public:
				class Request {
					private:
						Request() {}
						~Request() {}
					public:
						const static WORD PING = 0x700;
						const static WORD EXIT = 0x707;
						const static WORD IDENFITY = 0x70B;
						const static WORD RETURN_TO_CHARSERVER = 0x71C;
						const static WORD GET_ID = 0x753;
						const static WORD TERRAIN_COLLISION = 0x770;
						const static WORD SET_EMOTION = 0x781;
						const static WORD CHANGE_STANCE = 0x782;
						const static WORD LOCAL_CHAT = 0x783;
						const static WORD SHOUT_CHAT = 0x785;
						const static WORD REMOVE_VISIBLE_PLAYER = 0x794;
						const static WORD MOVEMENT = 0x79A;
						const static WORD TELEGATE = 0x7A8;
						const static WORD CLAN_MANAGER = 0x7E0;
						const static WORD MESSAGE_MANAGER = 0x7E1;
				};
				class Response {
					private:
						Response() {}
						~Response() {}
					public:
						const static WORD PING = 0x700;
						const static WORD EXIT = 0x707;
						const static WORD IDENFITY = 0x70C;
						const static WORD PLAYER_INFOS = 0x715;
						const static WORD PLAYER_INVENTORY = 0x716;
						const static WORD RETURN_TO_CHARSERVER = 0x71C;
						const static WORD QUEST_DATA = 0x71B;
						const static WORD UNKNOWN = 0x721;
						const static WORD SPAWN_ON_MAP = 0x730;
						const static WORD ASSIGN_ID = 0x753;
						const static WORD WEIGHT = 0x762;
						const static WORD TERRAIN_COLLISION = 0x770;
						const static WORD SET_EMOTION = 0x781;
						const static WORD CHANGE_STANCE = 0x782;
						const static WORD LOCAL_CHAT = 0x783;
						const static WORD SHOUT_CHAT = 0x785;
						const static WORD SPAWN_NPC = 0x791;
						const static WORD SPAWN_MONSTER = 0x792;
						const static WORD SPAWN_PLAYER = 0x0793;
						const static WORD REMOVE_VISIBLE_PLAYER = 0x794;
						const static WORD MOVEMENT_MONSTER = 0x797;
						const static WORD MOVEMENT = 0x79A;
						const static WORD SHOW_MONSTER_HP = 0x79F;
						const static WORD TELEGATE = 0x7A8;
						const static WORD GAMING_PLAN = 0x7DE;
						const static WORD CLAN_MANAGER = 0x7E0;
						const static WORD MESSAGE_MANAGER = 0x7E1;
				};
		};
};

#endif //__ROSE_PACKET_IDS__