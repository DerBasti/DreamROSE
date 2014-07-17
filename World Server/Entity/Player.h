#pragma once

#ifndef __ROSE_PLAYER__
#define __ROSE_PLAYER__

#include "..\..\Common\ClientSocket.h"
#include "..\..\Common\PacketIDs.h"
#include "Entity.h"

class Player : public Entity, public ClientSocket {
	private:
		struct _accountInfo {
			DWORD id;
			std::string userName;
			std::string password;
			bool isLoggedIn;
			BYTE accessLevel;
		} accountInfo;
		struct Attributes {

#define ATTRIB_EX(name) WORD name; WORD name##Ex;
			
			ATTRIB_EX(strength);
			ATTRIB_EX(dexterity);
			ATTRIB_EX(intelligence);
			ATTRIB_EX(concentration);
			ATTRIB_EX(charm);
			ATTRIB_EX(sensibility);

#undef ATTRIB_EX

			Attributes() {
				this->concentration = this->dexterity = this->intelligence = this->strength = 0x0F;
				this->charm = this->sensibility = 0x0A;
			}

			__inline WORD getStrength() const { return this->strength; }
			__inline WORD getStrengthEx() const {return this->strengthEx; }
			__inline WORD getStrengthTotal() { return (this->strength + this->strengthEx); }
			
			__inline WORD getDexterity() const { return this->dexterity; }
			__inline WORD getDexterityEx() const {return this->dexterityEx; }
			__inline WORD getDexterityTotal() { return (this->dexterity + this->dexterityEx); }

			__inline WORD getIntelligence() const { return this->intelligence; }
			__inline WORD getIntelligencex() const {return this->intelligenceEx; }
			__inline WORD getIntelligenceTotal() { return (this->intelligence + this->intelligenceEx); }

			__inline WORD getConcentration() const { return this->concentration; }
			__inline WORD getConcentrationEx() const { return this->concentrationEx; }
			__inline WORD getConcentrationTotal() { return (this->concentration + this->concentrationEx); }

			__inline WORD getCharm() const { return this->charm; }
			__inline WORD getCharmEx() const { return this->charmEx; }
			__inline WORD getCharmTotal() { return (this->charm + this->charmEx); }

			__inline WORD getSensibility() const { return this->sensibility; }
			__inline WORD getSensibilityEx() const { return this->sensibilityEx; }
			__inline WORD getSensibilityTotal() { return (this->sensibility + this->sensibilityEx); }

		} attributes;
		struct CharInfo {
			DWORD id;
			std::string name;
			WORD job;
			BYTE level;
			DWORD experience;
			WORD skillPoints;
			WORD statPoints;
			QWORD zulies;
			struct VisualTraits {
				BYTE sex;
				WORD hairStyle;
				WORD faceStyle;
				VisualTraits() {
					this->sex = 0x00;
					this->hairStyle = 0x00;
					this->faceStyle = 0x01;
				}
			} visualTraits;
			CharInfo() {
				this->name = "";
				this->id = this->experience = 0x00;
				this->job = this->skillPoints = this->statPoints = 0x00;
				this->zulies = 0x00;
				this->level = 0x00;
			}
		} charInfo;

		Item inventory[Inventory::MAXIMUM];

		struct BasicSkills {
			WORD id;
			BasicSkills() {
				id = 0x00;
			}
		} basicSkills[30];

		bool handlePacket();

		//Packetfunctions which exist for overview only
		bool pakPlayerInfos();
		bool pakQuestData();
		bool pakInventory();
		bool pakSpawnPlayer( Player* player );
		bool pakSpawnNPC( class NPC* npc );
		bool pakSpawnMonster(class Monster* monster);
		bool pakRemoveEntityVisually(Entity* entity);

		//Packets which are requested to be handled
		bool pakPing();
		bool pakExit();
		bool pakReturnToCharServer();
		bool pakIdentify();

		bool pakAssignID();
		bool pakTerrainCollision();
		bool pakSetEmotion();
		bool pakMoveCharacter();
		bool pakChangeStance();
		bool pakLocalChat();
		bool pakShoutChat();
		bool pakTelegate();
		
		void addSectorVisually(MapSector* newSector);
		void removeSectorVisually(MapSector* toRemove);
		void addEntityVisually(Entity* entity);
		
		const CharInfo::VisualTraits& getVisualTraits() const { return this->charInfo.visualTraits; }
	public:
		Player(SOCKET sock, ServerSocket* server);
		~Player();

		bool loadInfos();
		bool pakTelegate(const WORD mapId, const Position& pos);
		
		virtual void setPositionCurrent(const Position& newPos);
		virtual void setPositionDest(const Position& newPos);
		
		void updateAttackpower();
		void updateDefense();
		void updateMagicDefense();
		void updateHitrate();
		void updateDodgerate();
		void updateCritrate();
		void updateMovementSpeed();

		bool isAllied(Entity* entity);
		bool isAllied( class NPC* npc ) { return true; }
		bool isAllied( class Monster* mon ) { return false; }
		bool isAllied( Player* player) { return true; }

		__inline std::string getName() const { return this->charInfo.name; }

		__inline BYTE getLevel() const { return this->charInfo.level; }
		__inline WORD getJob() const { return this->charInfo.job; }

		//The same as "getJob()"
		__inline WORD getClass() const { return this->getJob(); }

		__inline WORD getStrength() const { return this->attributes.getStrength(); }
		__inline WORD getStrengthTotal() { return this->attributes.getStrengthTotal(); }
		
		__inline WORD getDexterity() const { return this->attributes.getDexterity(); }
		__inline WORD getDexterityTotal() { return this->attributes.getDexterityTotal(); }
		
		__inline WORD getIntelligence() const { return this->attributes.getIntelligence(); }
		__inline WORD getIntelligenceTotal() { return this->attributes.getIntelligenceTotal(); }
		
		__inline WORD getConcentration() const { return this->attributes.getConcentration(); }
		__inline WORD getConcentrationTotal() { return this->attributes.getConcentrationTotal(); }
		
		__inline WORD getCharm() const { return this->attributes.getCharm(); }
		__inline WORD getCharmTotal() { return this->attributes.getCharmTotal(); }
		
		__inline WORD getSensibility() const { return this->attributes.getSensibility(); }
		__inline WORD getSensibilityTotal() { return this->attributes.getSensibilityTotal(); }
};

#endif //__ROSE_PLAYER__