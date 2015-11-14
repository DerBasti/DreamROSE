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
					const static word_t CHARSERVER_IDENTIFY = 0x001;
					const static word_t ENCRYPTION = 0x703;
					const static word_t GET_SERVER_LIST = 0x704;
					const static word_t USER_LOGIN = 0x708;
					const static word_t GET_CHANNEL_IP = 0x70A;
					const static word_t GAME_GUARD = 0x808;
				};
				class Response {
				private:
					Response() {};
					~Response() {};
				public:
					const static word_t GET_SERVERLIST = 0x704;
					const static word_t USER_LOGIN = 0x708;
					const static word_t GET_CHANNEL_IP = 0x70A;
					const static word_t ENCRYPTION = 0x7FF;
					class UserLoginMessages {
					private:
						UserLoginMessages() {};
						~UserLoginMessages() {};
					public:
						const static byte_t SERVER_MAINTAINANCE = 0x00;
						const static byte_t GENERAL_ERROR = 0x01;
						const static byte_t BAD_PASSword_t = 0x03;
						const static byte_t ALREADY_LOGGED_IN = 0x04;
						const static byte_t BANNED = 0x05;
						const static byte_t TOPUP_ACCOUNT = 0x06;
						const static byte_t RETRY_CONNECTION = 0x07;
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
						const static word_t LOGINSERVER_IDENTIFY = 0x002;
						const static word_t IDENTIFY = 0x70B;
						const static word_t GET_CHARACTERS = 0x712;
						const static word_t CREATE_CHARACTER = 0x713;
						const static word_t DELETE_CHARACTER = 0x714;
						const static word_t GET_WORLDSERVER_IP = 0x715;
						const static word_t CLAN_MANAGER = 0x7E0;
						const static word_t MESSAGE_MANAGER = 0x7E1;
						const static word_t UNKNOWN = 0x7E5;
				};
				class Response {
					private:
						Response() {};
						~Response() {};
					public:
						const static word_t IDENTIFY = 0x70C;
						const static word_t GET_CHARACTERS = 0x712;
						const static word_t CREATE_CHARACTER = 0x713;
						const static word_t DELETE_CHARACTER = 0x714;
						const static word_t GET_WORLDSERVER_IP = 0x711;
						const static word_t RETURN_TO_CHARSERVER = 0x71C;

						const static word_t CLAN_MANAGER = 0x7E0;
						const static word_t MESSAGE_MANAGER = 0x7E1;
						const static word_t UNKNOWN = 0x7E5;
				};
				class Custom {
					private:
						Custom() {};
						~Custom() {};
					public:
						const static word_t CLIENT_EXIT = 0x1234;
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
						const static word_t PING = 0x700;
						const static word_t EXIT = 0x707;
						const static word_t IDENFITY = 0x70B;
						const static word_t NPC_TRADE = 0x717;
						const static word_t RETURN_TO_CHARSERVER = 0x71C;
						const static word_t QUEST = 0x730;
						const static word_t GET_ID = 0x753;
						const static word_t RESPAWN_AFTER_DEATH = 0x755;
						const static word_t RESPAWN_TOWN = 0x756;
						const static word_t TERRAIN_COLLISION = 0x771;
						const static word_t SET_EMOTION = 0x781;
						const static word_t CHANGE_STANCE = 0x782;
						const static word_t LOCAL_CHAT = 0x783;
						const static word_t WHISPER_CHAT = 0x784;
						const static word_t SHOUT_CHAT = 0x785;
						const static word_t REMOVE_VISIBLE_PLAYER = 0x794;
						const static word_t INIT_BASIC_ATTACK = 0x798;
						const static word_t MOVEMENT_PLAYER = 0x79A;
						const static word_t UPDATE_EXPERIENCE = 0x79B;
						const static word_t LEVEL_UP = 0x79E;
						const static word_t SHOW_MONSTER_HP = 0x79F;
						const static word_t USE_CONSUMABLE = 0x7A3;
						const static word_t DROP_FROM_INVENTORY = 0x7A4;
						const static word_t EQUIPMENT_CHANGE = 0x7A5;
						const static word_t PICK_DROP = 0x7A7;
						const static word_t TELEGATE = 0x7A8;
						const static word_t INCREASE_ATTRIBUTE = 0x7A9;
						const static word_t QUICKBAR_ACTION = 0x7AA;
						const static word_t LEARN_SKILL = 0x7B0;
						const static word_t INCREASE_SKILL_LEVEL = 0x7B1;
						const static word_t SKILL_ON_SELF = 0x7B2;
						const static word_t SKILL_ATTACK = 0x7B3;
						const static word_t SKILL_AOE = 0x7B4;
						const static word_t CLAN_MANAGER = 0x7E0;
						const static word_t MESSAGE_MANAGER = 0x7E1;
				};
				class Response {
					private:
						Response() {}
						~Response() {}
					public:
						const static word_t PING = 0x700;
						const static word_t ANNOUNCEMENT = 0x702;
						const static word_t EXIT = 0x707;
						const static word_t IDENFITY = 0x70C;
						const static word_t PLAYER_INFOS = 0x715;
						const static word_t PLAYER_INVENTORY = 0x716;
						const static word_t NPC_TRADE = 0x717;
						const static word_t UPDATE_INVENTORY = 0x718;
						const static word_t QUEST_DATA = 0x71B;
						const static word_t RETURN_TO_CHARSERVER = 0x71C;
						const static word_t REWARD_ZULIES = 0x71E;
						const static word_t REWARD_ITEM = 0x71F;
						const static word_t ADD_TO_ABILITY = 0x720;
						const static word_t CHANGE_ABILITY = 0x721;
						const static word_t UPDATE_QUEST_DATA = 0x723;
						const static word_t QUEST_ACTION = 0x730;
						const static word_t QUEST_ITEM = 0x731;
						const static word_t ASSIGN_ID = 0x753;
						const static word_t RESPAWN_AFTER_DEATH = 0x755;
						const static word_t WEIGHT = 0x762;
						const static word_t TERRAIN_COLLISION = 0x771;
						const static word_t CONVERT_MONSTER = 0x774;
						const static word_t SET_EMOTION = 0x781;
						const static word_t CHANGE_STANCE = 0x782;
						const static word_t LOCAL_CHAT = 0x783;
						const static word_t WHISPER_CHAT = 0x784;
						const static word_t SHOUT_CHAT = 0x785;
						const static word_t SPAWN_NPC = 0x791;
						const static word_t SPAWN_MONSTER = 0x792;
						const static word_t SPAWN_PLAYER = 0x0793;
						const static word_t REMOVE_VISIBLE_ENTITY = 0x794;
						const static word_t MOVEMENT_MONSTER = 0x797;
						const static word_t INIT_BASIC_ATTACK = 0x798;
						const static word_t BASIC_ATTACK = 0x799;
						const static word_t MOVEMENT_PLAYER = 0x79A;
						const static word_t UPDATE_EXPERIENCE = 0x79B;
						const static word_t LEVEL_UP = 0x79E;
						const static word_t SHOW_MONSTER_HP = 0x79F;
						const static word_t USE_CONSUMABLE = 0x7A3;
						const static word_t EQUIPMENT_CHANGE = 0x7A5;
						const static word_t SPAWN_DROP = 0x7A6;
						const static word_t PICK_DROP = 0x7A7;
						const static word_t TELEGATE = 0x7A8;
						const static word_t INCREASE_ATTRIBUTE = 0x7A9;
						const static word_t QUICKBAR_ACTION = 0x7AA;
						const static word_t LEARN_SKILL = 0x7B0;
						const static word_t INCREASE_SKILL_LEVEL = 0x7B1;
						const static word_t SKILL_ATTACK = 0x7B3;
						const static word_t SKILL_DAMAGE = 0x7B6;
						const static word_t BUFFS = 0x7B7;
						const static word_t SKILL_ANIMATION = 0x7B9;
						const static word_t SKILL_INITIATION = 0x7BB;
						const static word_t GAMING_PLAN = 0x7DE;
						const static word_t CLAN_MANAGER = 0x7E0;
						const static word_t MESSAGE_MANAGER = 0x7E1;
						const static word_t REGENERATION = 0x7EC;
				};
		};
};

#endif //__ROSE_PACKET_IDS__