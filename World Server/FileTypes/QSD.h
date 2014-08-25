#pragma once

#ifndef __ROSE_QSD__
#define __ROSE_QSD__

#include <string>
#include <map>
#include "..\..\Common\Definitions.h"
#include "..\..\QuickInfos\QuickInfo.h"
#include "..\Structures.h"
#include "STB.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

class Entity;

struct QuestHeaderInfo {
	const DWORD length;
	const DWORD operationType;
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
		BYTE checkNextTrigger;

#ifdef __ROSE_DEBUG__
		std::string qsdName;
		QSD* parent;
#endif
		std::string questName;
		DWORD questHash;
		WORD questId;

		FixedArray<Trackable<char>> conditions;
		FixedArray<Trackable<char>> actions;

		QuestEntry* previousQuest;
		QuestEntry* nextQuest;

		__inline void addCondition(Trackable<char>* newCond) { this->conditions.addValue(*newCond); }
		__inline void addAction(Trackable<char>* newAction) { this->actions.addValue(*newAction); }
	public:
		QuestEntry(const DWORD newQuestHash, const DWORD conditionCount, const DWORD actionCount) {
			this->questHash = newQuestHash;

			this->conditions.reserve(conditionCount);
			this->actions.reserve(actionCount);
		}
		__inline const DWORD getQuestHash() const { return this->questHash; }
		__inline const WORD getQuestId() const { return this->questId; }
		__inline QuestEntry* getPreviousQuest() const { return this->previousQuest; }
		__inline QuestEntry* getNextQuest() const { return (this->checkNextTrigger ? this->nextQuest : nullptr); }
		__inline const std::string& getQuestName() const { return this->questName; }

		__inline const DWORD getConditionCount() const { return this->conditions.size(); }
		__inline const DWORD getActionCount() const { return this->actions.size(); }
		__inline const FixedArray<Trackable<char>>& getConditions() const { return this->conditions; }
		__inline const FixedArray<Trackable<char>>& getActions() const { return this->actions; }
};

class PlayerQuest {
	public:
		const static BYTE QUEST_VAR_MAX = 10;
		const static BYTE QUEST_ITEMS_MAX = 6;
	private:
		FixedArray<WORD> questVars;
		FixedArray<Item> questItems;
		WORD questId;
		DWORD passedTime;
		union {
			DWORD subSwitch;
			BYTE subSwitchBYTE[4];
		};
	public:
		PlayerQuest(const WORD questId) {
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
		__inline Item getItem(const BYTE slot) const { return this->questItems[slot]; }
		__inline WORD getVar(const BYTE slot) const { return this->questVars[slot]; }
		__inline const WORD getQuestId() const { return this->questId; }
		__inline void setQuest(const WORD questId) { this->questId = questId; }
		__inline bool isEntryFree() const { return (this->getQuestId() == 0); }

		__inline const WORD getQuestVar(const WORD varType) { return this->questVars[varType]; }
		__inline const void setQuestVar(const WORD varType, const WORD newValue) { this->questVars[varType] = newValue; }
		__inline const DWORD getPassedTime() const { return this->passedTime; }
		__inline const DWORD getSubSwitch() const { return this->subSwitch; }
};

class QuestReply {
	private:
		QuestReply() {}
		~QuestReply() {}
	public:
		const static BYTE DELETE_OKAY = 0x03;
		const static BYTE DELETE_FAILED = 0x04;
		const static BYTE TRIGGER_OKAY = 0x05;
		const static BYTE TRIGGER_FAILED = 0x06;
};

extern const DWORD makeQuestHash(const char *data);

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
		static bool rewardUnknown29(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardSkillReset(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardSingleQuestVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);
		static bool rewardUnknown33(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) { return true; /* ? */ };
		static bool rewardNPCVisuality(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun);

		const static BYTE OPERATION_EQUAL = 0x00;
		const static BYTE OPERATION_BIGGER = 0x01;
		const static BYTE OPERATION_BIGGER_EQUAL = 0x02;
		const static BYTE OPERATION_SMALLER = 0x03;
		const static BYTE OPERATION_SMALLER_EQUAL = 0x04;
		const static BYTE OPERATION_RETURN_RHS = 0x05;
		const static BYTE OPERATION_ADDITION = 0x06;
		const static BYTE OPERATION_SUBTRACTION = 0x07;
		const static BYTE OPERATION_MULTIPLICATION = 0x08;
		const static BYTE OPERATION_DIVISION = 0x09;
		const static BYTE OPERATION_NOT_EQUAL = 0x0A;

		static DWORD currentQuestId;
	public:
		static bool runQuest(Entity* entity, const DWORD questHash, bool isTryoutRun = false);

		template<class _Ty1, class _Ty2> static bool checkOperation(_Ty1& first, const _Ty2& second, const BYTE operation);
		template<class _Ty> static _Ty resultOperation(_Ty first, const _Ty& second, const BYTE operation);
		template<class _Ty> static _Ty rewardOperation(Entity* entity, const _Ty& basicValue, const BYTE operation);

		static const char* getAbilityTypeName(BYTE abilityType);
		static const char* operationName(BYTE operation);
		static std::string conditionToString(const char* data);
		static std::string actionToString(const char* data);
};

struct QuestTrans {
	DWORD questHash;
	Entity* questTriggerCauser;
	Entity* questTarget; //e.g. player or npc
	PlayerQuest* selectedQuest;
	QuestTrans(const DWORD questHash, Entity* questCauser, Entity* target = nullptr) {
		this->questHash = questHash;
		this->questTriggerCauser = questCauser;
		this->questTarget = target;
		this->selectedQuest = nullptr;
	}
};

class QSD {
	private:
		std::string filePath;
		std::string internalName;

		template<class FileType> void read(FileType& file);
		std::map<const DWORD, QuestEntry*> questData;
		WORD id;
	public:
#ifdef __ROSE_USE_VFS__
	#ifdef __ROSE_DEBUG__
			QSD(WORD newId, VFS* pVFS, const char* filePath);
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
	DWORD checkType;
	WORD varNum;
	WORD varType;
	WORD amount;
	BYTE operation;
};

struct QuestCheckAbility {
	DWORD abilityType;
	DWORD amount;
	BYTE operation;
};

struct QuestCheckItem {
	DWORD itemId;
	DWORD itemSlot;
	DWORD amount;
	BYTE operation;
};

struct QuestSetVar {
	union {
		DWORD type;
		struct {
			WORD varNum;
			WORD varType;
		};
	};
	union {
		DWORD dwValue;
		WORD value;
	};
	BYTE operation;
};

struct QuestCondition000 {
	QuestHeaderInfo header;
	DWORD questId;
};

struct QuestCondition001 {
	QuestHeaderInfo header;
	DWORD dataCount;
	QuestCheckVar data[1];
};

typedef QuestCondition001 QuestCondition002;


struct QuestCondition003 {
	QuestHeaderInfo header;
	DWORD dataCount;
	QuestCheckAbility data[1];
}; 

struct QuestCondition004 {
	QuestHeaderInfo header;
	DWORD dataCount;
	QuestCheckItem data[1];
};

//Only trigger if the quest taker is a leader (for instnnce
struct QuestCondition005 {
	QuestHeaderInfo header;
	BYTE isLeader;
	DWORD level;
	BYTE reverse;
};

//Trigger within a certain radius (e.g. party teleport)
struct QuestCondition006 {
	QuestHeaderInfo header;
	DWORD mapId;
	DWORD x;
	DWORD y;
	DWORD z;
	DWORD radius;
};

struct QuestCondition007 {
	QuestHeaderInfo header;
	DWORD startTime;
	DWORD endTime;
};

struct QuestCondition008 {
	QuestHeaderInfo header;
	DWORD totalTime;
	BYTE operation;
};

struct QuestCondition009 {
	QuestHeaderInfo header;
	DWORD skillIdFirst;
	DWORD skillIdSecond;
	BYTE operation;
};

//PercentageLow <= RANDOM_CHANGE <= PercentageHigh
struct QuestCondition010 {
	QuestHeaderInfo header;
	BYTE percentageLow;
	BYTE percentageHigh;
};

struct QuestCondition011 {
	QuestHeaderInfo header;
	BYTE who; //0 = NPC, 1 = Player?
	union {
		WORD wVarNum;
		BYTE varNum;
	};
	DWORD amount;
	BYTE operation;
};

struct QuestCondition012 {
	QuestHeaderInfo header;
	WORD mapId;
	DWORD x;
	DWORD y;
	DWORD eventId;
};

struct QuestCondition013 {
	QuestHeaderInfo header;
	union {
		DWORD dwNpcId;
		WORD npcId;
	};
};

struct QuestCondition014 {
	QuestHeaderInfo header;
	WORD switchNum;
	BYTE isSwitchOn;
};

//If firstNum <= RANDOM_NUM <= secondNum
struct QuestCondition015 {
	QuestHeaderInfo header;
	WORD firstNum;
	WORD secondNum;
};

struct QuestCondition016 {
	QuestHeaderInfo header;
	BYTE who; //0 = NPC; 1 = Event (?); 2 = Player
	DWORD timeStart;
	DWORD timeEnd;
};

struct QuestNPCVar {
	union {
		DWORD dwNpcId;
		WORD npcId;
	};
	union {
		WORD wVarType;
		BYTE varType;
	};
};

struct QuestCondition017 {
	QuestHeaderInfo header;
	QuestNPCVar firstVar;
	QuestNPCVar secondVar;
	BYTE operation;
};

struct QuestCondition018 {
	QuestHeaderInfo header;

	BYTE day; //1-31

	BYTE hourStart;
	BYTE minuteStart;

	BYTE hourEnd;
	BYTE minuteEnd;
};


struct QuestCondition019 {
	QuestHeaderInfo header;

	BYTE weekDay; //0-6

	BYTE hourStart;
	BYTE minuteStart;

	BYTE hourEnd;
	BYTE minuteEnd;
};

//FOR PVP
typedef QuestCondition015 QuestCondition020; 

struct QuestCondition021 {
	QuestHeaderInfo header;
	BYTE selectedObjType;
	DWORD radius;
};

struct QuestCondition022 {
	QuestHeaderInfo header;
	WORD x;
	WORD y;
};

struct QuestCondition023 {
	QuestHeaderInfo header;
	BYTE isRegistered;
};

struct QuestCondition024 {
	QuestHeaderInfo header;
	union {
		WORD positionType; //Teleport near a point? -- COND 024
		WORD continueType; //COND 025
		WORD gradeType; //For Clan -- COND 026
		WORD pointType; //COND 027
		WORD memberAmount; //For Party -- COND 029
	};
	BYTE operation;
};

typedef QuestCondition024 QuestCondition025;
typedef QuestCondition024 QuestCondition026;
typedef QuestCondition024 QuestCondition027;


struct QuestCondition028 {
	QuestHeaderInfo header;
	DWORD moneyAmount;
	BYTE operation;
};

//Check member count (of party?)
typedef QuestCondition024 QuestCondition029;

//SkillCheck
typedef QuestCondition009 QuestCondition030;


struct QuestReward000 {
	QuestHeaderInfo header;
	union {
		DWORD dwQuestId; //BASIC
		WORD questId;
	};
	BYTE operation;
};

struct QuestReward001 {
	QuestHeaderInfo header;
	DWORD itemId; //ItemType | itemId
	BYTE operation;
	WORD amount;
	BYTE partyOption;
};

struct QuestReward002 {
	QuestHeaderInfo header;
	DWORD varAmount;
	QuestSetVar vars[1];
};

struct QuestReward003 : public QuestReward002 {
	BYTE partyOption;
};

typedef QuestReward002 QuestReward004;

//Add item to inventory [maybe also for party?]
struct QuestReward005 {
	QuestHeaderInfo header;
	BYTE rewardType;
	BYTE equate;
	DWORD amount;
	DWORD itemId; //ItemType | itemId
	BYTE partyOption;
	WORD itemOption; //???
};

//Heal for a certain percentage (also for party)
struct QuestReward006 {
	QuestHeaderInfo header;
	DWORD percentHP;
	DWORD percentMP;
	BYTE partyOption;
};

struct QuestReward007 {
	QuestHeaderInfo header;
	union {
		DWORD dwMapId;
		WORD mapId;
	};
	DWORD x;
	DWORD y;
	BYTE partyOption;
};

struct QuestReward008 {
	QuestHeaderInfo header;
	DWORD monsterId;
	DWORD amount;

	BYTE targetType;

	union {
		DWORD dwMapId;
		WORD mapId;
	};
	DWORD x;
	DWORD y;

	DWORD radius;
	DWORD teamId; //e.g. friendly/enemy
};

struct QuestReward009 {
	QuestHeaderInfo header;
	WORD triggerLength;
	char triggerName[1];
};

struct QuestReward010 {
	QuestHeaderInfo header;
};

struct QuestReward011 {
	QuestHeaderInfo header;
	BYTE isTargetSelected;
	union {
		WORD wVarType;
		BYTE varType;
	};
	union {
		DWORD dwValue;
		WORD value;
	};
	BYTE operation;
};

struct QuestReward012 {
	QuestHeaderInfo header;
	BYTE messageType; //whisper/shout/announce
	union {
		DWORD stringId; //LTB
		char message[1];
	};
};

struct QuestReward013 {
	QuestHeaderInfo header;
	BYTE targetType; //0 = NPC, 1 = Player
	DWORD seconds;
	WORD triggerLength;
	char triggerName[1];
};

struct QuestReward014 {
	QuestHeaderInfo header;
	BYTE operation;
	union {
		DWORD dwSkillId;
		WORD skillId;
	};
};

struct QuestReward015 {
	QuestHeaderInfo header;
	WORD switchId;
	BYTE operation;
};

//Clear Switch
struct QuestReward016 {
	QuestHeaderInfo header;
	WORD groupId;
};

//Clear Switch
struct QuestReward017 {
	QuestHeaderInfo header;
}; 

struct QuestReward018 {
	QuestHeaderInfo header;
	DWORD stringId;

	WORD dataCount;
	char data[1];
};

struct QuestReward019 {
	QuestHeaderInfo header;

	WORD mapId;
	WORD teamId;
	WORD triggerLength;
	char triggerName[1];

	DWORD hashOfNextTrigger;
};

struct QuestReward020 {
	QuestHeaderInfo header;
	BYTE pvpType;
};

struct QuestReward021 {
	QuestHeaderInfo header;
	DWORD x;
	DWORD y;
};

struct QuestReward022 {
	QuestHeaderInfo header;
	WORD mapId;
	BYTE operation; //0 = off, 1 = on, 2 = toggle (?)
};

struct QuestReward023 {
	QuestHeaderInfo header;
};

struct QuestReward024 {
	QuestHeaderInfo header;
	DWORD moneyAmount;
	BYTE operation;
};

struct QuestReward025 {
	QuestHeaderInfo header;
	WORD pointType;
	BYTE operation;
};

struct QuestReward026 {
	QuestHeaderInfo header;
	WORD skillId;
	BYTE operation;
};

struct QuestReward027 {
	QuestHeaderInfo header;
	WORD contType;
	BYTE operation;
};

struct QuestReward028 {
	QuestHeaderInfo header;
	DWORD range;
	WORD mapId;
	DWORD x;
	DWORD y;
};

struct QuestReward029 {
	QuestHeaderInfo header;
	WORD scriptLength;
	char script[1];
};

struct QuestReward030 {
	QuestHeaderInfo header;
};

struct QuestReward031 {
	QuestHeaderInfo header;
	DWORD monsterId;
	DWORD compareValue;
	QuestSetVar var;
};

struct QuestReward032 {
	QuestHeaderInfo header;
	DWORD itemId;
	DWORD compareAmount;
	BYTE partyOption;
};

struct QuestReward033 {
	QuestHeaderInfo header;
	WORD unknown;
};

struct QuestReward034 {
	QuestHeaderInfo header;
	BYTE unknown;
};



#endif //__ROSE_QSD__