#pragma once

#ifndef __ROSE_QSD__
#define __ROSE_QSD__

#include <string>
#include <map>
#include "..\..\Common\Definitions.h"
#include "..\..\QuickInfos\QuickInfo.h"
#include "STB.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

class Entity;

struct QuestHeader {
	const DWORD operationType;
};

struct QuestTrans;

class PlayerQuest {
	public:
		const static BYTE QUEST_VAR_MAX = 10;
		const static BYTE QUEST_ITEMS_MAX = 5;
	private:
		FixedArray<WORD> questVars;
		FixedArray<Item> questItems;
		QuestEntry* entry;
		DWORD passedTime;
		union {
			DWORD switchDWORD;
			BYTE switchBYTE[4];
		};
	public:
		PlayerQuest(QuestEntry* newEntry) {
			this->entry = newEntry;
			this->passedTime = 0x00;
			this->questVars.reserve(PlayerQuest::QUEST_VAR_MAX);
			this->questItems.reserve(PlayerQuest::QUEST_ITEMS_MAX);
		}
		~PlayerQuest() {
			this->entry = nullptr;
		}
		__inline Item getItem(const BYTE slot) const { return this->questItems[slot]; }
		__inline Item getVar(const BYTE slot) const { return this->questVars[slot]; }
		__inline QuestEntry* getQuest() const { return this->entry; }
		__inline void setQuest(QuestEntry* newQuest) { this->entry = newQuest; }
		__inline bool isEntryFree() const { return (this->getQuest() == nullptr); }

		__inline const DWORD getQuestId() const { return this->entry->getQuestId(); }
		__inline const DWORD getQuestHash() const { return this->entry->getQuestHash(); }
		__inline const WORD getQuestVar(const WORD varType) { return this->questVars[varType]; }
		__inline const void setQuestVar(const WORD varType, const WORD newValue) { this->questVars[varType] = newValue; }
		__inline const DWORD getPassedTime() const { return this->passedTime; }
		__inline const DWORD getSwitch() const { return this->switchDWORD; }
};

class QuestService {
	private:
		static bool checkCondition(QuestTrans* trans, const QuestHeader* header);
		static void applyActions(QuestTrans* trans, const QuestHeader* header);

		static bool checkSelectedQuest(QuestTrans* trans, const QuestHeader* header);
		static bool checkQuestVariables(QuestTrans* trans, const QuestHeader* conditionHeader);
		static bool checkUserVariables(QuestTrans* trans, const QuestHeader* header);
		static bool checkItemAmount(QuestTrans* trans, const QuestHeader* header);
		static bool checkPartyLeaderAndLevel(QuestTrans* trans, const QuestHeader* header);
		static bool checkDistanceFromPoint(QuestTrans* trans, const QuestHeader* header);
		static bool checkWorldTime(QuestTrans* trans, const QuestHeader* header);
		static bool checkRemainingTime(QuestTrans* trans, const QuestHeader* header);
		static bool checkSkill(QuestTrans* trans, const QuestHeader* header);
		static bool checkRandomPercentage(QuestTrans* trans, const QuestHeader* header);
		static bool checkObjectVar(QuestTrans* trans, const QuestHeader* header);
		static bool checkEventObject(QuestTrans* trans, const QuestHeader* header);
		static bool checkNPCVar(QuestTrans* trans, const QuestHeader* header);
		static bool checkSwitch(QuestTrans* trans, const QuestHeader* header);
		static bool checkPartyMemberCount(QuestTrans* trans, const QuestHeader* header); 
		static bool checkMapTime(QuestTrans* trans, const QuestHeader* header);
		static bool checkNPCVarDifferences(QuestTrans* trans, const QuestHeader* header);
		static bool checkServerTimeMonth(QuestTrans* trans, const QuestHeader* header);
		static bool checkServerTimeWeekday(QuestTrans* trans, const QuestHeader* header);
		static bool checkTeamId(QuestTrans* trans, const QuestHeader* header) { return false; } //TODO:
		static bool checkDistanceFromCenter(QuestTrans* trans, const QuestHeader* header);
		static bool checkChannelNumber(QuestTrans* trans, const QuestHeader* header) { return true; } //TODO: channel num differentiation?
		static bool checkIsClanMember(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanInternalPosition(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanContribution(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanLevel(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanScore(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanMoney(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanMemberCount(QuestTrans* trans, const QuestHeader* header);
		static bool checkClanSkill(QuestTrans* trans, const QuestHeader* header);

		static void rewardNewQuest(QuestTrans* trans, const QuestHeader* header);

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
		static bool runQuest(Entity* entity, const DWORD questHash);
		static const DWORD makeQuestHash(const char* str);
		
		template<class _Ty1, class _Ty2> static bool checkOperation(_Ty1& first, _Ty2& second, BYTE operation);
		template<class _Ty> static _Ty resultOperation(_Ty& first, _Ty& second, BYTE operation);

		static std::string conditionToString(const char* data);
		static std::string actionToString(const char* data);
};

class PlayerQuest;

class QuestEntry {
	private:
		friend class QSD;
		friend class QuestService;
		DWORD questHash;
		DWORD questId;

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
		__inline const DWORD getQuestId() const { return this->questId; }
		__inline const QuestEntry* getPreviousQuest() const { return this->previousQuest; }
		__inline const QuestEntry* getNextQuest() const { return this->nextQuest; }
};

struct QuestTrans {
	Entity* questTriggerCauser;
	Entity* questTarget; //e.g. player or npc
	PlayerQuest* selectedQuest;
	QuestTrans(Entity* questCauser, Entity* target = nullptr) {
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
		DWORD id;
	public:
	#ifdef __ROSE_USE_VFS__
		QSD(const DWORD id, VFS* pVFS, const char* filePath);
	#else
		QSD(const DWORD id, const char* filePath);
	#endif
		__inline std::string getInternalName() const { return this->internalName; }
		static const DWORD makeQuestHash(const char* questName);
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
	const DWORD abilityType;
	const DWORD amount;
	const BYTE operation;
};

struct QuestCheckItem {
	const DWORD itemId;
	const DWORD itemSlot;
	const DWORD amount;
	const BYTE operation;
};

struct QuestSetVar {
	union {
		const DWORD type;
		struct {
			const WORD varNum;
			const WORD varType;
		};
	};
	const WORD value;
	const BYTE operation;
};

struct QuestCondition000 {
	const QuestHeader header;
	const DWORD questId;
};

struct QuestCondition001 {
	const QuestHeader header;
	const DWORD dataCount;
	const QuestCheckVar* data;
};

typedef QuestCondition001 QuestCondition002;


struct QuestCondition003 {
	const QuestHeader header;
	const DWORD dataCount;
	const QuestCheckAbility* data;
}; 

struct QuestCondition004 {
	const QuestHeader header;
	const DWORD dataCount;
	const QuestCheckItem* data;
};

//Only trigger if the quest taker is a leader (for instnnce
struct QuestCondition005 {
	const QuestHeader header;
	const BYTE isLeader;
	const DWORD level;
	const BYTE reverse;
};

//Trigger within a certain radius (e.g. party teleport)
struct QuestCondition006 {
	const QuestHeader header;
	const DWORD mapId;
	const DWORD x;
	const DWORD y;
	const DWORD z;
	const DWORD radius;
};

struct QuestCondition007 {
	const QuestHeader header;
	const DWORD startTime;
	const DWORD endTime;
};

struct QuestCondition008 {
	const QuestHeader header;
	const DWORD totalTime;
	const BYTE operation;
};

struct QuestCondition009 {
	const QuestHeader header;
	const DWORD skillIdFirst;
	const DWORD skillIdSecond;
	const BYTE operation;
};

//PercentageLow <= RANDOM_CHANGE <= PercentageHigh
struct QuestCondition010 {
	const QuestHeader header;
	const BYTE percentageLow;
	const BYTE percentageHigh;
};

struct QuestCondition011 {
	const QuestHeader header;
	const BYTE who; //0 = NPC, 1 = Player?
	const WORD varNum;
	const DWORD amount;
	const BYTE operation;
};

struct QuestCondition012 {
	const QuestHeader header;
	const WORD mapId;
	const DWORD x;
	const DWORD y;
	const DWORD eventId;
};

struct QuestCondition013 {
	const QuestHeader header;
	const DWORD npcId;
};

struct QuestCondition014 {
	const QuestHeader header;
	const WORD questId;
	const BYTE onOrOff;
};

//If firstNum <= RANDOM_NUM <= secondNum
struct QuestCondition015 {
	const QuestHeader header;
	const WORD firstNum;
	const WORD secondNum;
};

struct QuestCondition016 {
	const QuestHeader header;
	const BYTE who; //0 = NPC; 1 = Event (?); 2 = Player
	const DWORD timeStart;
	const DWORD timeEnd;
};

struct QuestNPCVar {
	const DWORD npcId;
	const WORD varType;
};

struct QuestCondition017 {
	const QuestHeader header;
	const QuestNPCVar firstVar;
	const QuestNPCVar secondVar;
	const BYTE operation;
};

struct QuestCondition018 {
	const QuestHeader header;

	const BYTE day; //1-31

	const BYTE hourStart;
	const BYTE minuteStart;

	const BYTE hourEnd;
	const BYTE minuteEnd;
};


struct QuestCondition019 {
	const QuestHeader header;

	const BYTE weekDay; //0-6

	const BYTE hourStart;
	const BYTE minuteStart;

	const BYTE hourEnd;
	const BYTE minuteEnd;
};

//FOR PVP
typedef QuestCondition015 QuestCondition020; 

struct QuestCondition021 {
	const QuestHeader header;
	const BYTE selectedObjType;
	const DWORD radius;
};

struct QuestCondition022 {
	const QuestHeader header;
	const WORD x;
	const WORD y;
};

struct QuestCondition023 {
	const QuestHeader header;
	const BYTE isRegistered;
};

struct QuestCondition024 {
	const QuestHeader header;
	union {
		const WORD positionType; //Teleport near a point? -- COND 024
		const WORD continueType; //COND 025
		const WORD gradeType; //For Clan -- COND 026
		const WORD pointType; //COND 027
		const WORD memberAmount; //For Party -- COND 029
	};
	const BYTE operation;
};

typedef QuestCondition024 QuestCondition025;
typedef QuestCondition024 QuestCondition026;
typedef QuestCondition024 QuestCondition027;


struct QuestCondition028 {
	const QuestHeader header;
	const DWORD moneyAmount;
	const BYTE operation;
};

//Check member count (of party?)
typedef QuestCondition024 QuestCondition029;

//SkillCheck
typedef QuestCondition009 QuestCondition030;


struct QuestReward000 {
	const QuestHeader header;
	const DWORD questId; //BASIC
	const BYTE operation;
};

struct QuestReward001 {
	const QuestHeader header;
	const DWORD itemId; //ItemType | itemId
	const BYTE operation;
	const WORD amount;
	const BYTE partyOption;
};

struct QuestReward002 {
	const QuestHeader header;
	const DWORD varAmount;
	const QuestSetVar *vars;
};

struct QuestReward003 : public QuestReward002 {
	const BYTE partyOption;
};

typedef QuestReward002 QuestReward004;

//Add item to inventory [maybe also for party?]
struct QuestReward005 {
	const QuestHeader header;
	const BYTE targetType;
	const BYTE equate;
	const DWORD amount;
	const DWORD itemId; //ItemType | itemId
	const BYTE partyOption;
	const WORD itemOption; //???
};

//Heal for a certain percentage (also for party)
struct QuestReward006 {
	const QuestHeader header;
	const DWORD percentHP;
	const DWORD percentMP;
	const BYTE partyOption;
};

struct QuestReward007 {
	const QuestHeader header;
	const DWORD mapId;
	const DWORD x;
	const DWORD y;
	const BYTE partyOption;
};

struct QuestReward008 {
	const QuestHeader header;
	const DWORD monsterId;
	const DWORD amount;

	const BYTE targetType;

	const DWORD mapId;
	const DWORD x;
	const DWORD y;

	const DWORD radius;
	const DWORD teamId; //e.g. friendly/enemy
};

struct QuestReward009 {
	const QuestHeader header;
	const WORD triggerLength;
	const char* triggerName;
};

struct QuestReward010 {
	const QuestHeader header;
};

struct QuestReward011 {
	const QuestHeader header;
};

struct QuestReward012 {
	const QuestHeader header;
	const BYTE messageType; //whisper/shout/announce
	union {
		const DWORD stringId; //LTB
		const char *message;
	};
};

struct QuestReward013 {
	const QuestHeader header;
	const BYTE targetType; //0 = NPC, 1 = Player
	const DWORD seconds;
	const WORD triggerLength;
	const char *triggerName;
	
	const DWORD hashOfNextTrigger;
};

struct QuestReward014 {
	const QuestHeader header;
	const BYTE operation;
	const DWORD skillId;
};

struct QuestReward015 {
	const QuestHeader header;
	const WORD switchId;
	const BYTE operation;
};

//Clear Switch
struct QuestReward016 {
	const QuestHeader header;
	const WORD groupId;
};

//Clear Switch
struct QuestReward017 {
	const QuestHeader header;
}; 

struct QuestReward018 {
	const QuestHeader header;
	const DWORD stringId;

	const WORD dataCount;
	const char *data;
};

struct QuestReward019 {
	const QuestHeader header;

	const WORD mapId;
	const WORD teamId;
	const WORD triggerLength;
	const char* triggerName;

	const DWORD hashOfNextTrigger;
};

struct QuestReward020 {
	const QuestHeader header;
	const BYTE pvpType;
};

struct QuestReward021 {
	const QuestHeader header;
	const DWORD x;
	const DWORD y;
};

struct QuestReward022 {
	const QuestHeader header;
	const WORD mapId;
	const BYTE operation; //0 = off, 1 = on, 2 = toggle (?)
};

struct QuestReward023 {
	const QuestHeader header;
};

struct QuestReward024 {
	const QuestHeader header;
	const DWORD moneyAmount;
	const BYTE operation;
};

struct QuestReward025 {
	const QuestHeader header;
	const WORD pointType;
	const BYTE operation;
};

struct QuestReward026 {
	const QuestHeader header;
	const WORD skillId;
	const BYTE operation;
};

struct QuestReward027 {
	const QuestHeader header;
	const WORD contType;
	const BYTE operation;
};

struct QuestReward028 {
	const QuestHeader header;
	const DWORD range;
	const WORD mapId;
	const DWORD x;
	const DWORD y;
};

struct QuestReward029 {
	const QuestHeader header;
	const WORD scriptLength;
	const char* script;
};

struct QuestReward030 {
	const QuestHeader header;
};

struct QuestReward031 {
	const QuestHeader header;
	const DWORD monsterId;
	const DWORD compareValue;
	const QuestSetVar var;
};

struct QuestReward032 {
	const QuestHeader header;
	const DWORD itemId;
	const DWORD compareAmount;
	const BYTE partyOption;
};

struct QuestReward033 {
	const QuestHeader header;
	const WORD unknown;
};

struct QuestReward034 {
	const QuestHeader header;
	const BYTE unknown;
};



#endif //__ROSE_QSD__