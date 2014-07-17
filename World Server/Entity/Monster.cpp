#include "Monster.h"
#include "Player.h"
#include "..\WorldServer.h"

Monster::Monster(const NPCData* newData, const AIP* newAi, const WORD mapId, const Position& pos) {
	this->constructor(newData, newAi, mapId, pos);
	this->updateStats();
	this->entityInfo.type = Entity::TYPE_MONSTER;

	this->owner = nullptr;
}

Monster::~Monster() {
	this->data = nullptr;
	this->ai = nullptr;
	this->owner = nullptr;
}

bool Monster::isAllied( Entity* entity ) {
	switch(entity->getEntityType()) {
		case Entity::TYPE_PLAYER:
			return this->isAllied(dynamic_cast<Player*>(entity));
		case Entity::TYPE_NPC:
			return this->isAllied(dynamic_cast<NPC*>(entity));
		case Entity::TYPE_MONSTER:
			return this->isAllied(dynamic_cast<Monster*>(entity));
	}
	return true;
}
