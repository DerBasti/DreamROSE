#pragma once

#ifndef __ROSE_PLAYER__
#define __ROSE_PLAYER__

#include "..\..\Common\ClientSocket.h"
#include "..\..\Common\PacketIDs.h"
#include "..\FileTypes\QSD.h"
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
				this->resetStats();

				this->charmEx = this->concentrationEx = this->dexterityEx = 
				this->intelligenceEx = this->sensibilityEx = this->strengthEx = 0x00;
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

			void resetStats() {
				this->concentration = this->dexterity = this->intelligence = this->strength = 0x0F;
				this->charm = this->sensibility = 0x0A;
			}
		} attributes;
		struct CharInfo {
			DWORD id;
			std::string name;
			WORD job;
			BYTE level;
			DWORD experience;
			WORD skillPoints;
			WORD statPoints;
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
				this->level = 0x00;
			}
		} charInfo;

		//std::vector<ConsumedItem> consumedItems;
		Item inventory[Inventory::MAXIMUM];

		struct questInfo {
			const static BYTE JOURNEY_MAX = 10;

			PlayerQuest* selected;
			FixedArray<PlayerQuest*> journey;

			union {
				BYTE flag[0x40];
				DWORD dwFlag[0x10];
			};

			struct questVars {
				const static BYTE EPISODE_MAX = 5;
				const static BYTE JOB_MAX = 3;
				const static BYTE PLANET_MAX = 7;
				const static BYTE UNION_MAX = 10;

				FixedArray<WORD> switches;
				FixedArray<WORD> episode;
				FixedArray<WORD> job;
				FixedArray<WORD> planet;
				FixedArray<WORD> fraction;
			} var;
		} quest;

		FixedArray<Skill*> skills;

		bool handlePacket();

		//Packetfunctions which exist for overview only
		bool pakPlayerInfos();
		bool pakQuestData();
		bool pakInventory();
		bool pakSpawnPlayer( Player* player );
		bool pakSpawnNPC( class NPC* npc );
		bool pakSpawnMonster(class Monster* monster);
		bool pakSpawnDrop(class Drop* drop);
		bool pakRemoveEntityVisually(Entity* entity);
		bool pakUpdateInventoryVisually( const BYTE slotAmount, const BYTE* slotIds );

		//Packets which are requested to be handled
		bool pakPing();
		bool pakExit();
		bool pakReturnToCharServer();
		bool pakIdentify();

		bool pakAssignID();
		bool pakRespawnAfterDeath();
		bool pakTerrainCollision();
		bool pakSetEmotion();
		bool pakInitBasicAttack();
		bool pakShowMonsterHP(Monster *mon);
		bool pakMoveCharacter();
		bool pakIncreaseAttribute();
		bool pakChangeStance();
		bool pakLocalChat();
		bool pakShoutChat();
		bool pakTelegate();
		bool pakEquipmentChange();
		bool pakQuestAction();
		bool pakPickUpDrop();
		bool pakDropFromInventory();
		bool pakBuyFromNPC();
		bool pakSellToNPC();
		
		void addEntityVisually(Entity* entity);
		void removeEntityVisually(Entity* entity);
		bool setPositionVisually(const Position& pos);

		const BYTE getFreeInventorySlot(const Item& item);
		const CharInfo::VisualTraits& getVisualTraits() const { return this->charInfo.visualTraits; }

	public:
		Player(SOCKET sock, ServerSocket* server);
		~Player();

		bool loadInfos();
		bool saveInfos();
		bool pakTelegate(const WORD mapId, const Position& pos);
		bool pakUpdateLifeStats();
		
		virtual void setPositionCurrent(const Position& newPos);
		virtual void setPositionDest(const Position& newPos);
		
		DWORD getSpecialStatType(const WORD statType);
		bool changeAbility(const WORD abilityType, const DWORD amount, const BYTE operation);

		void updateAttackpower();
		void updateAttackSpeed();
		void updateDefense();
		void updateMagicDefense();
		void updateHitrate();
		void updateMaxHP();
		void updateDodgerate();
		void updateCritrate();
		void updateMovementSpeed();
		void updateIntervalBetweenAttacks();
		bool updateZulies(const QWORD newAmount);
		__inline QWORD getZulies() const { return this->inventory[0x00].amount; }
		void checkRegeneration();

		bool searchAndSelectQuest(const DWORD questId);
		__inline PlayerQuest* getSelectedQuest() const { return this->quest.selected; }
		PlayerQuest* getQuestByID(const WORD questId);
		PlayerQuest* getEmptyQuestSlot();
		const WORD getQuestVariable(WORD varType, const WORD varId);
		void setQuestVariable(WORD varType, const WORD varId, const WORD value);
		__inline const DWORD getQuestSwitch(const WORD value) { return (this->quest.flag[value >> 0x03] & (1 << (value & 0x07))); }
		void setQuestSwitch(const WORD switchNum, BYTE operation) {
			if (operation == 0x00)
				this->quest.flag[switchNum >> 0x03] &= ~(1 << (switchNum & 0x07));
			else if (operation == 0x01)
				this->quest.flag[switchNum >> 0x03] |= (1 << (switchNum & 0x07));
			else if (operation == 0x02) {
				if (this->getQuestSwitch(switchNum))
					return this->setQuestSwitch(switchNum, 0x00);
				else
					return this->setQuestSwitch(switchNum, 0x01);
			}
		}
		__inline void clearQuestSwitchGroup(const WORD switchGroup) { this->quest.flag[switchGroup] = 0x00; }

		WORD checkClothesForStats(const WORD statAmount, ...);
		WORD checkSkillsForStats(const WORD basicAmount, const WORD statAmount, ...);

		bool isAllied( class NPC* npc ) { return true; }
		bool isAllied( class Monster* mon ) { return false; }
		bool isAllied( Player* player) { return true; }

		__inline std::string getName() const { return this->charInfo.name; }

		__inline BYTE getLevel() const { return this->charInfo.level; }
		__inline void setLevel(const BYTE newLevel) { 
			this->charInfo.level = newLevel-1; 
			this->charInfo.experience = this->getExperienceForLevelup();
			this->addExperience(0);
		}
		__inline WORD getJob() const { return this->charInfo.job; }
		__inline DWORD getExperience() const { return this->charInfo.experience; }
		DWORD getExperienceForLevelup();
		void addExperience(const DWORD additionalExp);

		//The same as "getJob()"
		__inline WORD getClass() const { return this->getJob(); }

		__inline WORD getMaxHPW() const { return static_cast<WORD>(this->stats.getMaxHP()); }
		__inline WORD getMaxMPW() const { return static_cast<WORD>(this->stats.getMaxMP()); }

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

		void resetAttributes();
		void resetSkills();

		__inline Skill* getSkill(const BYTE skillSlot) { return this->skills[skillSlot]; }
		bool changeSkill(const WORD totalId);

		__inline bool isWeaponEquipped() const { return this->inventory[Inventory::WEAPON].amount > 0; }
		bool addItemToInventory(const Item& item);
		bool equipItem(const Item& item);
		Item getItemFromInventory(const WORD itemSlot); 
		Item getQuestItem(const DWORD itemId);

		float getAttackRange();
		bool getAttackAnimation(); 
};

#endif //__ROSE_PLAYER__