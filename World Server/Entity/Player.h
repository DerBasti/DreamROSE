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
			dword_t id;
			std::string userName;
			std::string password_t;
			bool isLoggedIn;
			byte_t accessLevel;
			bool debugMode;
		} accountInfo;
		struct Attributes {

#define ATTRIB_EX(name) word_t name; word_t name##Ex;
			
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

			__inline word_t getStrength() const { return this->strength; }
			__inline word_t getStrengthEx() const {return this->strengthEx; }
			__inline word_t getStrengthTotal() { return (this->strength + this->strengthEx); }
			
			__inline word_t getDexterity() const { return this->dexterity; }
			__inline word_t getDexterityEx() const {return this->dexterityEx; }
			__inline word_t getDexterityTotal() { return (this->dexterity + this->dexterityEx); }

			__inline word_t getIntelligence() const { return this->intelligence; }
			__inline word_t getIntelligencex() const {return this->intelligenceEx; }
			__inline word_t getIntelligenceTotal() { return (this->intelligence + this->intelligenceEx); }

			__inline word_t getConcentration() const { return this->concentration; }
			__inline word_t getConcentrationEx() const { return this->concentrationEx; }
			__inline word_t getConcentrationTotal() { return (this->concentration + this->concentrationEx); }

			__inline word_t getCharm() const { return this->charm; }
			__inline word_t getCharmEx() const { return this->charmEx; }
			__inline word_t getCharmTotal() { return (this->charm + this->charmEx); }

			__inline word_t getSensibility() const { return this->sensibility; }
			__inline word_t getSensibilityEx() const { return this->sensibilityEx; }
			__inline word_t getSensibilityTotal() { return (this->sensibility + this->sensibilityEx); }

			void resetStats() {
				this->concentration = this->dexterity = this->intelligence = this->strength = 0x0F;
				this->charm = this->sensibility = 0x0A;
			}
		} attributes;
		struct CharInfo {
			dword_t id;
			std::string name;
			word_t job;
			byte_t level;
			dword_t experience;
			word_t skillPoints;
			word_t statPoints;
			word_t respawnTownId;
			struct VisualTraits {
				byte_t sex;
				word_t hairStyle;
				word_t faceStyle;
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
			dword_t maxRate;
			dword_t valueConsumed;
			dword_t valuePerSecond;
			word_t influencedAbility;
			ChronoTimer timeStamp;

			ConsumedItem(const word_t abilityType, const dword_t max, const dword_t valueEachSecond) {
				this->influencedAbility = abilityType;
				this->maxRate = max;
				this->valueConsumed = 0x00;
				this->valuePerSecond = valueEachSecond;
			}
		};

		std::vector<ConsumedItem> consumedItems;
		PlayerInventory inventory;

		struct questInfo {
			const static byte_t JOURNEY_MAX = 10;
			const static byte_t FLAGS_MAX_byte_t = 0x40;
			const static byte_t FLAGS_MAX_dword_t = 0x10;

			PlayerQuest* selected;
			FixedArray<PlayerQuest*> journey;

			union {
				byte_t flag[0x40];
				dword_t dwFlag[0x10];
			};

			struct questVars {
				const static byte_t EPISODE_MAX = 5;
				const static byte_t JOB_MAX = 3;
				const static byte_t PLANET_MAX = 7;
				const static byte_t UNION_MAX = 10;

				FixedArray<word_t> episode;
				FixedArray<word_t> job;
				FixedArray<word_t> planet;
				FixedArray<word_t> fraction;
			} var;
		} quest;

		FixedArray<Skill*> skills;
		FixedArray<word_t> quickbar;

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
		bool pakUpdateInventoryVisually( const byte_t slotAmount, const byte_t* slotIds );

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
		bool pakShowMonsterHP();
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
		bool pakExecuteSkill();
		bool pakEquipmentChange();
		bool pakQuestAction();
		bool pakQuickbarAction();
		bool pakPickUpDrop();
		bool pakDropFromInventory();
		bool pakBuyFromNPC();
		bool pakSellToNPC();
		bool pakConsumeItem();
		
		void addEntityVisually(Entity* entity);
		void removeEntityVisually(Entity* entity);
		bool setPositionVisually(const position_t& pos);

		const byte_t getFreeInventorySlot(const Item& item);
		const CharInfo::VisualTraits& getVisualTraits() const { return this->charInfo.visualTraits; }

		bool saveQuests();
		bool loadQuests();

		bool addConsumableToList(const word_t abilityToInfluence, const dword_t maxValue, const dword_t valuePerMilliSecond);

	public:
		Player(SOCKET sock, ServerSocket* server);
		~Player();

		bool loadInfos();
		bool saveInfos();
		bool pakTelegate(const word_t mapId, const position_t& pos);
		bool pakUpdateLifeStats();
		
		virtual void setPositionCurrent(const position_t& newPos);
		virtual void setPositionDest(const position_t& newPos);
		
		//Retrieves a statType which is only available to a player (e.g. hair).
		dword_t getSpecialStatType(const word_t statType);

		//Changes (e.g. adds/removes) a given abilityType (e.g. current health, stat points) based on the given amount
		//and operation. The "sendChangePacket"-flag determines whether the client shall be notified immediately upon
		//change or not.
		//@Return Value: true if the abilityType was found and changed; In case the flag is set to true, the client has to
		//receive the packet as well in order to be true. All other cases will return a false.
		bool changeAbility(const word_t abilityType, const dword_t amount, const byte_t operationFromOperationService, bool sendChangePacket=true);

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
		bool updateZulies(const qword_t newAmount);
		__inline qword_t getZulies() const { return this->inventory[0x00].amount; }
		void checkRegeneration();

		bool executeSkill(const byte_t skillSlot);

		bool addQuest(const word_t questId);
		bool updateQuestData();
		bool sendQuestTrigger(const dword_t hash, bool success);
		bool sendQuestTriggerViaMonster(const word_t monType);
		bool searchAndSelectQuest(const dword_t questId);
		__inline PlayerQuest* getSelectedQuest() const { return this->quest.selected; }
		PlayerQuest* getQuestByID(const word_t questId);
		PlayerQuest* getEmptyQuestSlot();
		const word_t getQuestVariable(word_t varType, const word_t varId);
		void setQuestVariable(word_t varType, const word_t varId, const word_t value);
		__inline byte_t getQuestFlag(const word_t flagGroup) { return this->quest.flag[flagGroup>>3] & (1 << (flagGroup&0x07)); }
		void setQuestFlag(const word_t flagGroup, byte_t newValue);
		__inline void clearQuestFlag(const word_t flagGroup) { this->quest.flag[flagGroup>>3] = 0x00; }

		word_t checkClothesForStats(const word_t statAmount, ...);
		word_t checkSkillsForStats(const word_t basicAmount, const word_t statAmount, ...);

		bool isAllied( class NPC* npc ) { return true; }
		bool isAllied( class Monster* mon ) { return false; }
		bool isAllied( Player* player) { return true; }

		__inline std::string getName() const { return this->charInfo.name; }
		bool isInDebugMode() const { return this->accountInfo.debugMode; }
		void setDebugMode(bool wantedMode) { this->accountInfo.debugMode = wantedMode; }

		__inline byte_t getLevel() const { return this->charInfo.level; }
		__inline void setLevel(const byte_t newLevel) { 
			this->charInfo.level = newLevel-1; 
			this->charInfo.experience = this->getExperienceForLevelup();
			this->addExperience(0);
		}
		__inline word_t getJob() const { return this->charInfo.job; }
		__inline dword_t getExperience() const { return this->charInfo.experience; }
		dword_t getExperienceForLevelup();
		void addExperience(const dword_t additionalExp);

		//The same as "getJob()"
		__inline word_t getClass() const { return this->getJob(); }

		__inline word_t getMaxHPW() const { return static_cast<word_t>(this->stats.getMaxHP()); }
		__inline word_t getMaxMPW() const { return static_cast<word_t>(this->stats.getMaxMP()); }

		__inline word_t getStrength() const { return this->attributes.getStrength(); }
		__inline word_t getStrengthTotal() { return this->attributes.getStrengthTotal(); }
		
		__inline word_t getDexterity() const { return this->attributes.getDexterity(); }
		__inline word_t getDexterityTotal() { return this->attributes.getDexterityTotal(); }
		
		__inline word_t getIntelligence() const { return this->attributes.getIntelligence(); }
		__inline word_t getIntelligenceTotal() { return this->attributes.getIntelligenceTotal(); }
		
		__inline word_t getConcentration() const { return this->attributes.getConcentration(); }
		__inline word_t getConcentrationTotal() { return this->attributes.getConcentrationTotal(); }
		
		__inline word_t getCharm() const { return this->attributes.getCharm(); }
		__inline word_t getCharmTotal() { return this->attributes.getCharmTotal(); }
		
		__inline word_t getSensibility() const { return this->attributes.getSensibility(); }
		__inline word_t getSensibilityTotal() { return this->attributes.getSensibilityTotal(); }

		void resetAttributes();
		void resetSkills();

		__inline Skill* getSkill(const byte_t skillSlot) const { return this->skills[skillSlot]; }
		byte_t getSlotOfLearnedSkill(const word_t skillId) const;

		//Return values: 
		//0 = skill not found/learned; 
		//1 = skill found, skillLevel is lower than the required one; 
		//2 = skill found + skilllevel exceeds wanted skill
		const byte_t isSkillLearned(const Skill* skillToFind);

		//Changes a learned skill based on the given basic id. 
		//@Return Value: In case it was found and changed (including client notification), true will be returned, otherwise false.
		bool changeSkill(const word_t totalId);

		//Adds a skill to the client's skill list.
		//@Return Value: if a skillslot was assignable as well as the successful notification of the client, it will return true. In all other instances false.
		bool addSkill(Skill* skillToAdd);

		__inline bool isWeaponEquipped() const { return this->inventory[PlayerInventory::Slots::WEAPON].amount > 0; }
		bool addItemToInventory(const Item& item, byte_t slotId = std::numeric_limits<byte_t>::max());
		Item getItemFromInventory(const word_t itemSlot); 
		Item getQuestItem(const dword_t itemId);

		float getAttackRange() const;
		bool getAttackAnimation(); 
};

#endif //__ROSE_PLAYER__