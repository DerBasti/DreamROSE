#pragma once

#ifndef __ROSE_AIP__
#define __ROSE_AIP__

#include <iostream>
#include <vector>
#include <string>
#include "D:\Programmieren\QuickInfos\Trackable.hpp"
#include "..\..\Common\Definitions.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

#pragma warning(disable:4996)

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#ifndef __AIP_CONDITION_CODE__
#define __AIP_CONDITION_CODE__ 0x04000000
#endif

#ifndef __AIP_ACTION_CODE__
#define __AIP_ACTION_CODE__ 0x0B000000
#endif

#define __AIP_CONDITION_TYPE__(id) AICOND_##(id - __AIP_CONDITION_CODE__ - 0x01)
#define __AIP_ACTION_TYPE__(id) AICOND_##(id - __AIP_ACTION_CODE__ - 0x01)

#define __AIP_CONDITION_CREATION__(id) (__AIP_CONDITION_CODE__ | (id+0x01))
#define __AIP_ACTION_CREATION__(id) (__AIP_ACTION_CODE__ | (id+0x01))

#define __BASIC_AI_HEADER__ DWORD _size; DWORD type;

struct BasicAIP {
	private:
		__BASIC_AI_HEADER__;
	public:
		BasicAIP() {
			_size = 0x00;
			type = __AIP_CONDITION_CODE__;
		}
		BasicAIP(DWORD nSize, DWORD nType) {
			this->_size = nSize;
			this->type = nType;
		}
#pragma region Condition Codes
		const static DWORD __AIP_CONDITION_00__ = __AIP_CONDITION_CREATION__(0);
		const static DWORD __AIP_CONDITION_01__ = __AIP_CONDITION_CREATION__(1);
		const static DWORD __AIP_CONDITION_02__ = __AIP_CONDITION_CREATION__(2);
		const static DWORD __AIP_CONDITION_03__ = __AIP_CONDITION_CREATION__(3);
		const static DWORD __AIP_CONDITION_04__ = __AIP_CONDITION_CREATION__(4);
		const static DWORD __AIP_CONDITION_05__ = __AIP_CONDITION_CREATION__(5);
		const static DWORD __AIP_CONDITION_06__ = __AIP_CONDITION_CREATION__(6);
		const static DWORD __AIP_CONDITION_07__ = __AIP_CONDITION_CREATION__(7);
		const static DWORD __AIP_CONDITION_08__ = __AIP_CONDITION_CREATION__(8);
		const static DWORD __AIP_CONDITION_09__ = __AIP_CONDITION_CREATION__(9);
		const static DWORD __AIP_CONDITION_10__ = __AIP_CONDITION_CREATION__(10);
		const static DWORD __AIP_CONDITION_11__ = __AIP_CONDITION_CREATION__(11);
		const static DWORD __AIP_CONDITION_12__ = __AIP_CONDITION_CREATION__(12);
		const static DWORD __AIP_CONDITION_13__ = __AIP_CONDITION_CREATION__(13);
		const static DWORD __AIP_CONDITION_14__ = __AIP_CONDITION_CREATION__(14);
		const static DWORD __AIP_CONDITION_15__ = __AIP_CONDITION_CREATION__(15);
		const static DWORD __AIP_CONDITION_16__ = __AIP_CONDITION_CREATION__(16);
		const static DWORD __AIP_CONDITION_17__ = __AIP_CONDITION_CREATION__(17);
		const static DWORD __AIP_CONDITION_18__ = __AIP_CONDITION_CREATION__(18);
		const static DWORD __AIP_CONDITION_19__ = __AIP_CONDITION_CREATION__(19);
		const static DWORD __AIP_CONDITION_20__ = __AIP_CONDITION_CREATION__(20);
		const static DWORD __AIP_CONDITION_21__ = __AIP_CONDITION_CREATION__(21);
		const static DWORD __AIP_CONDITION_22__ = __AIP_CONDITION_CREATION__(22);
		const static DWORD __AIP_CONDITION_23__ = __AIP_CONDITION_CREATION__(23);
		const static DWORD __AIP_CONDITION_24__ = __AIP_CONDITION_CREATION__(24);
		const static DWORD __AIP_CONDITION_25__ = __AIP_CONDITION_CREATION__(25);
		const static DWORD __AIP_CONDITION_26__ = __AIP_CONDITION_CREATION__(26);
		const static DWORD __AIP_CONDITION_27__ = __AIP_CONDITION_CREATION__(27);
		const static DWORD __AIP_CONDITION_28__ = __AIP_CONDITION_CREATION__(28);
		const static DWORD __AIP_CONDITION_29__ = __AIP_CONDITION_CREATION__(29);
		const static DWORD __AIP_CONDITION_30__ = __AIP_CONDITION_CREATION__(30);
		const static DWORD __AIP_CONDITION_31__ = __AIP_CONDITION_CREATION__(31);
#pragma endregion
#pragma region Action Codes
		const static DWORD __AI_ACTION_00__ = __AIP_ACTION_CREATION__(0);
		const static DWORD __AI_ACTION_01__ = __AIP_ACTION_CREATION__(1);
		const static DWORD __AI_ACTION_02__ = __AIP_ACTION_CREATION__(2);
		const static DWORD __AI_ACTION_03__ = __AIP_ACTION_CREATION__(3);
		const static DWORD __AI_ACTION_04__ = __AIP_ACTION_CREATION__(4);
		const static DWORD __AI_ACTION_05__ = __AIP_ACTION_CREATION__(5);
		const static DWORD __AI_ACTION_06__ = __AIP_ACTION_CREATION__(6);
		const static DWORD __AI_ACTION_07__ = __AIP_ACTION_CREATION__(7);
		const static DWORD __AI_ACTION_08__ = __AIP_ACTION_CREATION__(8);
		const static DWORD __AI_ACTION_09__ = __AIP_ACTION_CREATION__(9);
		const static DWORD __AI_ACTION_10__ = __AIP_ACTION_CREATION__(10);
		const static DWORD __AI_ACTION_11__ = __AIP_ACTION_CREATION__(11);
		const static DWORD __AI_ACTION_12__ = __AIP_ACTION_CREATION__(12);
		const static DWORD __AI_ACTION_13__ = __AIP_ACTION_CREATION__(13);
		const static DWORD __AI_ACTION_14__ = __AIP_ACTION_CREATION__(14);
		const static DWORD __AI_ACTION_15__ = __AIP_ACTION_CREATION__(15);
		const static DWORD __AI_ACTION_16__ = __AIP_ACTION_CREATION__(16);
		const static DWORD __AI_ACTION_17__ = __AIP_ACTION_CREATION__(17);
		const static DWORD __AI_ACTION_18__ = __AIP_ACTION_CREATION__(18);
		const static DWORD __AI_ACTION_19__ = __AIP_ACTION_CREATION__(19);
		const static DWORD __AI_ACTION_20__ = __AIP_ACTION_CREATION__(20);
		const static DWORD __AI_ACTION_21__ = __AIP_ACTION_CREATION__(21);
		const static DWORD __AI_ACTION_22__ = __AIP_ACTION_CREATION__(22);
		const static DWORD __AI_ACTION_23__ = __AIP_ACTION_CREATION__(23);
		const static DWORD __AI_ACTION_24__ = __AIP_ACTION_CREATION__(24);
		const static DWORD __AI_ACTION_25__ = __AIP_ACTION_CREATION__(25);
		const static DWORD __AI_ACTION_26__ = __AIP_ACTION_CREATION__(26);
		const static DWORD __AI_ACTION_27__ = __AIP_ACTION_CREATION__(27);
		const static DWORD __AI_ACTION_28__ = __AIP_ACTION_CREATION__(28);
		const static DWORD __AI_ACTION_29__ = __AIP_ACTION_CREATION__(29);
		const static DWORD __AI_ACTION_30__ = __AIP_ACTION_CREATION__(30);
		const static DWORD __AI_ACTION_31__ = __AIP_ACTION_CREATION__(31);
		const static DWORD __AI_ACTION_32__ = __AIP_ACTION_CREATION__(32);
		const static DWORD __AI_ACTION_33__ = __AIP_ACTION_CREATION__(33);
		const static DWORD __AI_ACTION_34__ = __AIP_ACTION_CREATION__(34);
		const static DWORD __AI_ACTION_35__ = __AIP_ACTION_CREATION__(35);
		const static DWORD __AI_ACTION_36__ = __AIP_ACTION_CREATION__(36);
		const static DWORD __AI_ACTION_37__ = __AIP_ACTION_CREATION__(37);
		const static DWORD __AI_ACTION_38__ = __AIP_ACTION_CREATION__(38);
#pragma endregion

		__inline const DWORD getType() const { return this->type; }
		__inline const DWORD size() const { return this->_size; }
		const DWORD sizeInFile() const { 
			if(this->_size % 4 == 0)
				return this->_size;
			return this->_size + (4 - (this->_size % 4));
		}
};

#ifndef __MORE_OR_LESS_DEF__
#define __MORE_OR_LESS_DEF__(name) __inline bool needsMore##name() const { return this->moreOrLess == false; } \
	__inline bool needsLess##name() const { return !this->needsMore##name(); }
#endif

struct AITransfer {
	class Entity* designatedTarget;
	class Entity* nearestEntity;
	class Entity* lastFound;

	AITransfer() {
		this->designatedTarget = nearestEntity = lastFound = nullptr;
	}
};

class AIService {
	private:
		AIService() {};
		~AIService() {};

		const static BYTE ABILITY_LEVEL = 0x00;
		const static BYTE ABILITY_ATTACKPOWER = 0x01;
		const static BYTE ABILITY_DEFENSE = 0x02;
		const static BYTE ABILITY_MAGIC_DEFENSE = 0x03;
		const static BYTE ABILITY_GET_HP = 0x04;
		const static BYTE ABILITY_CHARM = 0x05;

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

		
#pragma region Condition Execution
		static bool conditionFightOrDelay();
		static bool conditionEnoughDamageReceived( class NPC* npc, const struct AICOND_01* ai);
		static bool conditionHasEnoughTargets( class NPC* npc, const struct AICOND_02* ai, AITransfer* trans);
		static bool conditionDistanceFromSpawn( class NPC* npc, const struct AICOND_03* ai);
		static bool conditionDistanceToTarget( class NPC* npc, const struct AICOND_04* ai);
		static bool conditionCheckAbilityDifference( class NPC* npc, const struct AICOND_05 *ai);
		static bool conditionCheckPercentHP(class NPC* npc, const struct AICOND_06* ai);
		static bool conditionRandomPercentageMet(const struct AICOND_07 *ai);
		static bool conditionFindNearestEligibleTarget( class NPC* npc, const struct AICOND_08* ai, AITransfer* trans);
		static bool conditionHasTargetChanged( class NPC* npc, const struct AICOND_09* ai, AITransfer* trans);
		static bool conditionCompareAbilities( class NPC* npc, const struct AICOND_10* ai, AITransfer* trans);
		static bool conditionIsStatSufficient( class NPC* npc, const struct AICOND_11* ai, AITransfer* trans);
		static bool conditionHasDaytimeArrived( class NPC* npc, const struct AICOND_12* ai);
		static bool conditionHasBuff( class NPC* npc, const struct AICOND_13* ai);
		static bool conditionIsObjectVarValid( class NPC* npc, const struct AICOND_14* ai);
		static bool conditionIsWorldVarValid(const struct AICOND_15* ai);
		static bool conditionIsEconomyVarValid(const struct AICOND_16* ai);
		static bool conditionIsNPCNearby( class NPC* npc, const struct AICOND_17* ai);
		static bool conditionCheckDistanceToOwner( class NPC* npc, const struct AICOND_18* ai);
		static bool conditionCheckZoneTime( class NPC* npc, const struct AICOND_19* ai);
		static bool conditionAreOwnStatsSufficient( class NPC* npc, const struct AICOND_20* ai);
		static bool conditionHasNoOwner( class NPC* npc, const struct AICOND_21* ai);
		static bool conditionHasOwner( class NPC* npc, const struct AICOND_22* ai);
		static bool conditionWorldTime(const struct AICOND_23* ai);
		static bool conditionWeekTime(const struct AICOND_24* ai);
		static bool conditionMonthTime(const struct AICOND_25* ai);
		static bool conditionUnknown(const struct AICOND_26* ai);
		static bool conditionLevelDiffToSurrounding(class NPC* npc, const struct AICOND_27* ai, AITransfer* trans);
		static bool conditionAIVariable(class NPC* npc, const struct AICOND_28* ai);
		static bool conditionIsTargetClanmaster(class NPC* npc, const struct AICOND_29* ai, AITransfer* trans);
		static bool conditionCreationTime(class NPC* npc, const struct AICOND_30* ai);
		static bool conditionIsCallerAvailable(class NPC* npc, AITransfer* trans);
#pragma endregion
#pragma region Action Execution
		static void actionStop(class NPC* npc);
		static void actionSetEmote(class NPC* npc, const struct AIACTION_01* act);
		static void actionSayMessage(class NPC* npc, const struct AIACTION_02* act);
		static void actionSetNewRandomPos(class NPC* npc, const struct AIACTION_03* act);
		static void actionSetNewPosFromSpawn(class NPC* npc, const struct AIACTION_04* act);
		static void actionSetPositionToFoundTarget(class NPC* npc, const struct AIACTION_05* act, AITransfer* trans);
		static void actionAttackTarget(class NPC* npc, const struct AIACTION_06* act);
		static void actionSpecialAttack(class NPC* npc);
		static void actionMoveToTarget(NPC * npc, const struct AIACTION_08* act, AITransfer* trans);
		static void actionConvert(class NPC* npc, const struct AIACTION_09* act);
		static void actionSpawnPet(class NPC* npc, const struct AIACTION_10* act);
		static void actionCallAlliesForAttack(class NPC* npc, const struct AIACTION_11 *act);
		static void actionAttackFoundTarget(class NPC* npc, AITransfer* trans);

		static void actionAttackNearestTarget(class NPC* npc, AITransfer* trans);
		static void actionCallEntireFamilyForAttack(class NPC* npc);
		static void actionAttackDesignatedTarget(class NPC* npc, AITransfer* trans);
		static void actionRunAway(class NPC* npc, const struct AIACTION_16* act);

		static void actionDropItem( const struct AIACTION_17* act);
		static void actionCallFewFamilyMembersForAttack(class NPC* npc, const struct AIACTION_18* act);
		static void actionSpawnPetAtPosition(class NPC* npc, const struct AIACTION_20* act, AITransfer *trans);
		static void actionKillNPC(class NPC* npc);

		//TODO: Implement Skills
		static void actionCastSkill(class NPC* npc, const struct AIACTION_24* act, AITransfer* trans);
		static void actionChangeNPCVar(class NPC* npc, const struct AIACTION_25* act);

		//TODO: implement World Variables
		static void actionChangeWorldVar(class NPC* npc, const struct AIACTION_26* act) {}

		//TODO: implement Economy Variables
		static void actionChangeEconomyVar(class NPC* npc, const struct AIACTION_27* act) {}

		//TODO: implement easy-to-access chat messages for NPC
		static void actionSayMessage(class NPC* npc, const struct AIACTION_28* act);
		static void actionMoveToOwner(class NPC *npc, const struct AIACTION_29* act);

		//TODO: implement questing
		static void actionSetQuestTrigger(class NPC* npc, const struct AIACTION_30* act);

		static void actionAttackOwnersTarget(class NPC* npc);
		static void actionSetMapAsPVPArea(class NPC* npc, const struct AIACTION_32* act);
		static void actionGiveItemsToOwner(class NPC* npc, const struct AIACTION_34* act);
		static void actionSetAIVar(class NPC* npc, const struct AIACTION_35* act);
#pragma endregion
	public:
		static WORD getAbilityType(BYTE abilityType, class Entity* entity);
		static const char* getAbilityTypeName(BYTE abilityType);
		static const char* operationName(BYTE operation);
		template<class _Ty1, class _Ty2> static bool checkOperation(_Ty1& first, _Ty2& second, BYTE operation);
		template<class _Ty> static _Ty resultOperation(_Ty& first, _Ty& second, BYTE operation);
		static bool run(class NPC* npcWithAI, const BYTE blockType, class Entity* target = nullptr, const DWORD damageDealt = 0x00);

		static bool checkConditions( const std::vector<Trackable<char>>& ai, class NPC* monster, AITransfer* trans );
		static void executeActions( const std::vector<Trackable<char>>& ai, class NPC* monster, AITransfer* trans );
};


struct AICOND_00 {
	private:
		__BASIC_AI_HEADER__;
		union {
			BYTE cFightOrDelay;
			char data[0x04];
		};
		friend class AIService;
	public:
		AICOND_00(BYTE fightOrDelay) {
			this->type = BasicAIP::__AIP_CONDITION_00__;
			this->_size = 0x08 + sizeof(BYTE);

			this->cFightOrDelay = fightOrDelay;
		}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf,"Wait or Attack (0x00)\n%s=====\n\n%sFightOrDelay: 0x%x\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->cFightOrDelay );
			return std::string(buf);
		}
		__inline BYTE fightOrDelay() const { return this->cFightOrDelay; }
};

struct AICOND_01 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD iDamage;
		union {
			bool cReceiveOrDeal;
			char data[0x04];
		};
	public:
		AICOND_01(DWORD damage, BYTE receiveOrDeal) {
			this->type = BasicAIP::__AIP_CONDITION_01__;
			this->_size = 0x08 + sizeof(bool) + sizeof(DWORD);

			this->iDamage = damage;
			this->cReceiveOrDeal = receiveOrDeal > 0 ? true : false;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Damage (0x01)\n%s=====\n%sDamage amount: 0x%x\n%sOnReceive: %i\n", indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDamage(), indent ? "\t\t\t" : "", this->isActionOnReceive() );
			return std::string(buf);
		}
		__inline DWORD getDamage() const { return this->iDamage; }
		__inline bool isActionOnReceive() const { return this->cReceiveOrDeal == false; }
		__inline bool isActionOnDeal() const { return !this->isActionOnReceive(); }
};

struct AICOND_02 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		DWORD distance;
		union {
			bool isAlliedEntity; //0 = false (enemy), 1 = true
			short wIsAlliedEntity;
		};
		short levelDiff[2];
		WORD entityAmount;
	public:
		const static BYTE LEVELDIFF_START = 0x00;
		const static BYTE LEVELDIFF_END = 0x01;
		AICOND_02( DWORD newDistance, bool needsAllied, short levelDiff_Start, short levelDiff_End, WORD entityAmount) {
			this->type = __AIP_CONDITION_CODE__ | 0x03;
			this->_size = 0x08 + sizeof(bool) + 0x0A;
			
			this->distance = newDistance;
			this->isAlliedEntity = needsAllied;
			this->levelDiff[AICOND_02::LEVELDIFF_START] = levelDiff_Start;
			this->levelDiff[AICOND_02::LEVELDIFF_END] = levelDiff_End;
			this->entityAmount = entityAmount;
		}
		std::string toString(bool indent = true) const {
			char buf[0x200] = {0x00};
			sprintf(buf,"Check nearby Entities (0x02)\n%s=====\n%sAllowed Distance: %f\n%sNeeds Ally: %i\n%sLevelDiff: [%i;%i]\n%sEntity amount: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(),
				indent ? "\t\t\t" : "", this->isAllyNeeded(), 
				indent ? "\t\t\t" : "", this->getLevelDiff(0x00), this->getLevelDiff(0x01),
				indent ? "\t\t\t" : "", this->getAmount());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline bool isAllyNeeded() const { return this->isAlliedEntity; }
		__inline short getLevelDiff(BYTE startOrEnd) const {
			return this->levelDiff[startOrEnd % 2];
		}
		__inline WORD getAmount() const { return this->entityAmount; }
};

struct AICOND_03 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		DWORD distance;
	public:
		AICOND_03(int newDist) {
			this->type = __AIP_CONDITION_CODE__ | 0x04;
			this->_size = 0x08 + 0x04;
			
			this->distance = newDist;
		}
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf,"Check Distance (Spawn) (0x03)\n%s=====\n%sAllowed Distance: %f\n",indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
};

struct AICOND_04 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		DWORD distance;
		BYTE moreOrLess;
	public:
		AICOND_04(DWORD newDist, BYTE cMoreOrLess) {
			this->type = __AIP_CONDITION_CODE__ | 0x05;
			this->_size = 0x08 + 0x05;
			
			this->distance = newDist;
			this->moreOrLess = cMoreOrLess;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Distance from possible Target (0x04)\n%s=====\n%sAllowed Distance: %f\n%sRequires less: %i\n",indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(),
				indent ? "\t\t\t" : "", this->needsLessDistance());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(Distance);
#else
		__inline bool needsMoreDistance() const { return this->moreOrLess == false; }
		__inline bool needsLessDistance() const { return !this->needsMoreDistance(); }
#endif
};

struct AICOND_05 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		union {
			BYTE abilityType;
			DWORD notUsed;
		};
		long difference;
		bool moreOrLess;
	public:
		AICOND_05(BYTE newAbilityType, long diff, BYTE isMoreOrLess) {
			this->type = __AIP_CONDITION_CODE__ | 0x06;
			this->_size = 0x08 + 0x05 + sizeof(bool);
			
			this->abilityType = newAbilityType;
			this->difference = diff;
			this->moreOrLess = isMoreOrLess > 0 ? true : false;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Ability (0x05)\n%s=====\n%sAbilityName: %s\n%sDifference: %i\n%sNeeds Less: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", AIService::getAbilityTypeName(this->getAbilityType()),
				indent ? "\t\t\t" : "", this->getAllowedDifference(),
				indent ? "\t\t\t" : "", this->needsLessDifference());
			return std::string(buf);
		}
		__inline BYTE getAbilityType() const { return this->abilityType; }
		__inline long getAllowedDifference() const { return this->difference; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(Difference);
#else
		__inline bool needsMoreDifference() const { return this->moreOrLess == false; }
		__inline bool needsLessDifference() const { return !this->needsMoreDifference(); }
#endif
};

struct AICOND_06 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		DWORD hp;
		bool moreOrLess;
	public:
		AICOND_06(DWORD HPToCheckFor, BYTE moreOrLess) {
			this->type = __AIP_CONDITION_CODE__ | 0x07;
			this->_size = 0x08 + 0x04 + sizeof(bool);
			
			this->hp = HPToCheckFor;
			this->moreOrLess = moreOrLess > 0 ? true : false;
		}
		std::string toString(bool indent = true) const {
			char buf[0x48] = {0x00};
			sprintf(buf,"Check HP (0x06)\n%s=====\n%sHP in Percent: %i\n%sNeedsLess: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getHPCheckMark(),
				indent ? "\t\t\t" : "", this->needsLessHP());
			return std::string(buf);
		}
		__inline DWORD getHPCheckMark() const { return this->hp; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(HP);
#else
		__inline bool needsMoreHP() const { return this->moreOrLess == false; }
		__inline bool needsLessHP() const { return !this->needsMoreHP(); }
#endif
};

struct AICOND_07 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		BYTE percent;
	public:
		AICOND_07(BYTE newPercent) {
			this->type = __AIP_CONDITION_CODE__ | 0x08;
			this->_size = 0x08 + 0x01;
			
			this->percent = newPercent;
		}
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf,"Check Percentage (0x07)\n%s=====\n%sRandom Percentage: %i%\n", indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getPercentage());
			return std::string(buf);
		}
		__inline BYTE getPercentage() const { return this->percent; }
};

struct AICOND_08 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		DWORD distance;
		short levelDifference[2];
		bool isAllied;
	public:
		const static BYTE LEVELDIFF_START = 0x00;
		const static BYTE LEVELDIFF_END = 0x01;
		AICOND_08(DWORD newDistance, short levelDiff_min, short levelDiff_max, BYTE isAllied) {
			this->type = __AIP_CONDITION_CODE__ | 0x09;
			this->_size = 0x08 + 0x08 + sizeof(bool);
			
			this->distance = newDistance;
			this->levelDifference[AICOND_08::LEVELDIFF_START] = levelDiff_min;
			this->levelDifference[AICOND_08::LEVELDIFF_END] = levelDiff_max;

			this->isAllied = isAllied > 0 ? true : false;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check until X fitting entities (0x08)\n%s=====\n%sAllowed Distance: %f\n%sLevelDiff: [%i;%i]\n%sNeeds Ally: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), indent ? "\t\t\t" : "", this->getLevelDifference(0x00),
				indent ? "\t\t\t" : "", this->getLevelDifference(0x01), indent ? "\t\t\t" : "", this->isAllyNeeded());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline short getLevelDifference(BYTE startOrEnd) const { return this->levelDifference[startOrEnd % 2]; }
		__inline short isAllyNeeded() const { return this->isAllied; }
};

struct AICOND_09 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
	public:
		AICOND_09() {
			this->type = __AIP_CONDITION_CODE__ | 0x0A;
			this->_size = 0x08;
		}
		__inline std::string toString() const { return std::string("HAS_TARGET_CHANGED (0x09)"); };
};

struct AICOND_10 {
	private:
		__BASIC_AI_HEADER__;
		friend class AIService;
		BYTE abilityType;
		bool moreOrLess;
	public:
		AICOND_10(BYTE _abilityType, BYTE _moreOrLess) {
			this->type = __AIP_CONDITION_CODE__ | 0x0B;
			this->_size = 0x08 + 0x01 + sizeof(bool);
			
			this->abilityType = _abilityType;
			this->moreOrLess = (_moreOrLess > 0 ? true : false);
		}
		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf,"Check AbilityDifference (CurTarget <-> DestTarget) (0x0A)\n%s=====\n%sAbilityType: %s\n%sNeeds Less: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", AIService::getAbilityTypeName(this->getAbilityType()),
				indent ? "\t\t\t" : "", this->needsLessAbility());
			return std::string(buf);
		}
		__inline BYTE getAbilityType() const { return this->abilityType; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(Ability);
#else //__MORE_OR_LESS_DEF__

#endif //__MORE_OR_LESS_DEF__
};

struct AICOND_11 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE abilityType;
			DWORD notUsed;
		};
		long value;
		bool moreOrLess;
	public:
		AICOND_11(BYTE _abilityType, long _value, BYTE _moreOrLess) {
			this->type = __AIP_CONDITION_CODE__ | 0x0C;
			this->_size = 0x08 + 0x05 + sizeof(bool);

			this->abilityType = _abilityType;
			this->value = _value;
			this->moreOrLess = (_moreOrLess > 0 ? true : false);
		}
		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf,"Check Ability Value (0x0B)\n%s=====\n%sAbilityType: %s\n%sValue: %i\n%sNeeds Less: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", AIService::getAbilityTypeName(this->getAbilityType()),
				indent ? "\t\t\t" : "", this->getAbilityValue(),
				indent ? "\t\t\t" : "", this->needsLessValue());
			return std::string(buf);
		}
		__inline BYTE getAbilityType() const { return this->abilityType; }
		__inline long getAbilityValue() const { return this->value; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(Value);
#else

#endif
};

struct AICOND_12 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE when;
	public:
		AICOND_12(BYTE _when) {
			this->type = __AIP_CONDITION_CODE__ | 0x0D;
			this->_size = 0x08 + 0x01;

			this->when = _when;
		}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf,"Check daytime (0=day, 1=night) (0x0C)\n%s=====\n%sWhen %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getWhen());
			return std::string(buf);
		}
		__inline BYTE getWhen() const { return this->when; }
};

struct AICOND_13 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE checkTarget;
		BYTE statusType;
		bool hasIt;
	public:
		AICOND_13(BYTE check, BYTE status, BYTE hasItOrNot) {
			this->type = __AIP_CONDITION_CODE__ | 0x0E;
			this->_size = 0x08 + 0x02 + sizeof(bool);

			this->checkTarget = check;
			this->statusType = status;
			this->hasIt = (hasItOrNot > 0 ? true : false);
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Buffs (0x0D)\n%s=====\n%sTargetType: %i\n%sBuffType: %i\n%sHasIt: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getTargetCheckType(),
				indent ? "\t\t\t" : "", this->getStatusType(),
				indent ? "\t\t\t" : "", this->needsStatusType());
			return std::string(buf);
		}
		__inline BYTE getTargetCheckType() const { return this->checkTarget; }
		__inline BYTE getStatusType() const { return this->statusType; }
		__inline bool needsStatusType() const { return this->hasIt; }
};

struct AICOND_14 {
	protected:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE variableIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
		AICOND_14() {}
	public:
		AICOND_14(BYTE varIdx, int newValue, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x0F;
			this->_size = 0x08 + 0x07;
			
			this->variableIdx = varIdx;
			this->value = newValue;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Variables (0x0E)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getVariableIndex() const { return this->variableIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AICOND_15 {	
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE variableIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AICOND_15(BYTE varIdx, int newValue, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x10;
			this->_size = 0x08 + 0x07;
			
			this->variableIdx = varIdx;
			this->value = newValue;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Variables (0x0F)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getVariableIndex() const { return this->variableIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }

};

struct AICOND_16 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE variableIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AICOND_16(BYTE varIdx, int newValue, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x11;
			this->_size = 0x08 + 0x07;
			
			this->variableIdx = varIdx;
			this->value = newValue;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Variables (0x10)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getVariableIndex() const { return this->variableIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }

};

struct AICOND_17 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD npcId;
	public:
		AICOND_17(DWORD npcType) {
			this->type = __AIP_CONDITION_CODE__ | 0x12;
			this->_size = 0x08 + 0x06;

			this->npcId = npcType;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check NPC on Map (0x11)\n%s=====\n%sNpcID: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getNpcId());
			return std::string(buf);
		}
		__inline DWORD getNpcId() const { return this->npcId; }
};

struct AICOND_18 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		BYTE operation;
	public:
		AICOND_18(DWORD dist, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x13;
			this->_size = 0x08 + 0x05;

			this->distance = dist;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Distance to Owner (0x12)\n%s=====\n%sDistance: %f\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AICOND_19 {
	protected:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD startTime;
		DWORD endTime;
		AICOND_19() {}
	public:
		AICOND_19(DWORD _startTime, DWORD _endTime) {
			this->type = __AIP_CONDITION_CODE__ | 0x14;
			this->_size = 0x08 + 0x08;

			this->startTime = _startTime;
			this->endTime = _endTime;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Zone Time (0x13)\n%s=====\n%sStartTime: %i, EndTime: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getStartTime(), this->getEndTime());
			return std::string(buf);
		}
		__inline DWORD getStartTime() const { return this->startTime; }
		__inline DWORD getEndTime() const { return this->endTime; }
};

struct AICOND_20 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE abilityType;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AICOND_20(BYTE ability, int val, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x15;
			this->_size = 0x08 + 0x06;

			this->abilityType = ability;
			this->value = val;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check AbilityValue (0x14)\n%s=====\n%sAbility Name: %s\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", AIService::getAbilityTypeName(this->getAbilityType()),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getAbilityType() const { return this->abilityType; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AICOND_21 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AICOND_21() { this->type = __AIP_CONDITION_CODE__ | 0x16; 
			this->_size = 0x08;
		}
		__inline std::string toString() const { return std::string("HAS_NO_OWNER (0x15)\n"); }
};

struct AICOND_22 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AICOND_22() { this->type = __AIP_CONDITION_CODE__ | 0x17; 
			this->_size = 0x08;
		}
		__inline std::string toString() const { return std::string("HAS_OWNER (0x16)\n"); }
};

struct AICOND_23 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD startTime;
		DWORD endTime;
	public:
		AICOND_23(DWORD startTime, DWORD endTime) {
			this->type = __AIP_CONDITION_CODE__ | 0x18;
			this->_size = 0x08 + 0x08;

			this->startTime = startTime;
			this->endTime = endTime;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Time (0x17)\n%s=====\n%sStartTime: %i\nEndTime: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getStartTime(), this->getEndTime());
			return std::string(buf);
		}
		__inline DWORD getStartTime() const { return this->startTime; }
		__inline DWORD getEndTime() const { return this->endTime; }
};

struct AICOND_24 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE day; //1-31
		BYTE start[2];
		BYTE end[2];
		char notUsed[3];
	public:
		const static BYTE HOUR_TIME = 0x00;
		const static BYTE MINUTE_TIME = 0x01;

		AICOND_24(BYTE dayNo, BYTE hourStart, BYTE minuteStart, BYTE hourEnd, BYTE minuteEnd) {
			this->type = __AIP_CONDITION_CODE__ | 0x19;
			this->_size = 0x08 + 0x05;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Day of Month (0x18)\n%s=====%s\nDay %i\n%sStartTime[%i:%i]\n%sEndTime[%i:%i]\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDay(), indent ? "\t\t\t" : "", this->getHourStart(), this->getMinuteStart(),
				indent ? "\t\t\t" : "", this->getHourEnd(), this->getMinuteEnd());
			return std::string(buf);
		}
		__inline BYTE getDay() const { return this->day; }
		__inline BYTE getHourStart() const { return this->start[AICOND_24::HOUR_TIME]; }
		__inline BYTE getHourEnd() const { return this->end[AICOND_24::HOUR_TIME]; }
		__inline BYTE getMinuteStart() const { return this->start[AICOND_24::MINUTE_TIME]; }
		__inline BYTE getMinuteEnd() const { return this->end[AICOND_24::MINUTE_TIME]; }
};

struct AICOND_25 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE weekDay; //0-6
		BYTE start[2];
		BYTE end[2];
		char notUsed[3];
	public:
		const static BYTE HOUR_TIME = 0x00;
		const static BYTE MINUTE_TIME = 0x01;

		AICOND_25(BYTE dayNo, BYTE hourStart, BYTE minuteStart, BYTE hourEnd, BYTE minuteEnd) {
			this->type = __AIP_CONDITION_CODE__ | 0x1A;
			this->_size = 0x08 + 0x05;
			
			this->weekDay = dayNo;
			this->start[AICOND_25::HOUR_TIME] = hourStart;
			this->end[AICOND_25::HOUR_TIME] = hourEnd;

			this->start[AICOND_25::MINUTE_TIME] = minuteStart;
			this->end[AICOND_25::MINUTE_TIME] = minuteEnd;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check Weekday (0x19)\n%s=====%s\nWeekday %i\n%sStartTime[%i:%i]\n%sEndTime[%i:%i]\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getWeekday(), indent ? "\t\t\t" : "", this->getHourStart(), this->getMinuteStart(),
				indent ? "\t\t\t" : "", this->getHourEnd(), this->getMinuteEnd());
			return std::string(buf);
		}
		__inline BYTE getWeekday() const { return this->weekDay; }
		__inline BYTE getHourStart() const { return this->start[AICOND_25::HOUR_TIME]; }
		__inline BYTE getHourEnd() const { return this->end[AICOND_25::HOUR_TIME]; }
		__inline BYTE getMinuteStart() const { return this->start[AICOND_25::MINUTE_TIME]; }
		__inline BYTE getMinuteEnd() const { return this->end[AICOND_25::MINUTE_TIME]; }
};


struct AICOND_26 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD x;
		WORD y;
	public:
		AICOND_26(WORD _x, WORD _y) {
			this->type = __AIP_CONDITION_CODE__ | 0x1B;
			this->_size = 0x08 + 0x04;
			this->x = _x;
			this->y = _y;
		}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf,"Check Coordinates (0x1A)\n%s=====%s\nCoordinate: [%i;%i]\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getX(), this->getY());
			return std::string(buf);
		}
		__inline WORD getX() const { return this->x; }
		__inline WORD getY() const { return this->y; }
};

struct AICOND_27 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		union {
			bool isAllied;
			WORD wIsAllied;
		};
		short levelDiff[2];
		WORD amount;
		BYTE operation;
	public:	
		const static BYTE LEVEL_DIFF_START = 0x00;
		const static BYTE LEVEL_DIFF_END = 0x00;
		AICOND_27(DWORD dist, BYTE allyFlag, short levelDiffStart, short levelDiffEnd, WORD count, BYTE op) {
			this->type = __AIP_CONDITION_CODE__ | 0x1C;
			this->_size = 0x08 + 0x0C;

			this->distance = dist;
			this->isAllied = allyFlag > 0 ? true : false;
			this->levelDiff[LEVEL_DIFF_START] = levelDiffStart;
			this->levelDiff[LEVEL_DIFF_END] = levelDiffEnd;
			this->amount = count;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf,"Check level of nearby Entities (0x1B)\n%s=====\n%sDistance: %f\n%sNeeds Ally: %i\n%sLevelDiff: [%i;%i]\n%sAmount: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->needsAlly(),
				indent ? "\t\t\t" : "", this->getLevelDiff(0x00), this->getLevelDiff(0x01),
				indent ? "\t\t\t" : "", this->getAmount(), 
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline bool needsAlly() const { return this->isAllied; }
		__inline short getLevelDiff(BYTE pos) const { return this->levelDiff[pos % 2]; }
		__inline WORD getAmount() const { return this->amount; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AICOND_28 {
	private:
		__BASIC_AI_HEADER__;
		union {
			BYTE varIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
		friend class AIService;
	public:
		AICOND_28(BYTE idx, int val, BYTE op) { 
			this->type = __AIP_CONDITION_CODE__ | 0x1D;
			this->_size = 0x08 + 0x07;

			this->varIdx = idx;
			this->value = val;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf,"Check AI-Variables (0x1C)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVarIndex(),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getVarIndex() const { return this->varIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AICOND_29 {
	private:
		__BASIC_AI_HEADER__;
		BYTE targetType;
		friend class AIService;
	public:
		AICOND_29(BYTE target_Type) { 
			this->type = __AIP_CONDITION_CODE__ | 0x1E;
			this->_size = 0x08 + 0x01;

			this->targetType = target_Type;
		}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf,"Check TargetType (0x1D)\n%s=====\n%sTargetType: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getTargetType());
			return std::string(buf);
		}
		__inline BYTE getTargetType() const { return this->targetType; }
};

struct AICOND_30 {
	private:
		__BASIC_AI_HEADER__;
		DWORD unknown;
		friend class AIService;
	public:
		AICOND_30(DWORD unknownSoFar) { 
			this->type = __AIP_CONDITION_CODE__ | 0x1F;
			this->_size = 0x08 + 0x04;

			this->unknown = unknownSoFar;
		}
		
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf,"Check ??? (0x1E)\n%s=====\n%sUnknown: 0x%x\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariable());
			return std::string(buf);
		}
		__inline DWORD getVariable() const { return this->unknown; }
};

struct AIConditions {
	union {
		const BYTE *ptr;
		const BasicAIP* header;
		const struct AICOND_00* cond00;
		const struct AICOND_01* cond01;
		const struct AICOND_02* cond02;
		const struct AICOND_03* cond03;
		const struct AICOND_04* cond04;
		const struct AICOND_05* cond05;
		const struct AICOND_06* cond06;
		const struct AICOND_07* cond07;
		const struct AICOND_08* cond08;
		const struct AICOND_09* cond09;
		const struct AICOND_10* cond10;
		const struct AICOND_11* cond11;
		const struct AICOND_12* cond12;
		const struct AICOND_13* cond13;
		const struct AICOND_14* cond14;
		const struct AICOND_15* cond15;
		const struct AICOND_16* cond16;
		const struct AICOND_17* cond17;
		const struct AICOND_18* cond18;
		const struct AICOND_19* cond19;
		const struct AICOND_20* cond20;
		const struct AICOND_21* cond21;
		const struct AICOND_22* cond22;
		const struct AICOND_23* cond23;
		const struct AICOND_24* cond24;
		const struct AICOND_25* cond25;
		const struct AICOND_26* cond26;
		const struct AICOND_27* cond27;
		const struct AICOND_28* cond28;
		const struct AICOND_29* cond29;
		const struct AICOND_30* cond30;
	};
	AIConditions(const char* data) {
		(*this) = data;
	}
	AIConditions& operator=(const char* data) {
		this->ptr = reinterpret_cast<const BYTE*>(data);
		return (*this);
	}
	std::string toString() const {
		switch(this->header->getType()) {
			case BasicAIP::__AIP_CONDITION_00__:
				return this->cond00->toString();
			case BasicAIP::__AIP_CONDITION_01__:
				return this->cond01->toString();
			case BasicAIP::__AIP_CONDITION_02__:
				return this->cond02->toString();
			case BasicAIP::__AIP_CONDITION_03__:
				return this->cond03->toString();
			case BasicAIP::__AIP_CONDITION_04__:
				return this->cond04->toString();
			case BasicAIP::__AIP_CONDITION_05__:
				return this->cond05->toString();
			case BasicAIP::__AIP_CONDITION_06__:
				return this->cond06->toString();
			case BasicAIP::__AIP_CONDITION_07__:
				return this->cond07->toString();
			case BasicAIP::__AIP_CONDITION_08__:
				return this->cond08->toString();
			case BasicAIP::__AIP_CONDITION_09__:
				return this->cond09->toString();
			case BasicAIP::__AIP_CONDITION_10__:
				return this->cond10->toString();
			case BasicAIP::__AIP_CONDITION_11__:
				return this->cond11->toString();
			case BasicAIP::__AIP_CONDITION_12__:
				return this->cond12->toString();
			case BasicAIP::__AIP_CONDITION_13__:
				return this->cond13->toString();
			case BasicAIP::__AIP_CONDITION_14__:
				return this->cond14->toString();
			case BasicAIP::__AIP_CONDITION_15__:
				return this->cond15->toString();
			case BasicAIP::__AIP_CONDITION_16__:
				return this->cond16->toString();
			case BasicAIP::__AIP_CONDITION_17__:
				return this->cond17->toString();
			case BasicAIP::__AIP_CONDITION_18__:
				return this->cond18->toString();
			case BasicAIP::__AIP_CONDITION_19__:
				return this->cond19->toString();
			case BasicAIP::__AIP_CONDITION_20__:
				return this->cond20->toString();
			case BasicAIP::__AIP_CONDITION_21__:
				return this->cond21->toString();
			case BasicAIP::__AIP_CONDITION_22__:
				return this->cond22->toString();
			case BasicAIP::__AIP_CONDITION_23__:
				return this->cond23->toString();
			case BasicAIP::__AIP_CONDITION_24__:
				return this->cond24->toString();
			case BasicAIP::__AIP_CONDITION_25__:
				return this->cond25->toString();
			case BasicAIP::__AIP_CONDITION_26__:
				return this->cond26->toString();
			case BasicAIP::__AIP_CONDITION_27__:
				return this->cond27->toString();
			case BasicAIP::__AIP_CONDITION_28__:
				return this->cond28->toString();
			case BasicAIP::__AIP_CONDITION_29__:
				return this->cond29->toString();
			case BasicAIP::__AIP_CONDITION_30__:
				return this->cond30->toString();
		}
		return std::string("");
	}
};

struct AIACTION_00 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_00() {
			this->type = __AIP_ACTION_CODE__ | 0x01;
			this->_size = 0x08;
		}

		std::string toString() const {
			return std::string("WAT_TO_DO (0x00)\n");
		}
};

struct AIACTION_01 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE action;
	public:
		AIACTION_01(BYTE actionId) {
			this->type = __AIP_ACTION_CODE__ | 0x02;
			this->_size = 0x08 + 0x01;

			this->action = actionId;
		}
		std::string toString(bool indent = true) const { 
			char buf[0x30] = {0x00};
			sprintf(buf, "Set Action (0x01)\n%s=====\n%sActionType: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getAction());
			return std::string(buf);
		}
		__inline BYTE getAction() const { return this->action; }
};
struct AIACTION_02 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD stringId;
	public:
		AIACTION_02(DWORD strId) {
			this->type = __AIP_ACTION_CODE__ | 0x03;
			this->_size = 0x08 + 0x04;

			this->stringId = strId;
		}
		std::string toString(bool indent = true) const { 
			char buf[0x40] = {0x00};
			sprintf(buf, "Say String (0x02)\n%s=====%s\nLTB-StringId: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getStringID());
			return std::string(buf);
		}
		__inline DWORD getStringID() const { return this->stringId; }
};

struct AIACTION_03 {
	protected:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		BYTE stance;
		AIACTION_03() {}
	public:
		AIACTION_03(DWORD dist, BYTE _stance) {
			this->type = __AIP_ACTION_CODE__ | 0x04;
			this->_size = 0x08 + 0x05;

			this->distance = dist;
			this->stance = _stance;
		}
		std::string toString(bool indent = true) const { 
			char buf[0x60] = {0x00};
			sprintf(buf, "Move to random position (0x03)\n%s=====\n%sMaxDistance: %f\n%sStance: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->getStance());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline BYTE getStance() const { return this->stance; }
};

struct AIACTION_04 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		BYTE stance;
	public:
		AIACTION_04(DWORD dist, BYTE _stance) {
			this->type = __AIP_ACTION_CODE__ | 0x05;
			this->_size = 0x08 + 0x05;

			this->distance = dist;
			this->stance = _stance;
		}
		std::string toString(bool indent = true) const { 
			char buf[0x50] = {0x00};
			sprintf(buf, "Move to Spawn (0x04)\n%s=====\n%sMaxDistance: %f\n%sStance: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->getStance());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline BYTE getStance() const { return this->stance; }
};

struct AIACTION_05 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		BYTE stance;
	public:
		AIACTION_05(BYTE newStance) {
			this->type = __AIP_ACTION_CODE__ | 0x06;
			this->_size = 0x08 + 0x01;

			this->stance = newStance;
		}
		__inline BYTE getStance() const { return this->stance; }

		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			::sprintf(buf, "MoveTo Target (0x05)\n%s=====\n%sStance: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getStance());
			return (std::string(buf));
		}
};

struct AIACTION_06 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		BYTE abilityType;
		bool moreOrLess;
	public:
		AIACTION_06(DWORD dist, BYTE ability, BYTE isMoreOrLess) {
			this->type = __AIP_ACTION_CODE__ | 0x07;
			this->_size = 0x08 + 0x05 + sizeof(bool);

			this->distance = dist;
			this->abilityType = ability;
			this->moreOrLess = (isMoreOrLess > 0 ? true : false);
		}
		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf, "Find and attack target (0x06)\n%s=====\n%sMaxDist: %f\n%sAbilityType: %s\n%sNeedsLess: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getAbilityType()),
				indent ? "\t\t\t" : "", this->needsLessAbility() );
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline BYTE getAbilityType() const { return this->abilityType; }
#ifdef __MORE_OR_LESS_DEF__
		__MORE_OR_LESS_DEF__(Ability);
#else

#endif
};

struct AIACTION_07 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_07() { this->type = __AIP_ACTION_CODE__ | 0x08; 
			this->_size = 0x08;
		}
		std::string toString() const {
			return std::string("SPECIAL_ATTACK (0x07)\n");
		}
};

struct AIACTION_08 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		BYTE stance;
	public:
		AIACTION_08(DWORD dist, BYTE _stance) {
			this->type = __AIP_ACTION_CODE__ | 0x09;
			this->_size = 0x08 + 0x05;

			this->distance = dist;
			this->stance = _stance;
		}
		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf, "MoveTo Target (0x08)\n%s=====\n%sMaxDist: %f\n%sStance: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->getStance());
			return std::string(buf);
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline BYTE getStance() const { return this->stance; }
};

struct AIACTION_09 {
	protected:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monsterId;
		AIACTION_09() { }
	public:
		AIACTION_09(WORD monId) {
			this->type = __AIP_ACTION_CODE__ | 0x0A;
			this->_size = 0x08 + 0x02;

			this->monsterId = monId;
		}
		__inline WORD getMonsterId() const { return this->monsterId; }
		
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf, "Convert Monster (0x09)\n%s=====\n%sNewMonID: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId());
			return std::string(buf);
		}
};

struct AIACTION_10 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monsterId;
	public:
		AIACTION_10(WORD monId) { 
			this->type = __AIP_ACTION_CODE__ | 0x0B; 
			this->_size = 0x08 + 0x02;
			
			this->monsterId = monId;
		}
		__inline WORD getMonsterId() const { return this->monsterId; }
		
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf, "Spawn Pet (0x0A)\n%s=====\n%sPetTypeId: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId());
			return std::string(buf);
		}
};

struct AIACTION_11 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		DWORD numOfMonsters;
	public:
		AIACTION_11(DWORD dist, DWORD numberOfMonsters) { 
			this->type = __AIP_ACTION_CODE__ | 0x0C;
			this->_size = 0x08 + 0x08;

			this->distance = dist;
			this->numOfMonsters = numberOfMonsters;
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		__inline DWORD getMonsterAmount() const { return this->numOfMonsters; }
		
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf, "Call Allies for Attack (0x0B)\n%s=====\n%sDistance: %f\n%sAmount: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->getMonsterAmount());
			return std::string(buf);
		}
};

struct AIACTION_12 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_12() { this->type = __AIP_ACTION_CODE__ | 0x0D; 
			this->_size = 0x08;
		}
		std::string toString() const {
			return std::string("ATTACK_NEAREST_TARGET (0x0C)\n");
		}
};

struct AIACTION_13 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_13() { this->type = __AIP_ACTION_CODE__ | 0x0E; 
			this->_size = 0x08;
		}
		std::string toString() const {
			return std::string("ATTACK_FOUND_TARGET (0x0D)\n");
		}
};

struct AIACTION_14 {
	protected:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
		AIACTION_14() {}
	public:
		AIACTION_14(DWORD dist) {
			this->type = __AIP_ACTION_CODE__ | 0x0F;
			this->_size = 0x08 + 0x04;
			this->distance = dist;
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf, "Call family for attack (0x0E)\n%s=====\n%sDistance: %f\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance());
			return std::string(buf);
		}
};

struct AIACTION_15 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_15() { this->type = __AIP_ACTION_CODE__ | 0x10; 
			this->_size = 0x08;}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf, "Attack Designated target (0x0F)\n%s====%sTrue\n",
				indent ? "\t\t\t" : "",indent ? "\t\t\t" : "" );
			return std::string(buf);
		}
};

struct AIACTION_16 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		DWORD distance;
	public:
		AIACTION_16(DWORD dist) {
			this->type = __AIP_ACTION_CODE__ + 0x11;
			this->_size = 0x08 + 0x04;
			this->distance = dist;
		}
		__inline float getDistance() const { return this->distance * 100.0f; }
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf, "Run Away (0x10)\n%s=====\n%sDistance: %f\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getDistance());
			return std::string(buf);
		}
};

struct AIACTION_17 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			struct {
				WORD itemNo0;
				WORD itemNo1;
				WORD itemNo2;
				WORD itemNo3;
				WORD itemNo4;
			};
			WORD items[5];
		};
		DWORD itemOwner;
	public:
		AIACTION_17(WORD item0, WORD item1, WORD item2, WORD item3, WORD item4) {
			this->type = __AIP_ACTION_CODE__ | 0x12;
			this->_size = 0x08 + 0x0A;

			this->itemNo0 = item0;
			this->itemNo1 = item1;
			this->itemNo2 = item2;
			this->itemNo3 = item3;
			this->itemNo4 = item4;

			this->itemOwner = 0x00;
		}
		AIACTION_17(WORD items[5]) {
			this->type = __AIP_ACTION_CODE__ | 0x12;
			this->_size = 0x08 + 0x0A;
			for(unsigned int i=0;i<5;i++) {
				this->items[i] = items[i];
			}
			this->itemOwner = 0x00;
		}
		std::string toString(bool indent = true) const {
			char buf[0x80] = {0x00};
			sprintf(buf, "Drop Item (0x11)\n%s=====\n", indent ? "\t\t\t" : "" );
			for(unsigned int i=0;i<5;i++) {
				if(this->items[i] > 0)
					sprintf(&buf[strlen(buf)], "%sItem[%i]: %i\n", indent ? "\t\t\t" : "",  i, this->items[i]);
			}
			return std::string(buf);
		}
		__inline WORD getItem(const BYTE itemOutOfFive) {
			return this->items[itemOutOfFive % 5];
		}
		__inline DWORD getItemOwner() const { return this->itemOwner; }
};

struct AIACTION_18 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monId;
		WORD monAmount;
		DWORD distance;
	public:
		AIACTION_18(WORD monsterId, WORD monsterAmount, DWORD dist) {
			this->type = __AIP_ACTION_CODE__ | 0x13;
			this->_size = 0x08 + 0x08;

			this->monId = monsterId;
			this->monAmount = monAmount;
			this->distance = dist;
		}
		__inline WORD getMonsterId() const { return this->monId; }
		__inline WORD getMonsterAmount() const { return this->monAmount; }
		__inline float getDistance() const { return this->distance * 100.0f; }
		
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Spawn Slaves (0x12)\n%s=====\n%sMonTypeId: %i\n%sAmount: %i\n%sMaxDistance: %f\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId(),
				indent ? "\t\t\t" : "", this->getMonsterAmount(),
				indent ? "\t\t\t" : "", this->getDistance());
			return std::string(buf);
		}
};

struct AIACTION_19 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_19() { this->type = __AIP_ACTION_CODE__ | 0x14; 
			this->_size = 0x08;
		}
		std::string toString(bool indent = true) const {
			char buf[0x40] = {0x00};
			sprintf(buf, "Attack nearest target (0x13)\n%s=====%sTrue\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "");
			return std::string(buf);
		}
};

struct AIACTION_20 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monId;
		union {
			BYTE positionType;
			WORD notUsed;
		};
		DWORD distance;
	public:
		const static BYTE CURRENT_POSITION = 0x00;
		const static BYTE DESIGNATED_TARGET_POSITION = 0x01;
		const static BYTE TARGET_POSITION = 0x02;
		AIACTION_20(WORD monsterId, BYTE posType, DWORD dist) {
			this->type = __AIP_ACTION_CODE__ | 0x15;
			this->_size = 0x08 + 0x08;

			this->monId = monsterId;
			this->positionType = posType;
			this->distance = dist;
		}
		__inline WORD getMonsterId() const { return this->monId; }
		__inline WORD getPositionType() const { return this->positionType; }
		__inline float getDistance() const { return this->distance * 100.0f; }
		std::string toString(bool indent = true) const {
			char buf[0xA0] = {0x00};
			sprintf(buf, "Spawn Slave at Position (0x14)\n%s=====\n%sMonTypeId: %i\n%sPositionType: %i\n%sMaxDistance: %f\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId(), 
				indent ? "\t\t\t" : "", this->getPositionType(),
				indent ? "\t\t\t" : "", this->getDistance());
			return std::string(buf);
		}
};

struct AIACTION_21 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_21() { this->type = __AIP_ACTION_CODE__ | 0x16; 
			this->_size = 0x08;
		}
		std::string toString(bool indent = true) const {
			return std::string("??? (0x15)\n");
		}
};
struct AIACTION_22 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_22() { this->type = __AIP_ACTION_CODE__ | 0x17; 
			this->_size = 0x08;
		}
		std::string toString() const {
			return std::string("??? (0x16)\n");
		}
};
struct AIACTION_23 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_23() { this->type = __AIP_ACTION_CODE__ | 0x18; 
			this->_size = 0x08;
		}
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf, "Suicide (0x17)\n%s=====%sTrue\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "");
			return std::string(buf);
		}
};

struct AIACTION_24 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE targetType;
			WORD notUsed;
		};
		WORD skillId;
		union {
			WORD motion;
			DWORD notUsed2;
		};
	public:
		const static BYTE FOUND_TARGET = 0x00;
		const static BYTE PREVIOUS_TARGET = 0x01;
		const static BYTE SELF_TARGET = 0x02;
		AIACTION_24(BYTE targetType, WORD skillID, WORD motionId) {
			this->type = __AIP_ACTION_CODE__ | 0x19;
			this->_size = 0x08 + 0x05;

			this->targetType = targetType;
			this->skillId = skillID;
			this->motion = motionId;
		}
		__inline BYTE getTargetType() const { return this->targetType; }
		__inline WORD getSkillId() const { return this->skillId; }
		__inline WORD getMotion() const { return this->motion; }
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Cast Skill (0x18)\n%s=====\n%sTargetType: %i\n%sSkillId: %i\n%sMotion: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getTargetType(), 
				indent ? "\t\t\t" : "", this->getSkillId(),
				indent ? "\t\t\t" : "", this->getMotion());
			return std::string(buf);
		}
};
struct AIACTION_25 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE varIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AIACTION_25(BYTE varIndex, int val, BYTE op) { 
			this->type = __AIP_ACTION_CODE__ | 0x1A; 
			this->_size = 0x08 + 0x07;
			
			this->varIdx = varIndex;
			this->value = val;
			this->operation = op;
		}
		__inline BYTE getVariableIndex() const { return this->varIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Set NPC Variable (0x19)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(), 
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
};
struct AIACTION_26 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE varIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AIACTION_26(BYTE varIndex, int val, BYTE op) { 
			this->type = __AIP_ACTION_CODE__ | 0x1B; 
			this->_size = 0x08 + 0x07;
			
			this->varIdx = varIndex;
			this->value = val;
			this->operation = op;
		}
		__inline BYTE getVariableIndex() const { return this->varIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Set World Variable (0x1A)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(),
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
};
struct AIACTION_27 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE varIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AIACTION_27(BYTE varIndex, int val, BYTE op) { 
			this->type = __AIP_ACTION_CODE__ | 0x1B; 
			this->_size = 0x08 + 0x07;
			
			this->varIdx = varIndex;
			this->value = val;
			this->operation = op;
		}
		__inline BYTE getVariableIndex() const { return this->varIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Set Economy Variable (0x1B)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(), 
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
};

struct AIACTION_28 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE messageType; //0 = normal, 1 = shout, 2 = announce (?)
			DWORD notUsed;
		};
		DWORD messageId;
	public:
		const static BYTE LOCAL_CHAT = 0x00;
		const static BYTE SHOUT_CHAT = 0x01;
		const static BYTE ANNOUNCE_CHAT = 0x02;
		AIACTION_28(BYTE msgType, DWORD msgId) { 
			this->type = __AIP_ACTION_CODE__ | 0x1D;
			this->_size = 0x08 + 0x05;

			this->messageType = msgType;
			this->messageId = msgId;
		}
		__inline BYTE getMessageType() const { return this->messageType; }
		__inline DWORD getMessageId() const { return this->messageId; }
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Say Message (0x1C)\n%s=====\n%sMessageType: %i\n%sMessageId: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMessageType(), 
				indent ? "\t\t\t" : "", this->getMessageId());
			return std::string(buf);
		}
};

struct AIACTION_29 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_29() { this->type = __AIP_ACTION_CODE__ | 0x1E; 
			this->_size = 0x08;
		}
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Move towards owner (0x1D)\n%s====%sTrue\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "");
			return std::string(buf);
		}
};

struct AIACTION_30 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE length;
			WORD len;
		};
		char triggerName[128];
	public:
		AIACTION_30(const char* trigger) {
			this->type = __AIP_ACTION_CODE__ | 0x1F;
			this->_size = 0x08 + 0x04 + strlen(trigger);
			
			//this->triggerName = trigger;
		}
		std::string getTriggerName() const { 
			return std::string(this->triggerName);
		}
		std::string toString(bool indent = true) const {
			std::string triggerName = this->getTriggerName();
			char buf[0x100] = {0x00};
			sprintf(buf, "QuestTrigger (0x1E)\n%s====\n%sTriggerName: %s\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", triggerName.c_str());
			return std::string(buf);
		}
};

struct AIACTION_31 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_31() { this->type = __AIP_ACTION_CODE__ | 0x20; 
			this->_size = 0x08;
		}
		std::string toString(bool indent = true) const {
			char buf[0x50] = {0x00};
			sprintf(buf, "Attack owner's target\n%s====%sTrue\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "");
			return std::string(buf);
		}
};

struct AIACTION_32 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD mapId;
		bool offOrOn;
	public:
		AIACTION_32(WORD map, BYTE setOffOrOn) {
			this->type = __AIP_ACTION_CODE__ | 0x21;
			this->_size = 0x08 + 0x02 + sizeof(bool);

			this->mapId = map;
			this->offOrOn = (setOffOrOn > 0 ? true : false);
		}
		__inline WORD getMapId() const { return this->mapId; }
		__inline bool setOn() const { return this->offOrOn; }
		__inline bool setOff() const { return !this->setOn(); }

		std::string toString(bool indent = true) const {
			char buf[0x100] = {0x00};
			sprintf(buf, "Set PVPMode on/off\n%s====\n%sMapId: %i\n%sSetOn: %i\n", 
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMapId(), 
				indent ? "\t\t\t" : "", this->setOn());
			return std::string(buf);
		}
};

//Zone Action (?)
struct AIACTION_33 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
	public:
		AIACTION_33() {
			this->type = __AIP_ACTION_CODE__ | 0x22;
			this->_size = 0x08;
		}
		std::string toString() const { return std::string("ZONE_TRIGGER(??)\n"); }
};

struct AIACTION_34 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD itemNum;
		WORD amount;
	public:
		AIACTION_34(WORD itemNo, WORD count) {
			this->type = __AIP_ACTION_CODE__ | 0x23;
			this->_size = 0x08 + 0x04;

			this->itemNum = itemNo;
			this->amount = count;
		}
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Give item to owner\n%s=====\n%sItem: %i\n%sAmount: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getItemNum(), 
				indent ? "\t\t\t" : "", this->getAmount());
			return std::string(buf);
		}
		__inline WORD getItemNum() const { return this->itemNum; }
		__inline WORD getAmount() const { return this->amount; }
};

struct AIACTION_35 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		union {
			BYTE varIdx;
			DWORD notUsed;
		};
		int value;
		BYTE operation;
	public:
		AIACTION_35(BYTE idx, int val, BYTE op) {
			this->type = __AIP_ACTION_CODE__ | 0x24;
			this->_size = 0x08 + 0x07;

			this->varIdx = idx;
			this->value = val;
			this->operation = op;
		}
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Set AI Variable (?)\n%s=====\n%sVarIdx: %i\n%sValue: %i\n%sOperation: %s\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getVariableIndex(), 
				indent ? "\t\t\t" : "", this->getValue(),
				indent ? "\t\t\t" : "", AIService::operationName(this->getOperationType()));
			return std::string(buf);
		}
		__inline BYTE getVariableIndex() const { return this->varIdx; }
		__inline int getValue() const { return this->value; }
		__inline BYTE getOperationType() const { return this->operation; }
};

struct AIACTION_36 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monId;
		BYTE assignMaster;
	public:
		AIACTION_36(WORD monId, BYTE assign) {
			this->type = __AIP_ACTION_CODE__ | 0x25;
			this->_size = 0x08 + 0x03;

			this->monId = monId;
			this->assignMaster = assign;
		}
		std::string toString(bool indent = true) const {
			char buf[0x60] = {0x00};
			sprintf(buf, "Spawn Monster\n%s=====\n%sMonId: %i\n%sAssignMaster: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId(), 
				indent ? "\t\t\t" : "", this->getAssignMaster());
			return std::string(buf);
		}
		__inline WORD getMonsterId() const { return this->monId; }
		__inline BYTE getAssignMaster() const { return this->assignMaster; }
};

struct AIACTION_37 {
	private:
		friend class AIService;
		__BASIC_AI_HEADER__;
		WORD monId;
		WORD positionType;
		DWORD distance;
		BYTE assignMaster;
	public:
		AIACTION_37(WORD monId, WORD posType, DWORD dist, BYTE assign) {
			this->type = __AIP_ACTION_CODE__ | 0x26;
			this->_size = 0x08 + 0x03;

			this->monId = monId;
			this->positionType = posType;
			this->distance = dist;
			this->assignMaster = assign;
		}
		std::string toString(bool indent = true) const {
			char buf[0xA0] = {0x00};
			sprintf(buf, "Spawn Monster at position\n%s=====\n%sMonId: %i\n%sPositionType: %i\n%sDistance: %f\n%sAssignMaster: %i\n",
				indent ? "\t\t\t" : "", indent ? "\t\t\t" : "", this->getMonsterId(), 
				indent ? "\t\t\t" : "", this->getPositionType(), 
				indent ? "\t\t\t" : "", this->getDistance(), 
				indent ? "\t\t\t" : "", this->getAssignMaster());
			return std::string(buf);
		}
		__inline WORD getMonsterId() const { return this->monId; }
		__inline BYTE getAssignMaster() const { return this->assignMaster; }
		__inline WORD getPositionType() const { return this->positionType; }
		__inline float getDistance() const { return this->distance * 100.0f; }
};

struct AIActions {
	union {
		const BYTE *ptr;
		const BasicAIP* header;
		const AIACTION_00* act00;
		const AIACTION_01* act01;
		const AIACTION_02* act02;
		const AIACTION_03* act03;
		const AIACTION_04* act04;
		const AIACTION_05* act05;
		const AIACTION_06* act06;
		const AIACTION_07* act07;
		const AIACTION_08* act08;
		const AIACTION_09* act09;
		const AIACTION_10* act10;
		const AIACTION_11* act11;
		const AIACTION_12* act12;
		const AIACTION_13* act13;
		const AIACTION_14* act14;
		const AIACTION_15* act15;
		const AIACTION_16* act16;
		const AIACTION_17* act17;
		const AIACTION_18* act18;
		const AIACTION_19* act19;
		const AIACTION_20* act20;
		const AIACTION_21* act21;
		const AIACTION_22* act22;
		const AIACTION_23* act23;
		const AIACTION_24* act24;
		const AIACTION_25* act25;
		const AIACTION_26* act26;
		const AIACTION_27* act27;
		const AIACTION_28* act28;
		const AIACTION_29* act29;
		const AIACTION_30* act30;
		const AIACTION_31* act31;
		const AIACTION_32* act32;
		const AIACTION_33* act33;
		const AIACTION_34* act34;
		const AIACTION_35* act35;
		const AIACTION_36* act36;
		const AIACTION_37* act37;
	};
	AIActions(const char* data) {
		(*this) = data;
	}
	AIActions& operator=(const char* data) {
		this->ptr = reinterpret_cast<const BYTE*>(data);
		return (*this);
	}
	
	std::string toString() const {
		switch(this->header->getType()) {
			case BasicAIP::__AI_ACTION_00__:
				return this->act00->toString();
			case BasicAIP::__AI_ACTION_01__:
				return this->act01->toString();
			case BasicAIP::__AI_ACTION_02__:
				return this->act02->toString();
			case BasicAIP::__AI_ACTION_03__:
				return this->act03->toString();
			case BasicAIP::__AI_ACTION_04__:
				return this->act04->toString();
			case BasicAIP::__AI_ACTION_05__:
				return this->act05->toString();
			case BasicAIP::__AI_ACTION_06__:
				return this->act06->toString();
			case BasicAIP::__AI_ACTION_07__:
				return this->act07->toString();
			case BasicAIP::__AI_ACTION_08__:
				return this->act08->toString();
			case BasicAIP::__AI_ACTION_09__:
				return this->act09->toString();
			case BasicAIP::__AI_ACTION_10__:
				return this->act10->toString();
			case BasicAIP::__AI_ACTION_11__:
				return this->act11->toString();
			case BasicAIP::__AI_ACTION_12__:
				return this->act12->toString();
			case BasicAIP::__AI_ACTION_13__:
				return this->act13->toString();
			case BasicAIP::__AI_ACTION_14__:
				return this->act14->toString();
			case BasicAIP::__AI_ACTION_15__:
				return this->act15->toString();
			case BasicAIP::__AI_ACTION_16__:
				return this->act16->toString();
			case BasicAIP::__AI_ACTION_17__:
				return this->act17->toString();
			case BasicAIP::__AI_ACTION_18__:
				return this->act18->toString();
			case BasicAIP::__AI_ACTION_19__:
				return this->act19->toString();
			case BasicAIP::__AI_ACTION_20__:
				return this->act20->toString();
			case BasicAIP::__AI_ACTION_21__:
				return this->act21->toString();
			case BasicAIP::__AI_ACTION_22__:
				return this->act22->toString();
			case BasicAIP::__AI_ACTION_23__:
				return this->act23->toString();
			case BasicAIP::__AI_ACTION_24__:
				return this->act24->toString();
			case BasicAIP::__AI_ACTION_25__:
				return this->act25->toString();
			case BasicAIP::__AI_ACTION_26__:
				return this->act26->toString();
			case BasicAIP::__AI_ACTION_27__:
				return this->act27->toString();
			case BasicAIP::__AI_ACTION_28__:
				return this->act28->toString();
			case BasicAIP::__AI_ACTION_29__:
				return this->act29->toString();
			case BasicAIP::__AI_ACTION_30__:
				return this->act30->toString();
			case BasicAIP::__AI_ACTION_31__:
				return this->act31->toString();
			case BasicAIP::__AI_ACTION_32__:
				return this->act32->toString();
			case BasicAIP::__AI_ACTION_33__:
				return this->act33->toString();
			case BasicAIP::__AI_ACTION_34__:
				return this->act34->toString();
			case BasicAIP::__AI_ACTION_35__:
				return this->act35->toString();
			case BasicAIP::__AI_ACTION_36__:
				return this->act36->toString();
			case BasicAIP::__AI_ACTION_37__:
				return this->act37->toString();
		}
		return std::string("");
	}
};

class AIP {
	public:
		const static BYTE MAX_BLOCKS = 0x06;
		struct Record {
			private:
				friend class AIP;
				std::vector<Trackable<char>> conditions;
				std::vector<Trackable<char>> actions;
			public:

				Record() { 
					conditions.clear();
					actions.clear();
				}
				__inline const std::vector<Trackable<char>>& getConditions() const {
					return this->conditions;
				}
				__inline const Trackable<char>& getCondition(const size_t pos) const {
					return this->conditions.at(pos);
				}
				__inline const std::vector<Trackable<char>>& getActions() const {
					return this->actions;
				}
				__inline const Trackable<char>& getAction(const size_t pos) const {
					return this->actions.at(pos);
				}
				__inline const DWORD getConditionCount() const { return this->conditions.size(); }
				__inline const DWORD getActionCount() const { return this->actions.size(); }
		};
	private:
		struct Block {
			std::vector<AIP::Record> records;
		} blocks[MAX_BLOCKS];
		WORD id;
		std::string filePath;
		DWORD checkInterval;
		DWORD damageAmountTrigger; //Or Damage dealt/received?
		template<class FileType> void loadFrom(FileType& file);
	public:
		const static BYTE ON_SPAWN = 0x00;
		const static BYTE ON_IDLE = 0x01;
		const static BYTE ON_ATTACK = 0x02;
		const static BYTE ON_DAMAGED = 0x03;
		const static BYTE ON_ENEMY_DEATH = 0x04;
		const static BYTE ON_SELF_DEATH = 0x05;

		AIP() {
			this->checkInterval = 0x00;
			this->damageAmountTrigger = 0x00;
			this->filePath = std::string("");
			this->id = 0x00;
		}
#ifdef __ROSE_USE_VFS__
		AIP(const WORD id, VFSData& data);
#else
		AIP(const WORD id, const char* fileName);
#endif
		~AIP();

		__inline WORD getId() const { return this->id; }
		__inline BYTE getBlockCount() const { return AIP::MAX_BLOCKS; }
		__inline DWORD getRecordCount(BYTE blockId) const { return this->blocks[blockId % this->getBlockCount()].records.size(); }
		__inline const std::vector<AIP::Record>& getRecords(const BYTE blockId) const {
			return this->blocks[blockId % this->getBlockCount()].records;
		}
		__inline std::string getFilePath() const { return std::string(this->filePath); }
		__inline DWORD getCheckInterval() const { return this->checkInterval; }
		__inline DWORD getTriggerDamageAmount() const { return this->damageAmountTrigger; }
};

#endif //__ROSE_AIP__