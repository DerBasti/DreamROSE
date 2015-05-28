#pragma once

#ifndef __ROSE_QSD__
#define __ROSE_QSD__

#include <string>
#include <map>
#include "..\..\Common\Definitions.h"
#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include "..\Structures.h"
#include "STB.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

class Entity;

struct QuestHeaderInfo {
	const dword_t length;
	const dword_t operationType;
};

struct AbstractQuestInfo {
	QuestHeaderInfo header;
	const char data[512];
};

struct QuestTrans;

class QuestEntry {
	private:
		friend class QSD;
		friend class QuestService;
		byte_t checkNextTrigger;

#ifdef __ROSE_DEBUG__
		std::string qsdName;
		QSD* parent;
#endif
		std::string questName;
		dword_t questHash;
		word_t questId;

		FixedArray<Trackable<char>> conditions;
		FixedArray<Trackable<char>> actions;

		QuestEntry* previousQuest;
		QuestEntry* nextQuest;

		__inline void addCondition(Trackable<char>* newCond) { this->conditions.addValue(*newCond); }
		__inline void addAction(Trackable<char>* newAction) { this->actions.addValue(*newAction); }
	public:
		QuestEntry(const dword_t newQuestHash, const dword_t conditionCount, const dword_t actionCount) {
			this->questHash = newQuestHash;

			this->conditions.reserve(conditionCount);
			this->actions.reserve(actionCount);
		}
		__inline const dword_t getQuestHash() const { return this->questHash; }
		__inline const word_t getQuestId() const { return this->questId; }
		__inline QuestEntry* getPreviousQuest() const { return this->previousQuest; }
		__inline QuestEntry* getNextQuest() const { return (this->checkNextTrigger ? this->nextQuest : nullptr); }
		__inline const std::string& getQuestName() const { return this->questName; }

		__inline const dword_t getConditionCount() const { return this->conditions.size(); }
		__inline const dword_t getActionCount() const { return this->actions.size(); }
		__inline const FixedArray<Trackable<char>>& getConditions() const { return this->conditions; }
		__inline const FixedArray<Trackable<char>>& getActions() const { return this->actions; }
};

class PlayerQuest {
	public:
		const static byte_t QUEST_VAR_MAX = 10;
		const static byte_t QUEST_ITEMS_MAX = 6;
	private:
		FixedArray<WORD> questVars;
		FixedArray<Item> questItems;
		word_t questId;
		dword_t passedTime;
		union {
			dword_t lever;
			byte_t leverBYTE[4];
		};
		byte_t slotId;
	public:
		PlayerQuest(const byte_t slotId) {
			this->slotId = slotId;
			this->questId = 0x00;
			this->passedTime = 0x00;
			this->questVars.reserve(PlayerQuest::QUEST_VAR_MAX);
			for (unsigned int i = 0; i < PlayerQuest::QUEST_VAR_MAX; i++) {
				this->questVars.addValue(0x00);
			}
			this->questItems.reserve(PlayerQuest::QUEST_ITEMS_MAX);
			for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
				this->questItems.addValue(Item());
			}
		}
		~PlayerQuest() {
			this->reset();
		}
		void reset() {
			this->questId = 0x00;
			this->passedTime = 0x00;
			this->questVars.reserve(PlayerQuest::QUEST_VAR_MAX);
			for (unsigned int i = 0; i < PlayerQuest::QUEST_VAR_MAX; i++) {
				this->questVars[i] = 0x00;
			}
			this->questItems.reserve(PlayerQuest::QUEST_ITEMS_MAX);
			for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
				this->questItems[i].clear();
			}
		}
		bool removeItemIfSufficient(const Item& item) {
			for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
				if (this->questItems[i].type == item.type && this->questItems[i].id == item.id) {
					if (this->questItems[i].amount >= item.amount) {
						this->questItems[i].amount -= item.amount;
						return true;
					}
					break;
				}
			}
			return false;
		}
		void addItem(const Item& item) { 
			for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
				if (this->questItems[i].type == item.type && this->questItems[i].id == item.id) {
					this->questItems[i].amount += item.amount;
					return;
				}
			}
			for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
				if (!this->questItems[i].isValid()) {
					this->questItems[i] = item;
					return;
				}
			}
		}

		//To make things easier
		__inline const byte_t getSlotId() const { return this->slotId; }
		__inline Item getItem(const byte_t slot) const { return this->questItems[slot]; }
		__inline word_t getVar(const byte_t slot) const { return this->questVars[slot]; }
		__inline const word_t getQuestId() const { return this->questId; }
		__inline void setQuest(const word_t questId) { this->questId = questId; }
		__inline bool isEntryFree() const { return (this->getQuestId() == 0); }

		__inline const word_t getQuestVar(const word_t varType) { return this->questVars[varType]; }
		__inline const void setQuestVar(const word_t varType, const word_t newValue) { this->questVars[varType] = newValue; }
		__inline const dword_t getPassedTime() const { return this->passedTime; }
		__inline void setPassedTime(const dword_t newTime) { this->passedTime = newTime; }

		__inline const dword_t getSwitch(const word_t switchBit) { return (this->leverBYTE[switchBit>>3] & (1 << (switchBit & 0x07))); }
		__inline const dword_t getAllSwitches() const { return this->lever; }
		void setSwitch(const word_t switchNum, const dword_t value);
};

class QuestReply {
	private:
		QuestReply() {}
		~QuestReply() {}
	public:
		const static byte_t ADD_OKAY = 0x01;
		const static byte_t ADD_FAILED = 0x02;
		const static byte_t DELETE_OKAY = 0x03;
		const static byte_t DELETE_FAILED = 0x04;
		const static byte_t TRIGGER_OKAY = 0x05;
		const static byte_t TRIGGER_FAILED = 0x06;
};

extern const dword_t makeQuestHash(const char *data);

class QuestService {
	private:
		static bool checkCondition(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool applyActions(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);

		static bool checkSelectedQuest(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkQuestVariables(QuestTrans* trans, const AbstractQuestInfo* conditionHeader);
		static bool checkUserVariables(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkItemAmount(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkPartyLeaderAndLevel(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkDistanceFromPoint(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkWorldTime(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkRemainingTime(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkSkill(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkRandomPercentage(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkObjectVar(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkEventObject(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkNPCVar(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkSwitch(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkPartyMemberCount(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkMapTime(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkNPCVarDifferences(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkServerTimeMonth(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkServerTimeWeekday(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkTeamId(QuestTrans* trans, const AbstractQuestInfo* header) { return false; } //TODO:
		static bool checkDistanceFromCenter(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkChannelNumber(QuestTrans* trans, const AbstractQuestInfo* header) { return true; } //TODO: channel num differentiation?
		static bool checkIsClanMember(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanInternalPosition(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanContribution(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanLevel(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanScore(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanMoney(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanMemberCount(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkClanSkill(QuestTrans* trans, const AbstractQuestInfo* header);
		static bool checkUNKNOWN(QuestTrans* trans, const AbstractQuestInfo* header);

		typedef bool(*CONDITION_FUNCTION_PTR)(QuestTrans* trans, const AbstractQuestInfo* header);
		static const CONDITION_FUNCTION_PTR conditions[50];

		static bool rewardNewQuest(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardAbility(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardExpMoneyOrItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardRegeneration(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardWarp(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardSpawnMonster(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardNextQuestHash(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardResetStats(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardObjectVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardNPCMessage(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestTriggerWhenTimeExpired(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardNewSkill(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestSwitch(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestClearSwitch(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardQuestClearAllSwitches(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardServerAnnouncement(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardClanLevelIncrease(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardClanMoneyChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardClanScoreChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardClanSkillChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardWarpNearbyClanMember(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardScriptTrigger(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardSkillReset(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardSingleQuestVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardUnknown33(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) { return true; /* ? */ };
		static bool rewardNPCVisuality(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardUNKNOWN(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);

		typedef bool (*REWARD_FUNCTION_PTR)(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryout);
		static const REWARD_FUNCTION_PTR rewards[50];

		static dword_t currentQuestId;
	public:

		static const dword_t runQuest(Entity* entity, const dword_t questHash);

		template<class _Ty> static _Ty rewardOperation(Entity* entity, const _Ty& basicValue, const byte_t operation);

		static const char* getAbilityTypeName(byte_t abilityType);
		static const char* operationName(byte_t operation);
		static std::string conditionToString(const char* data);
		static std::string actionToString(const char* data);
};

struct QuestTrans {
	//A work-around for the problem that a quest may be established before it gets selected during a tryout-run.
	//Nothing ever happens during this phase, yet the QSD relies on the aforementioned creation. In order to fix this issue,
	//this flag will signal whether this step (virtually) occured or not.
	bool wasPreviouslyEstablishingQuest;
	dword_t questHash;
	Entity* questTriggerCauser;
	Entity* questTarget; //e.g. player or npc
	PlayerQuest* selectedQuest;
	QuestTrans(const dword_t questHash, Entity* questCauser, Entity* target = nullptr) {
		this->questHash = questHash;
		this->questTriggerCauser = questCauser;
		this->questTarget = target;
		this->selectedQuest = nullptr;
		this->wasPreviouslyEstablishingQuest = false;
	}
};

class QSD {
	private:
		std::string filePath;
		std::string internalName;

		template<class FileType> void read(FileType& file);
		std::map<const DWORD, QuestEntry*> questData;
		word_t id;
	public:
#ifdef __ROSE_USE_VFS__
	#ifdef __ROSE_DEBUG__
			QSD(word_t newId, VFS* pVFS, const char* filePath);
	#else 
			QSD(VFS* pVFS, const char* filePath);
	#endif //__ROSE_DEBUG__
#else
		QSD(const char* filePath);
#endif
		__inline std::string getInternalName() const { return this->internalName; }
		__inline std::map<const DWORD, QuestEntry*>& getQuests() { return this->questData; }
};


struct QuestCheckVar {
	union {
		dword_t notUsed_CHECKTYPE;
		struct {
			word_t varNum;
			word_t varType;
		};
	};
	//union {
	//	dword_t notUsed_AMOUNT;
	word_t amount;
	//};
	byte_t operation;
};

struct QuestCheckAbility {
	dword_t abilityType;
	dword_t amount;
	byte_t operation;
};

struct QuestCheckItem {
	dword_t itemId;
	dword_t itemSlot;
	dword_t amount;
	byte_t operation;
};

typedef QuestCheckVar QuestSetVar;

struct QuestCondition000 {
	QuestHeaderInfo header;
	dword_t questId;
};

struct QuestCondition001 {
	QuestHeaderInfo header;
	dword_t dataCount;
	QuestCheckVar data[1];
};

typedef QuestCondition001 QuestCondition002;


struct QuestCondition003 {
	QuestHeaderInfo header;
	dword_t dataCount;
	QuestCheckAbility data[1];
}; 

struct QuestCondition004 {
	QuestHeaderInfo header;
	dword_t dataCount;
	QuestCheckItem data[1];
};

//Only trigger if the quest taker is a leader (for instnnce
struct QuestCondition005 {
	QuestHeaderInfo header;
	byte_t isLeader;
	dword_t level;
	byte_t reverse;
};

//Trigger within a certain radius (e.g. party teleport)
struct QuestCondition006 {
	QuestHeaderInfo header;
	dword_t mapId;
	dword_t x;
	dword_t y;
	dword_t z;
	dword_t radius;
};

struct QuestCondition007 {
	QuestHeaderInfo header;
	dword_t startTime;
	dword_t endTime;
};

struct QuestCondition008 {
	QuestHeaderInfo header;
	dword_t totalTime;
	byte_t operation;
};

struct QuestCondition009 {
	QuestHeaderInfo header;
	dword_t skillIdFirst;
	dword_t skillIdSecond;
	byte_t operation;
};

//PercentageLow <= RANDOM_CHANGE <= PercentageHigh
struct QuestCondition010 {
	QuestHeaderInfo header;
	byte_t percentageLow;
	byte_t percentageHigh;
};

struct QuestCondition011 {
	QuestHeaderInfo header;
	byte_t who; //0 = NPC, 1 = Player?
	union {
		word_t notUsed_VARNUM;
		byte_t varNum;
	};
	dword_t amount;
	byte_t operation;
};

struct QuestCondition012 {
	QuestHeaderInfo header;
	word_t mapId;
	dword_t x;
	dword_t y;
	dword_t eventId;
};

struct QuestCondition013 {
	QuestHeaderInfo header;
	union {
		dword_t notUsed_NPCID;
		word_t npcId;
	};
};

struct QuestCondition014 {
	QuestHeaderInfo header;
	word_t switchNum;
	byte_t isSwitchOn;
};

//If firstNum <= RANDOM_NUM <= secondNum
struct QuestCondition015 {
	QuestHeaderInfo header;
	word_t firstNum;
	word_t secondNum;
};

struct QuestCondition016 {
	QuestHeaderInfo header;
	byte_t who; //0 = NPC; 1 = Event (?); 2 = Player
	dword_t timeStart;
	dword_t timeEnd;
};

struct QuestNPCVar {
	union {
		dword_t notUsed_NPCID;
		word_t npcId;
	};
	union {
		word_t notUsed_VARTYPE;
		byte_t varType;
	};
};

struct QuestCondition017 {
	QuestHeaderInfo header;
	QuestNPCVar firstVar;
	QuestNPCVar secondVar;
	byte_t operation;
};

struct QuestCondition018 {
	QuestHeaderInfo header;

	byte_t day; //1-31

	byte_t hourStart;
	byte_t minuteStart;

	byte_t hourEnd;
	byte_t minuteEnd;
};


struct QuestCondition019 {
	QuestHeaderInfo header;

	byte_t weekDay; //0-6

	byte_t hourStart;
	byte_t minuteStart;

	byte_t hourEnd;
	byte_t minuteEnd;
};

//FOR PVP
typedef QuestCondition015 QuestCondition020; 

struct QuestCondition021 {
	QuestHeaderInfo header;
	byte_t selectedObjType;
	dword_t radius;
};

struct QuestCondition022 {
	QuestHeaderInfo header;
	word_t x;
	word_t y;
};

struct QuestCondition023 {
	QuestHeaderInfo header;
	byte_t isRegistered;
};

struct QuestCondition024 {
	QuestHeaderInfo header;
	union {
		word_t positionType; //Teleport near a point? -- COND 024
		word_t continueType; //COND 025
		word_t gradeType; //For Clan -- COND 026
		word_t pointType; //COND 027
		word_t memberAmount; //For Party -- COND 029
	};
	byte_t operation;
};

typedef QuestCondition024 QuestCondition025;
typedef QuestCondition024 QuestCondition026;
typedef QuestCondition024 QuestCondition027;


struct QuestCondition028 {
	QuestHeaderInfo header;
	dword_t moneyAmount;
	byte_t operation;
};

//Check member count (of party?)
typedef QuestCondition024 QuestCondition029;

//SkillCheck
typedef QuestCondition009 QuestCondition030;


struct QuestReward000 {
	QuestHeaderInfo header;
	union {
		dword_t notUsed_QUESTID; //BASIC
		word_t questId;
	};
	byte_t operation;
};

struct QuestReward001 {
	QuestHeaderInfo header;
	dword_t itemId; //ItemType | itemId
	byte_t operation;
	word_t amount;
	byte_t partyOption;
};

struct QuestReward002 {
	QuestHeaderInfo header;
	dword_t varAmount;
	QuestSetVar vars[1];
};

struct QuestReward003 : public QuestReward002 {
	byte_t partyOption;
};

typedef QuestReward002 QuestReward004;

//Add item to inventory [maybe also for party?]
struct QuestReward005 {
	QuestHeaderInfo header;
	byte_t rewardType;
	byte_t equate;
	dword_t amount;
	dword_t itemId; //ItemType | itemId
	byte_t partyOption;
	word_t itemOption; //???
};

//Heal for a certain percentage (also for party)
struct QuestReward006 {
	QuestHeaderInfo header;
	dword_t percentHP;
	dword_t percentMP;
	byte_t partyOption;
};

struct QuestReward007 {
	QuestHeaderInfo header;
	union {
		dword_t notUsed_MAPID;
		word_t mapId;
	};
	dword_t x;
	dword_t y;
	byte_t partyOption;
};

struct QuestReward008 {
	QuestHeaderInfo header;
	dword_t monsterId;
	dword_t amount;

	byte_t targetType;

	union {
		dword_t notUsed_MAPID;
		word_t mapId;
	};
	dword_t x;
	dword_t y;

	dword_t radius;
	dword_t teamId; //e.g. friendly/enemy
};

struct QuestReward009 {
	QuestHeaderInfo header;
	word_t triggerLength;
	char triggerName[1];
};

struct QuestReward010 {
	QuestHeaderInfo header;
};

struct QuestReward011 {
	QuestHeaderInfo header;
	byte_t isTargetSelected;
	union {
		word_t notUsed_VARTYPE;
		byte_t varType;
	};
	union {
		dword_t notUsed_VALUE;
		word_t value;
	};
	byte_t operation;
};

struct QuestReward012 {
	QuestHeaderInfo header;
	byte_t messageType; //whisper/shout/announce
	union {
		dword_t stringId; //LTB
		char message[1];
	};
};

struct QuestReward013 {
	QuestHeaderInfo header;
	byte_t targetType; //0 = NPC, 1 = Player
	dword_t seconds;
	word_t triggerLength;
	char triggerName[1];
};

struct QuestReward014 {
	QuestHeaderInfo header;
	byte_t operation;
	union {
		dword_t notUsed_SKILLID;
		word_t skillId;
	};
};

struct QuestReward015 {
	QuestHeaderInfo header;
	word_t switchId;
	byte_t operation;
};

//Clear Switch
struct QuestReward016 {
	QuestHeaderInfo header;
	word_t groupId;
};

//Clear Switch
struct QuestReward017 {
	QuestHeaderInfo header;
}; 

struct QuestReward018 {
	QuestHeaderInfo header;
	dword_t stringId;

	word_t dataCount;
	char data[1];
};

struct QuestReward019 {
	QuestHeaderInfo header;

	word_t mapId;
	word_t teamId;
	word_t triggerLength;
	char triggerName[1];

	dword_t hashOfNextTrigger;
};

struct QuestReward020 {
	QuestHeaderInfo header;
	byte_t pvpType;
};

struct QuestReward021 {
	QuestHeaderInfo header;
	dword_t x;
	dword_t y;
};

struct QuestReward022 {
	QuestHeaderInfo header;
	word_t mapId;
	byte_t operation; //0 = off, 1 = on, 2 = toggle (?)
};

struct QuestReward023 {
	QuestHeaderInfo header;
};

struct QuestReward024 {
	QuestHeaderInfo header;
	dword_t moneyAmount;
	byte_t operation;
};

struct QuestReward025 {
	QuestHeaderInfo header;
	word_t pointType;
	byte_t operation;
};

struct QuestReward026 {
	QuestHeaderInfo header;
	word_t skillId;
	byte_t operation;
};

struct QuestReward027 {
	QuestHeaderInfo header;
	word_t contType;
	byte_t operation;
};

struct QuestReward028 {
	QuestHeaderInfo header;
	dword_t range;
	word_t mapId;
	dword_t x;
	dword_t y;
};

struct QuestReward029 {
	QuestHeaderInfo header;
	word_t scriptLength;
	char script[1];
};

struct QuestReward030 {
	QuestHeaderInfo header;
};

struct QuestReward031 {
	QuestHeaderInfo header;
	dword_t monsterId;
	dword_t compareValue;
	QuestSetVar var;
};

struct QuestReward032 {
	QuestHeaderInfo header;
	dword_t itemId;
	dword_t compareAmount;
	byte_t partyOption;
};

struct QuestReward033 {
	QuestHeaderInfo header;
	word_t unknown;
};

struct QuestReward034 {
	QuestHeaderInfo header;
	byte_t unknown;
};



#endif //__ROSE_QSD__