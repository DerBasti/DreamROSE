#include "Structures.h"
#include "Buffs.h"
#include "FileTypes\STB.h"

const dword_t Status::getBuffsVisuality(byte_t buffType) { 
	return this->buffs.getVisuality(buffType); 
}
bool Status::checkBuffs() {
	return this->buffs.checkBuffs();
}

bool Status::addBuff(const byte_t visualityBit, const word_t amount, const dword_t timeInMilliseconds) {
	return this->buffs.addBuff(visualityBit, amount, timeInMilliseconds);
}

NPCData::NPCData(NPCSTB* stb, const word_t rowId) {
	this->id = rowId;
	this->name = stb->getName(rowId);
	this->walkSpeed = stb->getWalkSpeed(rowId);
	this->sprintSpeed = stb->getSprintSpeed(rowId);
	this->level = stb->getLevel(rowId);
	this->hpPerLevel = stb->getHPperLevel(rowId);
	this->maxHP = this->hpPerLevel * this->level;
	this->attackPower = stb->getAttackpower(rowId);
	this->hitrate = stb->getHitrate(rowId);
	this->defense = stb->getDefense(rowId);
	this->magicDefense = stb->getMagicDefense(rowId);
	this->dodgeRate = stb->getDodgerate(rowId);
	this->attackSpeed = stb->getAttackspeed(rowId);
	this->expPerLevel = stb->getExperience(rowId);
	this->attackRange = stb->getAttackrange(rowId);
	this->AIId = stb->getAIFileId(rowId);
	this->isNPC = stb->isNPCEntry(rowId);
	this->moneyPercentage = stb->getMoneyChance(rowId);
	this->dropTableId = stb->getDroptableId(rowId);
	this->questHash = ::makeQuestHash(stb->getQuestName(rowId).c_str());
	this->dialogId = 0x00;
}