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
	Player* player = dynamic_cast<Player*>(entity);
	if(player)
		return this->isAllied(player);
	NPC* npc = dynamic_cast<NPC*>(entity);
	if(npc)
		return this->isAllied(npc);
	return this->isAllied(dynamic_cast<Monster*>(entity)); 
}
