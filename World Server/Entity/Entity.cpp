#include "Entity.h"
#include "Player.h"
#include "Monster.h"
#include "..\WorldServer.h"

Entity::Entity() {
	this->combat.target = nullptr;
	this->status.buffs.clearBuff();
	this->visibleSectors.clear();
	this->position.lastSectorCheckTime = 0x00;
	this->updateStats();
	this->entityInfo.nearestSector = nullptr;
	this->entityInfo.ingame = true;
	this->entityInfo.id = 0x00; //invalid id
}

Entity::~Entity() {
	if(this->getSector())
		this->getSector()->removeEntity(this);
	this->entityInfo.ingame = false;
	mainServer->freeClientId(this);
}

void Entity::setPositionCurrent(const Position& newPos) { 
	this->position.current = newPos; 
}

void Entity::setPositionDest(const Position& newPos) { 
	this->position.destination = newPos; 
	this->setPositionVisually(newPos);
}

void Entity::setTarget(Entity* target) { 
	if(target && target->getCurrentHP()>0) {
		Packet pak( PacketID::World::Response::INIT_BASIC_ATTACK);
		pak.addWord( this->getClientId() );
		pak.addWord( target->getClientId() );
		pak.addWord ( this->getMovementSpeed() );
		pak.addFloat( target->getCurrentX() );
		pak.addFloat( target->getCurrentY() );
		if (this->getEntityType() != Entity::TYPE_PLAYER)
			this->setStance( Stance::NPC_RUNNING );
		this->sendToVisible(pak);
	}
	this->combat.setTarget(target); 
}

bool Entity::isAllied( Entity* entity ) {
	switch(entity->getEntityType()) {
		case Entity::TYPE_PLAYER:
			return this->isAllied( dynamic_cast<Player*>(entity) );
		case Entity::TYPE_NPC:
			return this->isAllied( dynamic_cast<NPC*>(entity) );
		case Entity::TYPE_MONSTER:
			return this->isAllied( dynamic_cast<Monster*>(entity) );
	}
	return true;
}


bool Entity::movementRoutine() {
	//In case there is no change in the wanted position, do nothing
	if(this->getTarget() == nullptr) {
		if(this->position.current == this->position.destination) {
			this->position.lastCheckTime = clock();
			return false;
		}
	} else { //We have a target, let's find out if we're in attack range
		float distToTarget = this->position.current.distanceTo(this->getTarget()->getPositionCurrent());
		
		float range = 100.0f;
		if(!this->isAllied(this->getTarget()))
			range = this->getAttackRange();
		if(distToTarget <= range) {
			this->position.lastCheckTime = clock();
			return false;
		} else {
			this->position.destination = this->getTarget()->getPositionCurrent();
		}
	}
	//Calculate the difference between each point (x and y)
	float fX = this->position.destination.x - this->position.current.x;
	float fY = this->position.destination.y - this->position.current.y;

	float distance = this->position.destination.distanceTo(this->position.current);
	clock_t timePassed = clock() - this->position.lastCheckTime;
	float neededTime = distance * 1000 / this->getMovementSpeed();

	//In case the time which has passed since the last check is bigger
	//than the time needed to reach the point, or a marginal distance is
	//necessary to get to the destination (10cm), we just say we've reached
	//the destination just now
	if(timePassed >= neededTime || distance <= 10.0f) {
		this->position.current = this->getPositionDest();
	} else {
		//Otherwise we calculate the new current position based on the
		//previously calculated point-distances and multiply them with the
		//percentage of the timepassed compare to the needed time
		//E.g. 50% ratio out of timePassed/neededTime -> 50% covered
		Position newCurrent( static_cast<float>(fX * (timePassed/neededTime) + this->getCurrentX() ),
							 static_cast<float>(fY * (timePassed/neededTime) + this->getCurrentY() )
						   );
		this->position.current = newCurrent;
	}
	this->position.lastCheckTime = clock();
	return true;
}

bool Entity::attackRoutine() {
	if(this->getTarget() == nullptr)
		return false;
	if(clock() - this->combat.lastAttackTime <= this->intervalBetweenAttacks())
		return false;
	return this->attackEnemy();
}

bool Entity::attackEnemy() { 
	Entity *enemy = this->getTarget();
	if (!enemy) {
		std::cout << "attackEnemy() was called by " << this->getName().c_str() << "(" << this->getClientId() << ") without a target!\n";
		return false;
	}
	WORD damage = 0x00;
	WORD defense = 0x00;
	printf("%s(%i) [%i] attacks %s(%i) [%i]\n", this->getName().c_str(), this->getClientId(),
		this->intervalBetweenAttacks(), enemy->getName().c_str(), enemy->getClientId(),
		enemy->intervalBetweenAttacks());
	if(this->getEntityType() == Entity::TYPE_PLAYER) {
		if((this->getAttackPower() + 20) > (enemy->getDefensePhysical() * 7 / 10 + 5))
			damage = (this->getAttackPower() + 20) - (enemy->getDefensePhysical() * 7 / 10 + 5);
		else
			damage = 5;
	}
	else {
		if((this->getAttackPower() * 0.8f) > (this->getDefensePhysical() * 0.8f))
			damage = static_cast<WORD>((this->getAttackPower() * 0.8f) - (this->getDefensePhysical() * 0.8f));
		else
			damage = 5;
	}

	damage += QuickInfo::round<WORD>(static_cast<float>(rand() / static_cast<float>(RAND_MAX)) * damage * 0.1f);
	WORD flag = 0x0000; //0x2000 = hit-animation; 0x4000 = crit, = 0x8000 = Dead
	
	bool success = enemy->addDamage( this, damage, flag);
	if (flag & 0x8000 && enemy->getEntityType() != Entity::TYPE_PLAYER) {
		delete enemy;
		enemy = nullptr;

		this->setTarget(nullptr);
	}

	this->combat.lastAttackTime = clock();
	return success;
}

bool Entity::addDamage(Entity* enemy, const WORD damage, WORD& flag) {
	if (damage >= this->getCurrentHP()) {
		this->stats.curHP = 0x00;
		flag |= 0x8000;

		this->onDeath();
	}
	else {
		this->stats.curHP -= damage;
		this->onDamageReceived(enemy, damage);
	}
	Packet pak(PacketID::World::Response::BASIC_ATTACK);
	pak.addWord(enemy->getClientId());
	pak.addWord(this->getClientId());
	pak.addWord((damage & 0x7FF) | flag);
	return this->sendToVisible(pak);
}

MapSector* Entity::checkForNewSector() {
	if (this->getLastSectorCheckTime() >= MapSector::DEFAULT_CHECK_TIME) {
		MapSector* nearestSector =  mainServer->getMap(this->getMapId())->getSector(this->getPositionCurrent());
		if(nearestSector != this->entityInfo.getSector()) {
			return nearestSector;
		}
		this->position.lastSectorCheckTime = clock() + MapSector::DEFAULT_CHECK_TIME; 
	}
	return nullptr;
}

void Entity::checkVisuality() {
	if(!this->getSector())
		return;
	Map* currentMap = mainServer->getMap(this->getMapId());
	UniqueSortedList<DWORD, MapSector*> newSectorList;

	//Get all the surrounding sectors; possible duplicates are eliminated
	//via internal checks in "UniqueSortedList"
	newSectorList.add(this->getSector()->getId(), this->getSector());
	for(unsigned int i=0;i<MapSector::SURROUNDING_MAX;i++) {
		MapSector* surSector = currentMap->getSurroundingSector( this->getSector(), i );
		newSectorList.add( surSector->getId(), surSector );
	}
	//In case everything remained the same, do nothing
	if(newSectorList == this->visibleSectors)
		return;
	//visualityLog.putString("NewSectorList != currentVisibleSectors!\n");
	MapSector* sector = nullptr;
	for(unsigned int i=0;i<this->visibleSectors.size();) {
		//Previously found sectors which are still in sight are not "new"
		if(newSectorList.findKey(this->visibleSectors.getKey(i)) != static_cast<size_t>(-1)) {
			sector = newSectorList.getValue(newSectorList.findKey(this->visibleSectors.getKey(i)));
		//	visualityLog.putStringWithVarOnly("[ALREADY VISIBLE] Sector %i\n", sector->getId(), sector->getCenter().x, sector->getCenter().y);
			newSectorList.removeByKey(this->visibleSectors.getKey(i));
			i++;
		} else { 
			//Sectors, who are now out of sight are to be removed
			sector = this->visibleSectors.getValue(i);
		//	visualityLog.putStringWithVarOnly("[OUT OF SIGHT] Sector %i [%f, %f]\n", sector->getId() , sector->getCenter().x, sector->getCenter().y);
			this->removeSectorVisually(sector);
			this->visibleSectors.removeAt(i);
		}
	}
	//Remaining new sectors are now to be added
	for(unsigned int i=0;i<newSectorList.size();i++) {
		sector = newSectorList.getValue(i);
		//visualityLog.putStringWithVarOnly("[ADDED] Sector %i [%f, %f]!\n", sector->getId(), sector->getCenter().x, sector->getCenter().y);
		MapSector* sector = newSectorList.getValue(i);
		this->addSectorVisually(sector);
		this->visibleSectors.add(sector->getId(), sector);
	}
	//visualityLog.putStringWithVarOnly("-=-=-=-=-=-=-=-=-\n\n", sector->getId(), sector->getCenter().x, sector->getCenter().y);
}

void Entity::addSectorVisually(MapSector* newSector) {
	LinkedList<Entity*>::Node* pNode = newSector->getFirstEntity();
	for(;pNode;pNode = pNode->getNextNode()) {
		Entity* entity = pNode->getValue();	
		if(!entity || !entity->isIngame() || entity == this)
			continue;

		//Two-way removal
		this->addEntityVisually(entity);
		entity->addEntityVisually(this);
	}
}

void Entity::removeSectorVisually(MapSector* toRemove) {
	LinkedList<Entity*>::Node* pNode = toRemove->getFirstEntity();
	for(;pNode;pNode = pNode->getNextNode()) {
		Entity* entity = pNode->getValue();
		if(!entity || !entity->isIngame() || entity == this)
			continue;

		//Two-way removal
		this->removeEntityVisually(entity);
		entity->removeEntityVisually(this);
	}
}

bool Entity::sendToVisible(Packet& pak, Entity* exceptThis) {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		MapSector* curSector = this->visibleSectors.getValue(i);
		LinkedList<Entity*>::Node* pNode = curSector->getFirstPlayer();
		for(;pNode;pNode = curSector->getNextPlayer(pNode)) {
			Player* player = dynamic_cast<Player*>(pNode->getValue());
			if(!player || !player->isIngame() || pNode->getValue() == exceptThis)
				continue;
			player->sendData(pak);
		}
	}
	return true;
}

bool Entity::sendToMap(Packet& pak) {
#ifdef __MAPSECTOR_LL__
	Map* currentMap = mainServer->getMap(this->getMapId());
	for(unsigned int i=0;i<currentMap->getSectorCount();i++) {
		MapSector* curSector = currentMap->getSector(i);
		if(!curSector) //Not likely gonna happen
			continue; 
		LinkedList<Entity*>::Node* eNode = curSector->getFirstPlayer();
		for(;eNode;eNode = curSector->getNextPlayer(eNode)) {
			Player* player = dynamic_cast<Player*>(eNode->getValue());
			if(!player || !player->isIngame())
				continue;
			player->sendData(pak);
		}
	}
	return true;
#else
	return false; //??
#endif
}

void Entity::setSector(MapSector* newSector) {
#ifdef __MAPSECTOR_LL__
	if(this->entityInfo.getSector() != nullptr) {
		this->entityInfo.getSector()->removeEntity(this);
	}
	if(newSector) {
		newSector->addEntity(this);
	}
	this->entityInfo.setSector(newSector);
#else
	this->entityInfo.sector = newSector;
#endif
}