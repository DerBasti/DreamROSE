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
			WORD respawnTownId;
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
				this->job = this->skillPoints = this->statPoints = this->respawnTownId = 0x00;
				this->level = 0x00;
			}
		} charInfo;

		struct ConsumedItem {
			DWORD maxRate;
			DWORD valueConsumed;
			DWORD valuePerSecond;
			WORD influencedAbility;
			clock_t timeStamp;

			ConsumedItem(const WORD abilityType, const DWORD max, const DWORD valueEachSecond) {
				this->influencedAbility = abilityType;
				this->maxRate = max;
				this->valueConsumed = 0x00;
				this->valuePerSecond = valueEachSecond;
				this->timeStamp = clock();
			}
		};

		std::vector<ConsumedItem> consumedItems;
		PlayerInventory inventory;

		struct questInfo {
			const static BYTE JOURNEY_MAX = 10;
			const static BYTE FLAGS_MAX_BYTE = 0x40;
			const static BYTE FLAGS_MAX_DWORD = 0x10;

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
		bool pakRespawnTown();
		bool pakTelegate();
		bool pakLearnSkill();
		bool pakIncreaseSkillLevel();
		bool pakEquipmentChange();
		bool pakQuestAction();
		bool pakPickUpDrop();
		bool pakDropFromInventory();
		bool pakBuyFromNPC();
		bool pakSellToNPC();
		bool pakConsumeItem();
		
		void addEntityVisually(Entity* entity);
		void removeEntityVisually(Entity* entity);
		bool setPositionVisually(const Position& pos);

		const BYTE getFreeInventorySlot(const Item& item);
		const CharInfo::VisualTraits& getVisualTraits() const { return this->charInfo.visualTraits; }

		bool saveQuests();
		bool loadQuests();

		bool addConsumableToList(const WORD abilityToInfluence, const DWORD maxValue, const DWORD valuePerMilliSecond);

	public:
		Player(SOCKET sock, ServerSocket* server);
		~Player();

		bool loadInfos();
		bool saveInfos();
		bool pakTelegate(const WORD mapId, const Position& pos);
		bool pakUpdateLifeStats();
		
		virtual void setPositionCurrent(const Position& newPos);
		virtual void setPositionDest(const Position& newPos);
		
		//Retrieves a statType which is only available to a player (e.g. hair).
		DWORD getSpecialStatType(const WORD statType);

		//Changes (e.g. adds/removes) a given abilityType (e.g. current health, stat points) based on the given amount
		//and operation. The "sendChangePacket"-flag determines whether the client shall be notified immediately upon
		//change or not.
		//@Return Value: true if the abilityType was found and changed; In case the flag is set to true, the client has to
		//receive the packet as well in order to be true. All other cases will return a false.
		bool changeAbility(const WORD abilityType, const DWORD amount, const BYTE operationFromOperationService, bool sendChangePacket=true);

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

		bool addQuest(const WORD questId);
		bool updateQuestData();
		bool sendQuestTrigger(const DWORD hash, bool success);
		bool sendQuestTriggerViaMonster(const WORD monType);
		bool searchAndSelectQuest(const DWORD questId);
		__inline PlayerQuest* getSelectedQuest() const { return this->quest.selected; }
		PlayerQuest* getQuestByID(const WORD questId);
		PlayerQuest* getEmptyQuestSlot();
		const WORD getQuestVariable(WORD varType, const WORD varId);
		void setQuestVariable(WORD varType, const WORD varId, const WORD value);
		__inline BYTE getQuestFlag(const WORD flagGroup) { return this->quest.flag[flagGroup>>3] & (1 << (flagGroup&0x07)); }
		void setQuestFlag(const WORD flagGroup, BYTE newValue);
		__inline void clearQuestFlag(const WORD flagGroup) { this->quest.flag[flagGroup>>3] = 0x00; }

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

		__inline Skill* getSkill(const BYTE skillSlot) const { return this->skills[skillSlot]; }


		//Return values: 
		//0 = skill not found/learned; 
		//1 = skill found, skillLevel is lower than the required one; 
		//2 = skill found + skilllevel exceeds wanted skill
		const BYTE isSkillLearned(const Skill* skillToFind);

		//Changes a learned skill based on the given basic id. 
		//@Return Value: In case it was found and changed (including client notification), true will be returned, otherwise false.
		bool changeSkill(const WORD totalId);

		//Adds a skill to the client's skill list.
		//@Return Value: if a skillslot was assignable as well as the successful notification of the client, it will return true. In all other instances false.
		bool addSkill(Skill* skillToAdd);

		__inline bool isWeaponEquipped() const { return this->inventory[PlayerInventory::Slots::WEAPON].amount > 0; }
		bool addItemToInventory(const Item& item, BYTE slotId = std::numeric_limits<BYTE>::max());
		Item getItemFromInventory(const WORD itemSlot); 
		Item getQuestItem(const DWORD itemId);

		float getAttackRange();
		bool getAttackAnimation(); 
};

#endif //__ROSE_PLAYER__