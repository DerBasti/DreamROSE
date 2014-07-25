#include "AIP.h"
#include "..\WorldServer.h"
#include "..\Entity\Player.h"
#include "..\Entity\Drop.h"
#include "..\Entity\Monster.h"

#include "D:\Programmieren\CMyFile\MyFile.h"

AIP::AIP(const char* fileName) {
	CMyFile file(fileName, "rb");
	if(file.exists())
		this->loadFrom(file);
	file.close();
}

AIP::~AIP() {

}

#ifndef __READ_STRING__
#define __READ_STRING__(len, buf) fread(&len, 1, 1, fh); fread(buf, 1, len, fh); buf[len] = 0x00;
#endif
/*
	FORMAT:
		DWORD blockCount;
		DWORD idleTimeInSeconds
		DWORD attackMoveTimeInSeconds
		
		DWORD strLen
		strLen AIname

		foreach(block : blockCount)
			BYTE blockName[0x20]
			DWORD recordsInBlock
			foreach(record : recordsInBlock) 
				DWORD conditionCount
				if conditionCount > 0
					DWORD blockLen
					DWORD operationCode
					BYTE data[(blockLen - (sizeof(DWORD)*2)]
				DWORD actionCount
				if actionCount > 0
					DWORD blockLen
					DWORD operationCode
					BYTE data[(blockLen - (sizeof(DWORD)*2)]
			end_foreach(record)
		end_foreach(block)

*/
void AIP::loadFrom(CMyFile &fh) {
	DWORD triggerCount = fh.read<DWORD>();

	if(triggerCount != AIP::MAX_BLOCKS)
		return;

	BYTE len = 0x00; char buf[0x200] = {0x00};
	this->checkInterval = fh.read<DWORD>();
	this->damageAmountTrigger = fh.read<DWORD>();
	fh.readStringT<DWORD, char>(buf);

	for(unsigned int i=0;i<AIP::MAX_BLOCKS;i++) {
		DWORD dTmp = fh.skip(0x20); //0x20 FIXED STRING LENGTH
		
		DWORD recordCount = fh.read<DWORD>();
		this->blocks[i].records.reserve(recordCount);
		for(unsigned int j=0;j<recordCount;j++) {
			fh.skip(0x20); //FIXED STRING
			this->blocks[i].records.push_back(AIP::Record());
			DWORD condCount = fh.read<DWORD>();
			if(condCount>0) {
				std::vector<Trackable<char>>& conditions = this->blocks[i].records[j].conditions;
				conditions.reserve(condCount);
				for(unsigned int k=0;k<condCount;k++) {
					fh.readStringT(0x08, buf);
					DWORD dLen = *((DWORD*)buf);
					fh.readStringT(dLen - 0x08, &buf[0x08]);
					conditions.push_back(Trackable<char>(buf, dLen));
				}
			}
			DWORD actionCount = fh.read<DWORD>();
			if(actionCount > 0) {
				std::vector<Trackable<char>>& actions = this->blocks[i].records[j].actions;
				actions.reserve(actionCount);
				for(unsigned int k=0;k<actionCount;k++) {
					fh.readStringT(0x08, buf);
					DWORD aLen = *((DWORD*)buf);
					fh.readStringT(aLen - 0x08, &buf[0x08]);
					actions.push_back(Trackable<char>(buf, aLen));
				}
			}
		}
	}
}

const char* AIService::getAbilityTypeName(BYTE abilityType) {
	switch(abilityType) {
		case AIService::ABILITY_LEVEL:
			return "Level";
		case AIService::ABILITY_ATTACKPOWER:
			return "Attackpower";
		case AIService::ABILITY_DEFENSE:
			return "Defense";
		case AIService::ABILITY_MAGIC_DEFENSE:
			return "Magic Defense";
		case AIService::ABILITY_CHARM:
			return "Charm";
	}
	return "UNKNOWN!";
}

WORD AIService::getAbilityType( BYTE abilityType, Entity* entity ) {
	switch(abilityType) {
		case AIService::ABILITY_LEVEL:
			return entity->getLevel();
		case AIService::ABILITY_ATTACKPOWER:
			return entity->getAttackPower();
		case AIService::ABILITY_DEFENSE:
			return entity->getDefensePhysical();
		case AIService::ABILITY_MAGIC_DEFENSE:
			return entity->getDefenseMagical();
		case AIService::ABILITY_CHARM:
			return entity->getCharm();
	}
	return 0;
}

const char* AIService::operationName(BYTE operation) {
	switch(operation) {
		case AIService::OPERATION_EQUAL:
			return "==";
		case AIService::OPERATION_BIGGER:
			return ">";
		case AIService::OPERATION_BIGGER_EQUAL:
			return ">=";
		case AIService::OPERATION_SMALLER:
			return  "<";
		case AIService::OPERATION_SMALLER_EQUAL:
			return "<=";
		case AIService::OPERATION_NOT_EQUAL:
			return "!=";
		case AIService::OPERATION_ADDITION:
			return "+";
		case AIService::OPERATION_SUBTRACTION:
			return "-";
		case AIService::OPERATION_MULTIPLICATION:
			return "*";
		case AIService::OPERATION_DIVISION:
			return "/";
		case AIService::OPERATION_RETURN_RHS:
			return "this = rhs";
	}
	return "UNKNOWN";
}

template<class _Ty1, class _Ty2> bool AIService::checkOperation(_Ty1& first, _Ty2& second, BYTE operation) {
	switch(operation) {
		case AIService::OPERATION_EQUAL:
			return (first == second);
		case AIService::OPERATION_BIGGER:
			return (first > second);
		case AIService::OPERATION_BIGGER_EQUAL:
			return (first >= second);
		case AIService::OPERATION_SMALLER:
			return (first < second);
		case AIService::OPERATION_SMALLER_EQUAL:
			return (first <= second);
		case AIService::OPERATION_NOT_EQUAL:
			return (first != second);
	}
	return false;
}

template<class _Ty> static _Ty AIService::resultOperation(_Ty& first, _Ty& second, BYTE operation) {
	switch(operation) {
		case AIService::OPERATION_ADDITION:
			return _Ty(first + second);
		case AIService::OPERATION_SUBTRACTION:
			return _Ty(first - second);
		case AIService::OPERATION_MULTIPLICATION:
			return _Ty(first * second);
		case AIService::OPERATION_DIVISION:
			return _Ty(first / second);
		case AIService::OPERATION_RETURN_RHS:
			return _Ty(second);
	}
	return _Ty(0);
}

bool AIService::run(NPC* npc, const BYTE blockId, Entity* target, const DWORD dmgDealt) {
	WORD timeDiff = static_cast<WORD>(time(nullptr) - npc->getTimeAICheck());
	if(npc->getAI() == nullptr) {
		return false;
	}
	//Check overall conditions
	switch(blockId) {
		case AIP::ON_IDLE:
			if( timeDiff <= npc->getAI()->getCheckInterval() )
				return false;
		break;
		case AIP::ON_DAMAGED:
			if(npc->getTarget() != nullptr) {
				if(QuickInfo::random<DWORD>(100) <= npc->getAI()->getTriggerDamageAmount())
					return false;
			}
		break;
	}
	npc->setTimeAICheck();
	const std::vector<AIP::Record>& block = npc->getAI()->getRecords(blockId);
	for(unsigned int i=0;i<block.size();i++) {
		const AIP::Record& curRecord = block.at(i);

		AITransfer trans; trans.designatedTarget = target;
		const std::vector<Trackable<char>>& conditions = curRecord.getConditions();
		if(AIService::checkConditions(conditions, npc, &trans)) {
			const std::vector<Trackable<char>>& actions = curRecord.getActions();
			AIService::executeActions(actions, npc, &trans);
			return true;
		}
	}
	return false;
}

#pragma region Condition Functions
bool AIService::checkConditions( const std::vector<Trackable<char>>& ai, NPC *npc, AITransfer* trans) {
	bool result = true;
	for(unsigned int i=0;i<ai.size();i++) {
		const char* data = ai.at(i).getData();
		const BasicAIP* curAI = reinterpret_cast<const BasicAIP*>(data);
		if(!curAI) 
			continue;
		switch(curAI->getType()) {
			case BasicAIP::__AIP_CONDITION_00__:
				result &= AIService::conditionFightOrDelay();
			break;
			case BasicAIP::__AIP_CONDITION_01__:
				result &= AIService::conditionEnoughDamageReceived(npc, reinterpret_cast<const AICOND_01*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_02__:
				result &= AIService::conditionHasEnoughTargets(npc, reinterpret_cast<const AICOND_02*>(curAI), trans);
			break;
			case BasicAIP::__AIP_CONDITION_03__:
				result &= AIService::conditionDistanceFromSpawn(npc, reinterpret_cast<const AICOND_03*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_04__:
				result &= AIService::conditionDistanceToTarget(npc, reinterpret_cast<const AICOND_04*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_05__:
				result &= AIService::conditionCheckAbilityDifference(npc, reinterpret_cast<const AICOND_05*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_06__:
				result &= AIService::conditionCheckPercentHP(npc, reinterpret_cast<const AICOND_06*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_07__:
				result &= conditionRandomPercentageMet(reinterpret_cast<const AICOND_07*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_08__:
				result &= conditionFindNearestEligibleTarget(npc, reinterpret_cast<const AICOND_08*>(curAI), trans);
			break;
			case BasicAIP::__AIP_CONDITION_09__:
				result &= conditionHasTargetChanged(npc, reinterpret_cast<const AICOND_09*>(curAI), trans);
			break;
			case BasicAIP::__AIP_CONDITION_10__:
				result &= conditionCompareAbilities(npc, reinterpret_cast<const AICOND_10*>(curAI), trans);
			break;
			case BasicAIP::__AIP_CONDITION_11__:
				result &= conditionIsStatSufficient(npc, reinterpret_cast<const AICOND_11*>(curAI), trans);
			break;
			case BasicAIP::__AIP_CONDITION_12__:
				result &= conditionHasDaytimeArrived(npc, reinterpret_cast<const AICOND_12*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_13__:
				result &= conditionHasBuff(npc, reinterpret_cast<const AICOND_13*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_14__:
				result &= conditionIsObjectVarValid(npc, reinterpret_cast<const AICOND_14*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_15__:
				result &= conditionIsWorldVarValid(reinterpret_cast<const AICOND_15*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_16__:
				result &= conditionIsEconomyVarValid(reinterpret_cast<const AICOND_16*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_17__:
				result &= conditionIsNPCNearby(npc, reinterpret_cast<const AICOND_17*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_18__:
				result &= conditionCheckDistanceToOwner(npc, reinterpret_cast<const AICOND_18*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_19__:
				result &= conditionCheckZoneTime(npc, reinterpret_cast<const AICOND_19*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_20__:
				result &= conditionAreOwnStatsSufficient(npc, reinterpret_cast<const AICOND_20*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_21__:
				result &= conditionHasNoOwner(npc, reinterpret_cast<const AICOND_21*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_22__:
				result &= conditionHasOwner(npc, reinterpret_cast<const AICOND_22*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_23__:
				result &= conditionWorldTime(reinterpret_cast<const AICOND_23*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_24__:
				result &= conditionWeekTime(reinterpret_cast<const AICOND_24*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_25__:
				result &= conditionMonthTime(reinterpret_cast<const AICOND_25*>(curAI));
			break;
			case BasicAIP::__AIP_CONDITION_26__:
				result &= conditionUnknown(reinterpret_cast<const AICOND_26*>(curAI));
			break;
			default:
				result &= true;
		}
		if(!result)
			return false;
	}
	return result;
}
		
bool AIService::conditionFightOrDelay() {
	return false; //??
}

bool AIService::conditionEnoughDamageReceived(NPC* npc, const AICOND_01* condition) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon)
		return false;

	//Deal damage --> nothing happens
	if(condition->isActionOnDeal())
		return false;

	DWORD dmgReceived = mon->getMaxHP() - mon->getCurrentHP();
	if(dmgReceived >= condition->getDamage())
		return true;
	return false;
}

bool AIService::conditionHasEnoughTargets(NPC *npc, const AICOND_02* cond, AITransfer* trans) {
	bool alliedStatus = cond->isAlliedEntity;
	DWORD targetCount = 0x00;
	float nearestDist = 9999999.0f;

	Entity* target = nullptr;
	for(unsigned int i=0;i<npc->getVisibleSectors().size(); i++) {
		MapSector* sector = npc->getVisibleSectors().getValue(i);
		LinkedList<Entity*>::Node* eNode = sector->getFirstEntity();
		for(;eNode;eNode = eNode->getNextNode()) {
			Entity* curEntity = eNode->getValue();
			if(!curEntity || !curEntity->isIngame() || curEntity->getEntityType() == Entity::TYPE_DROP) {
				continue;
			}short levelDiff = curEntity->getLevel() - npc->getLevel();
			if(cond->levelDiff[AICOND_02::LEVELDIFF_START] >= levelDiff &&
				cond->levelDiff[AICOND_02::LEVELDIFF_END] <= levelDiff &&
				dynamic_cast<Entity*>(npc)->isAllied( curEntity ) == alliedStatus) {
				
				targetCount++;
			
				float dist = curEntity->getPositionCurrent().distanceTo(npc->getPositionCurrent());
				if(dist <= nearestDist) {
					trans->nearestEntity = curEntity;
				}
				if(targetCount >= cond->entityAmount) {
					trans->lastFound = curEntity;
					return true;
				}
			}
		}
	}
	return false;
}

bool AIService::conditionDistanceFromSpawn( NPC* npc, const AICOND_03* cond ) {
	if(!npc->getSpawn())
		return false;
	if(npc->getSpawnPosition().distanceTo(npc->getPositionCurrent()) >= cond->getDistance())
		return true;
	return false;
}

bool AIService::conditionDistanceToTarget( NPC* npc, const AICOND_04* cond ) {
	if(!npc->getTarget())
		return false;

	float distance = npc->getPositionCurrent().distanceTo(npc->getTarget()->getPositionCurrent());
	float condDist = cond->getDistance();
	if (AIService::checkOperation(distance, condDist, cond->moreOrLess ? AIService::OPERATION_SMALLER_EQUAL : AIService::OPERATION_BIGGER_EQUAL))
		return true;
	return false;
}

bool AIService::conditionCheckAbilityDifference(NPC* npc, const AICOND_05* cond) {
	if(!npc->getTarget())
		return false;

	WORD abilityValue = AIService::getAbilityType(cond->abilityType, npc->getTarget());
	if(AIService::checkOperation(abilityValue, cond->difference, cond->moreOrLess ? AIService::OPERATION_SMALLER_EQUAL : AIService::OPERATION_BIGGER_EQUAL))
		return true;
	return false;
}

bool AIService::conditionCheckPercentHP(NPC* npc, const AICOND_06* cond) {
	BYTE percentHP = npc->getPercentHP();
	return AIService::checkOperation(percentHP, cond->hp, cond->moreOrLess ? AIService::OPERATION_SMALLER_EQUAL : AIService::OPERATION_BIGGER_EQUAL);
}

bool AIService::conditionRandomPercentageMet(const AICOND_07* cond) {
	BYTE randomPercentage = rand() % 100;
	if(randomPercentage <= cond->percent)
		return true;
	return false;
}

bool AIService::conditionFindNearestEligibleTarget(NPC* npc, const AICOND_08* cond, AITransfer* trans) {
	UniqueSortedList<DWORD, MapSector*> sectors = npc->getVisibleSectors();
	for(unsigned int i=0;i<sectors.size();i++) {
		MapSector* currentSector = sectors.getValue(i);
		LinkedList<Entity*>::Node* eNode = currentSector->getFirstEntity();
		for(;eNode;eNode = eNode->getNextNode()) {
			Entity* entity = eNode->getValue();
			if(!entity || !entity->isIngame() || entity->getEntityType() == Entity::TYPE_DROP) {
				continue;
			}
			short levelDiff = npc->getLevel() - entity->getLevel();
			if(dynamic_cast<Entity*>(npc)->isAllied(entity) == cond->isAllied && 
				cond->levelDifference[ AICOND_08::LEVELDIFF_START ] >= levelDiff &&
				cond->levelDifference[ AICOND_08::LEVELDIFF_END ] <= levelDiff &&
				npc->getPositionCurrent().distanceTo(entity->getPositionCurrent()) <= cond->getDistance()) {
					trans->lastFound = trans->nearestEntity = entity;
					return true;
			}
		}
	}
	return false;
}

bool AIService::conditionHasTargetChanged(NPC *npc, const AICOND_09* cond, AITransfer* trans) {
	if(npc->getTarget() != trans->designatedTarget)
		return true;
	return false;
}

bool AIService::conditionCompareAbilities(NPC *npc, const AICOND_10* cond, AITransfer* trans) {
	if(npc->getTarget() == nullptr || trans->designatedTarget == nullptr)
		return false;

	WORD npcTargetValue = AIService::getAbilityType(cond->abilityType, npc->getTarget());

	WORD transTargetValue = AIService::getAbilityType(cond->abilityType, trans->designatedTarget);
	return AIService::checkOperation(npcTargetValue, transTargetValue, cond->moreOrLess ? AIService::OPERATION_BIGGER : AIService::OPERATION_SMALLER);
}

bool AIService::conditionIsStatSufficient(NPC* npc, const AICOND_11* cond, AITransfer* trans) {
	if(npc->getTarget() != nullptr || trans->designatedTarget == nullptr)
		return false;

	WORD targetValue = AIService::getAbilityType(cond->abilityType, trans->designatedTarget);
	return AIService::checkOperation(targetValue, cond->value, cond->moreOrLess ? AIService::OPERATION_SMALLER_EQUAL : AIService::OPERATION_BIGGER_EQUAL);
}

bool AIService::conditionHasDaytimeArrived(NPC* npc, const AICOND_12* cond) {
	if(mainServer->getMapTime(npc->getMapId()) == cond->when)
		return true;
	return false;
}

bool AIService::conditionHasBuff(NPC* npc, const AICOND_13* cond) {
	Entity* target = npc;
	if(cond->checkTarget) {
		target = npc->getTarget();
		if(target == nullptr)
			return false;
	}
	
	DWORD buffs = 0x00;
	switch( cond->statusType ) {
		case 0:
			buffs = target->getBuffStatus( Buffs::POSITIVE_BUFFS );
		break;
		case 1:
			buffs = target->getBuffStatus( Buffs::NEGATIVE_BUFFS );
		break;
		default:
			buffs = target->getBuffStatus( 0x00 );
	}
	return cond->hasIt ? buffs > 0 : buffs == 0;
}

bool AIService::conditionIsObjectVarValid(NPC* npc, const AICOND_14* cond) {
	int value = npc->getObjVar(cond->variableIdx);
	return AIService::checkOperation(value, cond->value, cond->operation);
}

bool AIService::conditionIsWorldVarValid(const AICOND_15* cond) {
	int value = mainServer->getWorldVariable(cond->variableIdx);
	return AIService::checkOperation(value, cond->value, cond->operation);
}

bool AIService::conditionIsEconomyVarValid(const AICOND_16* cond) {
	int value = mainServer->getEconomyVariable(cond->variableIdx);
	return AIService::checkOperation(value, cond->value, cond->operation);
}

bool AIService::conditionIsNPCNearby(NPC *npc, const AICOND_17* ai) {
	Map* currentMap = mainServer->getMap(npc->getMapId());
	LinkedList<Entity*>::Node* eNode = currentMap->getFirstEntity();
	for(;eNode;eNode = eNode->getNextNode()) {
		Entity* curEntity = eNode->getValue();
		if(!curEntity || curEntity->getEntityType() == Entity::TYPE_PLAYER) {
			continue;
		}
		NPC* npc = dynamic_cast<NPC*>(curEntity);
		if (npc && npc->getTypeId() == ai->getNpcId())
			return true;
	}
	return false;
}

bool AIService::conditionCheckDistanceToOwner(NPC* npc, const AICOND_18* cond) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon || mon->getOwner() == nullptr)
		return false;

	DWORD dist = static_cast<DWORD>(mon->getPositionCurrent().distanceTo(mon->getPositionCurrent()));
	float condDist = cond->getDistance();
	return AIService::checkOperation(dist, condDist, cond->operation);
}

bool AIService::conditionCheckZoneTime(NPC* npc, const AICOND_19* cond) {
	DWORD localTime = mainServer->getMapTime(npc->getMapId());
	if(localTime >= cond->startTime && localTime <= cond->endTime)
		return true;
	return false;
}

bool AIService::conditionAreOwnStatsSufficient(NPC* npc, const AICOND_20* cond) {
	WORD value = AIService::getAbilityType(cond->abilityType, npc);
	return AIService::checkOperation(value, cond->value, cond->operation);
}

bool AIService::conditionHasNoOwner(NPC* npc, const AICOND_21* ai) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon || mon->getOwner() != nullptr)
		return false;
	return true;
}

bool AIService::conditionHasOwner(NPC* npc, const AICOND_22* ai) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon || mon->getOwner() == nullptr)
		return false;
	return true;
}

 bool AIService::conditionWorldTime(const AICOND_23* ai) {
	 if(mainServer->getWorldTime() >= ai->getStartTime() && mainServer->getWorldTime() <= ai->getEndTime())
		 return true;
	 return false;
 }
 
bool AIService::conditionWeekTime(const AICOND_24* cond) {
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);

	if(cond->day != 0) {
		if(sTime.wDay != cond->day)
			return false;
	}
	unsigned __int16 Minutes = ((sTime.wHour * 60) + sTime.wMinute);
	unsigned __int16 wFrom = (cond->hour[AICOND_24::START_TIME] * 60) + cond->minute[AICOND_24::START_TIME];
	unsigned __int16 wTo = (cond->hour[AICOND_24::END_TIME] * 60) + cond->minute[AICOND_24::END_TIME];
	if(Minutes >= wFrom && Minutes <= wTo)
		return true;
	return false;
}

bool AIService::conditionMonthTime(const AICOND_25* cond) {
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	if(cond->weekDay != sTime.wDayOfWeek) {
		return false;
	}
	unsigned __int16 Minutes = ((sTime.wHour * 60) + sTime.wMinute);
	unsigned __int16 wFrom = (cond->hour[AICOND_24::START_TIME] * 60) + cond->minute[AICOND_24::START_TIME];
	unsigned __int16 wTo = (cond->hour[AICOND_24::END_TIME] * 60) + cond->minute[AICOND_24::END_TIME];
	if(Minutes >= wFrom && Minutes <= wTo)
		return true;
	return false;
}

bool AIService::conditionUnknown(const AICOND_26* ai) { 
	return true; //??x
}
#pragma endregion

#pragma region Action Functions
void AIService::executeActions( const std::vector<Trackable<char>>& ai, NPC* npc, AITransfer* trans ) {
	for(unsigned int i=0;i<ai.size();i++) {
		Trackable<char> data = ai.at(i);
		const BasicAIP* aip = reinterpret_cast<const BasicAIP*>(data.getData());
		switch(aip->getType()) {
			case BasicAIP::__AI_ACTION_00__:
				AIService::actionStop(npc);
			break;
			case BasicAIP::__AI_ACTION_01__:
				AIService::actionSetEmote(npc, reinterpret_cast<const AIACTION_01*>(aip));
			break;
			case BasicAIP::__AI_ACTION_02__:
				AIService::actionSayMessage(npc, reinterpret_cast<const AIACTION_02*>(aip));
			break;
			case BasicAIP::__AI_ACTION_03__:
				AIService::actionSetNewRandomPos(npc, reinterpret_cast<const AIACTION_03*>(aip));
			break;
			case BasicAIP::__AI_ACTION_04__:
				AIService::actionSetNewPosFromSpawn(npc, reinterpret_cast<const AIACTION_04*>(aip));
			break;
			case BasicAIP::__AI_ACTION_05__:
				AIService::actionSetPositionToFoundTarget(npc, reinterpret_cast<const AIACTION_05*>(aip), trans);
			break;
			case BasicAIP::__AI_ACTION_06__:
				AIService::actionAttackTarget(npc, reinterpret_cast<const AIACTION_06*>(aip));
			break;
			case BasicAIP::__AI_ACTION_07__:
				AIService::actionSpecialAttack(npc);
			break;
			case BasicAIP::__AI_ACTION_08__:
				AIService::actionMoveToTarget(npc, reinterpret_cast<const AIACTION_08*>(aip), trans);
			break;
			case BasicAIP::__AI_ACTION_09__:
				AIService::actionConvert(npc, reinterpret_cast<const AIACTION_09*>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, reinterpret_cast<const AIACTION_10*>(aip));
			break;
			case BasicAIP::__AI_ACTION_11__:
				AIService::actionCallAlliesForAttack(npc, reinterpret_cast<const AIACTION_11*>(aip));
			break;
			case BasicAIP::__AI_ACTION_12__:
			case BasicAIP::__AI_ACTION_19__:
				AIService::actionAttackNearestTarget(npc, trans);
			break;
			case BasicAIP::__AI_ACTION_13__:
				AIService::actionAttackFoundTarget(npc, trans);
			break;
			case BasicAIP::__AI_ACTION_14__:
				AIService::actionCallEntireFamilyForAttack(npc);
			break;
			case BasicAIP::__AI_ACTION_15__:
				AIService::actionAttackDesignatedTarget(npc, trans);
			break;
			case BasicAIP::__AI_ACTION_16__:
				AIService::actionRunAway(npc, reinterpret_cast<const AIACTION_16*>(aip));
			break;
			case BasicAIP::__AI_ACTION_17__:
				AIService::actionDropItem( reinterpret_cast<const AIACTION_17*>(aip));
			break;
			case BasicAIP::__AI_ACTION_18__:
				AIService::actionCallFewFamilyMembersForAttack(npc, reinterpret_cast<const AIACTION_18*>(aip));
			break;
			case BasicAIP::__AI_ACTION_20__:
				AIService::actionSpawnPetAtPosition(npc, reinterpret_cast<const AIACTION_20*>(aip), trans);
			break;
			case BasicAIP::__AI_ACTION_21__:
				AIService::actionKillNPC(npc);
			break;
			case BasicAIP::__AI_ACTION_24__:
				AIService::actionCastSkill(npc, reinterpret_cast<const AIACTION_24*>(aip), trans);
			break;
			case BasicAIP::__AI_ACTION_25__:
				AIService::actionChangeNPCVar(npc, reinterpret_cast<const AIACTION_25*>(aip));
			break;
			case BasicAIP::__AI_ACTION_26__:
				AIService::actionChangeWorldVar(npc, reinterpret_cast<const AIACTION_26*>(aip));
			break;
			case BasicAIP::__AI_ACTION_27__:
				AIService::actionChangeEconomyVar(npc, reinterpret_cast<const AIACTION_27*>(aip));
			break;
			case BasicAIP::__AI_ACTION_28__:
				AIService::actionSayMessage(npc, reinterpret_cast<const AIACTION_28*>(aip));
			break;
			case BasicAIP::__AI_ACTION_29__:
				AIService::actionMoveToOwner(npc, reinterpret_cast<const AIACTION_29*>(aip));
			break;
			case BasicAIP::__AI_ACTION_30__:
				AIService::actionSetQuestTrigger(npc, reinterpret_cast<const AIACTION_30*>(aip));
			break;
			case BasicAIP::__AI_ACTION_31__:
				AIService::actionAttackOwnersTarget(npc);
			break;
			case BasicAIP::__AI_ACTION_32__:
				AIService::actionSetMapAsPVPArea(npc, reinterpret_cast<const AIACTION_32*>(aip));
			break;
			case BasicAIP::__AI_ACTION_34__:
				AIService::actionGiveItemsToOwner(npc, reinterpret_cast<const AIACTION_34*>(aip));
			break;
				/*
			case BasicAIP::__AI_ACTION_15__:
				AIService::actionSpawnPet(npc, reinterpret_cast<const AIACTION_02&>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, reinterpret_cast<const AIACTION_02&>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, dynamic_cast<AIACTION_02&>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, dynamic_cast<AIACTION_02&>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, dynamic_cast<AIACTION_02&>(aip));
			break;
			case BasicAIP::__AI_ACTION_10__:
				AIService::actionSpawnPet(npc, dynamic_cast<AIACTION_02&>(aip));
			break;
			*/
		}
	}
}

void AIService::actionStop(NPC* npc) {
	npc->setPositionDest(npc->getPositionCurrent());
	return;
}

void AIService::actionSetEmote(NPC* npc, const AIACTION_01* act) {
	//npc->setEmote( act->action );
}

void AIService::actionSayMessage(NPC* npc, const AIACTION_02* act) {
	//WorldServer::ChatService::sendMessage(npc, LTB[act->msgId]);
}

void AIService::actionSetNewRandomPos(NPC* npc, const AIACTION_03* act) {
	Position newPos( npc->getPositionCurrent() );
	newPos.x += QuickInfo::fRand(act->getDistance(), true);
	newPos.y += QuickInfo::fRand(act->getDistance(), true);
	
	npc->setStance(act->getStance());
	npc->setPositionDest(newPos);
}

void AIService::actionSetNewPosFromSpawn(NPC* npc, const AIACTION_04* act) {
	Position newPos( npc->getSpawnPosition() );
	
	const float dist = act->getDistance();
	newPos.x += QuickInfo::fRand(dist, true);
	newPos.x += QuickInfo::fRand(dist, true);
	
	npc->setStance(act->stance);
	npc->setPositionDest(newPos);
}

void AIService::actionSetPositionToFoundTarget(NPC* npc, const AIACTION_05* act, AITransfer* trans) {
	if(trans->lastFound == nullptr)
		return;

	Position newPos( trans->lastFound->getPositionCurrent() );
	newPos.x += QuickInfo::fRand(200, true);
	newPos.y += QuickInfo::fRand(200, true);
	
	npc->setStance(act->stance);
	npc->setPositionDest( newPos );
}

void AIService::actionAttackTarget(NPC* npc, const AIACTION_06* act) {
	Entity* maxTarget = nullptr;
	Entity* minTarget = nullptr;

	UniqueSortedList<DWORD, MapSector*> sectors = npc->getVisibleSectors();
	WORD minValue = 0xFFFF; WORD maxValue = 0;
	for(unsigned int i=0;i<sectors.size();i++) {
		MapSector* sector = sectors.getValue(i);
		LinkedList<Entity*>::Node* eNode = sector->getFirstEntity();
		
		for(;eNode;eNode = eNode->getNextNode()) {
		Entity* curChar = eNode->getValue();
			if(!curChar || dynamic_cast<Entity*>(npc)->isAllied(curChar))
				continue;

			WORD value = AIService::getAbilityType(act->abilityType, curChar);
			if(value < minValue) {
				minTarget = curChar;
				minValue = value;
			} else if(value > maxValue) {
				maxTarget = curChar;
				maxValue = value;
			}
		}
	}
	if(act->moreOrLess) {
		if(minTarget)
			npc->setTarget(minTarget);
	} else {
		if(maxTarget)
			npc->setTarget(maxTarget);
	}
}

void AIService::actionSpecialAttack(NPC* npc) {
}

void AIService::actionMoveToTarget(NPC * npc, const AIACTION_08 *act, AITransfer* trans) {
	if(npc->getTarget() == nullptr)
		return;

	float fX = npc->getTarget()->getCurrentX() - npc->getCurrentX();
	float fY = npc->getTarget()->getCurrentY() - npc->getCurrentY();
	float dist = npc->getPositionCurrent().distanceTo(npc->getTarget()->getPositionCurrent());
	
	npc->setStance(act->stance);
	npc->setPositionDest(Position(npc->getCurrentX() - (act->getDistance() * fX / dist),
		npc->getCurrentY() - (act->getDistance() * fY / dist)));
}

void AIService::actionConvert(NPC* npc, const AIACTION_09* act) {
	mainServer->convertTo( npc, act->monsterId );	
}

void AIService::actionSpawnPet(NPC* npc, const AIACTION_10* act) {
	//TODO: SPAWN MONSTER
}

void AIService::actionCallAlliesForAttack(NPC* npc, const AIACTION_11 *act) {
	Entity* target = npc->getTarget();
	if(!target)
		return;

	UniqueSortedList<DWORD, MapSector*> sectors = npc->getVisibleSectors();
	Entity* ally = nullptr;

	DWORD numOfAttackers = 0x00;
	for(unsigned int i=0;i<sectors.size();i++) {
		MapSector* sector = sectors.getValue(i);
		LinkedList<Entity*>::Node* eNode = sector->getFirstEntity();
		while(eNode) {
			ally = eNode->getValue();
			eNode = eNode->getNextNode();
			if( ally->isAllied( npc ) && ally->getTarget() == nullptr || ally->getEntityType() == Entity::TYPE_MONSTER) {
				ally->setTarget( target );

				numOfAttackers++;
				if(numOfAttackers >= act->numOfMonsters)
					return;
			}
		}
	}
}


void AIService::actionAttackNearestTarget(NPC* npc, AITransfer* trans) {
	npc->setTarget(trans->nearestEntity);
}

void AIService::actionAttackFoundTarget(NPC* npc, AITransfer* trans) {
	npc->setTarget( trans->lastFound );
}

void AIService::actionCallEntireFamilyForAttack(NPC* npc) {
	Entity* target = npc->getTarget();
	if(!target)
		return;

	WORD npcId = npc->getTypeId();

	UniqueSortedList<DWORD, MapSector*> sectors = npc->getVisibleSectors();
	for(unsigned int i=0;i<sectors.size();i++) {
		MapSector* sector = sectors.getValue(i);
		LinkedList<Entity*>::Node* nNode = sector->getFirstNPC();
		for(;nNode;nNode = sector->getNextNPC(nNode)) {
			NPC *currentNPC = dynamic_cast<NPC*>(nNode->getValue());
			if(!currentNPC || currentNPC->getTypeId() != npcId)
				continue;
			if(npc->isAllied(currentNPC) && npc->getTarget() == nullptr) {
				npc->setTarget(target);
			}
		}
	}
}

void AIService::actionAttackDesignatedTarget(NPC* npc, AITransfer* trans) {
	npc->setTarget(trans->designatedTarget);
}

void AIService::actionRunAway(NPC* npc, const AIACTION_16* act) {
	Position newPos = npc->getPositionCurrent();

	float xMod = QuickInfo::fRand(act->getDistance(), true);
	float yMod = 1.0f - xMod;
	newPos.x = newPos.x + static_cast<float>(xMod * act->getDistance());
	newPos.y = newPos.y + static_cast<float>(yMod * act->getDistance());
	
	//clear target
	npc->setTarget( nullptr );
	npc->setStance(Stance::RUNNING);
	npc->setPositionDest(newPos);
}

void AIService::actionDropItem(const AIACTION_17* act) {
	short dropItem = act->items[ rand() % 5 ];
	
	Item item;
	item.type = dropItem / 1000;
	item.id = dropItem % 1000;

	new Drop(nullptr, item, true);
}

void AIService::actionCallFewFamilyMembersForAttack(NPC* npc, const AIACTION_18* act) {
	Entity *target = npc->getTarget();
	if(!target)
		return;

	UniqueSortedList<DWORD, MapSector*> sectors = npc->getVisibleSectors();
	DWORD callCount = 0x00;
	for(unsigned int i=0;i<sectors.size();i++) {
		MapSector* sector = sectors.getValue(i);
		LinkedList<Entity*>::Node* nNode = sector->getFirstNPC();
		for(;nNode;nNode = sector->getNextNPC(nNode)) {
			NPC* curNPC = dynamic_cast<NPC*>(nNode->getValue());
			if(!curNPC || curNPC->getTarget() != nullptr)
				continue;
			if(curNPC->getTypeId() == npc->getTypeId() &&
				npc->isAllied(curNPC) && 
				curNPC->getPositionCurrent().distanceTo(npc->getPositionCurrent()) <= act->getDistance()) {
					curNPC->setTarget(target);

					callCount++;
					if(callCount >= act->monAmount)
						return;
			}
		}
	}
}

void AIService::actionSpawnPetAtPosition(NPC* npc, const AIACTION_20* act, AITransfer *trans) {
	switch(act->positionType) {
		case AIACTION_20::CURRENT_POSITION:

		break;
		case AIACTION_20::DESIGNATED_TARGET_POSITION:

		break;
		case AIACTION_20::TARGET_POSITION:

		break;
	}
}

void AIService::actionKillNPC(NPC* npc) {
	npc->addDamage(npc->getCurrentHP());
}

void AIService::actionCastSkill(NPC* npc, const AIACTION_24* act, AITransfer* trans) {
	switch(act->targetType) {
		case AIACTION_24::FOUND_TARGET:

		break;
		case AIACTION_24::PREVIOUS_TARGET:

		break;
		case AIACTION_24::SELF_TARGET:

		break;
	}
}

void AIService::actionChangeNPCVar(NPC* npc, const AIACTION_25* act) {
	int val = npc->getObjVar(act->varIdx);
	int actVal = act->getValue();
	int newVal = AIService::resultOperation(val, actVal, act->operation);

	npc->setObjVar(act->varIdx, newVal);
}

void AIService::actionSayMessage(NPC* npc, const AIACTION_28* act) {
	switch(act->messageType) {
		case AIACTION_28::LOCAL_CHAT:

		break;
		case AIACTION_28::SHOUT_CHAT:

		break;
		case AIACTION_28::ANNOUNCE_CHAT:

		break;
	}
}

void AIService::actionMoveToOwner(NPC *npc, const AIACTION_29* act) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon || mon->getOwner() == nullptr)
		return;

	Entity* owner = mon->getOwner();
	float dist = mon->getPositionCurrent().distanceTo( owner->getPositionCurrent() );
	
	mon->setPositionDest( Position(owner->getCurrentX() - (0.2f * dist), owner->getCurrentY() - (0.2f * dist)) );
	mon->setStance(Stance::RUNNING);
}

void AIService::actionSetQuestTrigger(NPC *npc, const AIACTION_30* act) {
	//TODO: QUESTING
}

void AIService::actionAttackOwnersTarget(NPC* npc) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon) return;

	Entity *owner = mon->getOwner();
	if(owner && owner->getTarget() != nullptr && !owner->isAllied(owner->getTarget())) {
		mon->setTarget(owner->getTarget());
	}
}

void AIService::actionSetMapAsPVPArea(NPC* npc, const AIACTION_32* act) {
	//TODO: TRIGGER PVP AREAS
	if(act->mapId == 0) {
		//SET CURRENT MAP TO PVP
	} else {

	}
}
void AIService::actionGiveItemsToOwner(NPC* npc, const AIACTION_34* act) {
	Monster* mon = dynamic_cast<Monster*>(npc);
	if(!mon || !mon->getOwner())
		return;

	Player* player = reinterpret_cast<Player*>(mon->getOwner());
	if(!player)
		return;

	//TODO: player->addToInventory( act->getItemNum(), act->getAmount() );
}

void AIService::actionSetAIVar(NPC* npc, const AIACTION_35* act) {
	int val = npc->getAIVar(act->getVariableIndex());
	int newVal = act->getValue();
	int result = AIService::resultOperation(val, newVal, act->getOperationType());
	npc->setAIVar(act->getVariableIndex(), result);
}
#pragma endregion