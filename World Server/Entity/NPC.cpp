#include "NPC.h"
#include "Monster.h"
#include "Player.h"
#include "..\WorldServer.h"

long NPC_ATTACK_INTERVAL = 250000;

void NPC::constructor(const NPCData* newData, const AIP* newAi, const WORD mapId, const Position& pos) {
	this->data = newData;
	this->ai = newAi;
	this->spawn = nullptr;

	this->entityInfo.ingame = true;
	this->entityInfo.type = Entity::TYPE_NPC;
	this->entityInfo.setMapId(mapId);
	this->position.source = pos;
	this->setPositionCurrent(pos);
	this->setPositionDest(pos);

	mainServer->assignClientID(this);

	this->lastAICheck = 0x00;

	for(unsigned int i=0;i<NPC::MAX_AIVAR;i++)
		this->aiVar[i]=0x00;
	for(unsigned int i=0;i<NPC::MAX_OBJVAR;i++)
		this->objVar[i] = 0x00;

	this->updateStats();

	if(this->data != nullptr && this->ai != nullptr) {
		this->stats.maxHP = this->stats.curHP = this->data->getHPPerLevel() * this->data->getLevel();
		Map* map = mainServer->getMap(this->getMapId());
		MapSector* sector = map->getSector(this->getPositionCurrent());
		this->setSector(sector);
		this->checkVisuality();
		AIService::run(this, AIP::ON_SPAWN);
	}
}

bool NPC::getAttackAnimation() { 
	this->combat.attackAnimation = mainServer->getAttackMotionNPC(this->getTypeId()); 
	return this->combat.attackAnimation != nullptr; 
}

WORD NPC::getNextAttackTime() const {
	if (this->combat.attackAnimation != nullptr) {
		if (this->combat.nextAttackId < this->combat.attackAnimation->getAttackTimerCount())
			return this->combat.attackAnimation->getAttackTimer(this->combat.nextAttackId) * this->data->getAttackspeed() / this->getAttackSpeed();
		return this->combat.attackAnimation->getTotalAnimationTime() + 500; //safe value to make sure nothing else happens during this time.
	}
	return 0;
}

WORD NPC::getTotalAttackAnimationTime() {
	if (this->combat.attackAnimation != nullptr) {
		//return this->combat.attackAnimation->getTotalAnimationTime() * this->data->getAttackspeed() / this->getAttackSpeed();
		return this->combat.attackAnimation->getTotalAnimationTime() * 100 / this->getAttackSpeed();
	}
	return 0;
}

void NPC::updateAttackpower() {
	if(this->data) {
		this->stats.attackPower = this->data->getAttackpower();
	}
}

void NPC::updateAttackSpeed() {
	if(this->data) {
		this->stats.attackSpeed = this->data->getAttackspeed();
	}
}
		
void NPC::updateDefense() {
	if(this->data) {
		this->stats.defensePhysical = this->data->getDefense();
	}
}

void NPC::updateMagicDefense() {
	if(this->data) {
		this->stats.defenseMagical = this->data->getMagicDefense();
	}
}

void NPC::updateHitrate() {
	if(this->data) {
		this->stats.hitRate = this->data->getHitrate();
	}
}

void NPC::updateDodgerate() {
	if(this->data) {
		this->stats.dodgeRate = this->data->getDodgerate();
	}
}

void NPC::setStance(const BYTE newStance) {
	this->status.stance = newStance;

	this->updateMovementSpeed();
}
		
void NPC::updateMovementSpeed() {
	if(this->data) {
		switch(this->status.getStance().asBYTE()) {
			case Stance::NPC_RUNNING:
				this->stats.movementSpeed = this->data->getSprintSpeed();
			break;
			case Stance::NPC_WALKING:
				this->stats.movementSpeed = this->data->getWalkspeed();
				return;
			break;
			default:
				this->stats.movementSpeed = 200;
		}
	}
}

bool NPC::onDamageReceived(Entity* enemy, const WORD damage) {
	AIService::run(this, AIP::ON_DAMAGED, enemy, damage);

	if (this->damageDealers.containsKey(enemy->getClientId())) {
		WORD id = enemy->getClientId();
		this->damageDealers.getValueByKey(id) += damage;
	}
	else {
		this->damageDealers.add(enemy->getClientId(), damage);
	}

	return true;
}

float NPC::getAttackRange() {
	if(this->data) {
		return this->data->getAttackRange();
	}
	return 100.0f; //1m
}

bool NPC::setPositionVisually(const Position& pos) {
	Entity* target = this->combat.getTarget();

	Packet pak(PacketID::World::Response::MOVEMENT_MONSTER);
	pak.addWord(this->getClientId());
	pak.addWord(target != nullptr ? target->getClientId() : 0x00);
	pak.addWord(this->getMovementSpeed());
	pak.addFloat(static_cast<float>(pos.x));
	pak.addFloat(static_cast<float>(pos.y));
	pak.addWord(0xcdcd); //Z-Axis
	pak.addByte(this->status.getStance().asBYTE());

	this->sendToVisible(pak);
	return true;
}

void NPC::setObjVar(BYTE idx, WORD newVal) { 
	if(idx == 0x00) {
		int before = this->getObjVar(idx);
		this->objVar[idx] = newVal;
		if(before != this->objVar[idx]) {
			//Event?
		}
	} else {
		this->objVar[idx] = newVal;
	}
}

const AIP* NPC::getAI() const {
	return this->ai;
}