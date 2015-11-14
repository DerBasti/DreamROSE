#include "Entity.h"
#include "Player.h"
#include "Monster.h"
#include "..\WorldServer.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

Entity::Entity() {
	this->combat.target = nullptr;
	this->status.buffs.clearBuff();
	this->visibleSectors.clear();
	this->position.lastSectorCheckTime.start();
	this->updateStats();
	this->entityInfo.nearestSector = nullptr;
	this->entityInfo.ingame = true;
	this->entityInfo.id = 0x00; //invalid id
}

Entity::~Entity() {
	if(this->getSector())
		this->getSector()->removeEntity(this);
	this->entityInfo.ingame = false;
	mainServer->getMap(this->getMapId())->freeLocalId(this);
}

void Entity::setPositionCurrent(const position_t& newPos) {
	this->position.current = newPos; 
}

void Entity::setPositionDest(const position_t& newPos) {
	this->position.destination = newPos; 
	this->setPositionVisually(newPos);
}

void Entity::addAttacker(Entity* enemy) {
	if (enemy != nullptr) {
		this->combat.myAttackers.insert(std::pair<const word_t, Entity*>(enemy->getLocalId(), enemy));
	}
}
void Entity::removeAttacker(Entity* enemy) {
	if (enemy != nullptr) {
		this->combat.myAttackers.erase(enemy->getLocalId());
	}
}

void Entity::setTarget(Entity* target) { 
	if (target && target->getCurrentHP()>0) {

		if (this->getEntityType() != Entity::TYPE_PLAYER)
			this->setStance(Stance::NPC_RUNNING);

		Packet pak( PacketID::World::Response::INIT_BASIC_ATTACK);
		pak.addWord(this->getLocalId());
		pak.addWord(target->getLocalId());
		pak.addWord( this->getMovementSpeed() );
		pak.addFloat( target->getCurrentX() );
		pak.addFloat( target->getCurrentY() );

		this->sendToVisible(pak);
		target->addAttacker(this);
	}
	this->combat.type = Combat::NORMAL;

	//Remove previous target
	if (this->getTarget() != nullptr) {
		this->getTarget()->removeAttacker(this);
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

Entity* Entity::getVisibleEntity(const word_t localId) const {
	UniqueSortedList<dword_t, Map::Sector*> list = this->getVisibleSectors();
	for (unsigned int i = 0; i < list.size(); i++) {
		Map::Sector* sector = list.getValueAtPosition(i);
		LinkedList<Entity*>::Node* current = sector->getFirstEntity();
		do {
			if (current != nullptr && current->getValue() != nullptr && current->getValue()->getLocalId() == localId) {
				return current->getValue();
			}
		} while ((current = sector->getNextEntity(current)) != nullptr);
	}
	return false;
}

bool Entity::isVisible(const word_t localId) const {
	return (this->getVisibleEntity(localId) != nullptr);
}

bool Entity::isVisible(const Entity* entity) const {
	if (entity == nullptr) {
		return false;
	}
	return this->isVisible(entity->getLocalId());
}

bool Entity::playAnimation() {
	//not necessary.
	if (this->getTarget() == nullptr && this->animation == nullptr) {
		return true;
	}
	word_t framesAlreadyPlayed = this->animation.getFramesAlreadyPlayed();
	word_t framesToPlay = this->animation.getFramesToPlay(this->getAttackSpeed());
	if (framesToPlay <= framesAlreadyPlayed) { //in case nothing happened yet
		return true;
	}

	bool attackSuccess = true;
	word_t currentFrame = framesAlreadyPlayed;
	for (; (currentFrame < framesToPlay) && (currentFrame < this->animation.getFrameAmount()); currentFrame++) {
		word_t frameType = this->animation.getFrameType(currentFrame);
		switch (frameType) {
			case ZMO::MOTION_MELEE_ATTACK:
			case ZMO::MOTION_RANGED_ATTACK:
			case ZMO::MOTION_MAGIC_ATTACK:
			case ZMO::MOTION_SKILL_MELEE_SINGLE:
				if (this->isPlayer()) {
					Player* curPlayer = dynamic_cast<Player*>(this);
					if (curPlayer->isInDebugMode()) {
						ChatService::sendDebugMessage(curPlayer, "Proc'ing attack frame [%i of type %i] now (of frames to play: %i).\n", currentFrame, this->animation.getFrameType(currentFrame), framesToPlay);
					}
				}
				attackSuccess = this->attackEnemy();
			break;
			case ZMO::MOTION_SKILL_MELEE:
				if (!this->getCurrentSkill()) {
					throw NullpointerException("[%s] tried to execute a skill without a valid assignment!\n", this->getName().c_str());
				}
				if (this->isPlayer()) {
					Player* curPlayer = dynamic_cast<Player*>(this);
					if (curPlayer->isInDebugMode()) {
						ChatService::sendDebugMessage(curPlayer, "Attacking [%s] with skill %i at [%i of %i]\n", this->getTarget()->getName().c_str(), this->combat.skill->getId(), currentFrame, this->animation.getFrameAmount());
					}
				}
				if (this->getCurrentSkill()->getAOERange() > 0) {
					float distance = this->getCurrentSkill()->getAOERange();
					std::vector<Entity*> enemiesInSight = this->getCurrentSkill()->getInitRange() == 0 ? this->getAllVisibleEntitiesWithinDistance(distance, false) : this->getTarget()->getAllVisibleEntitiesWithinDistance(distance, false);
					for (dword_t i = 0; i < enemiesInSight.size(); i++) {
						attackSuccess &= this->attackEnemy(enemiesInSight[i]);
					}
				}
				else {
					attackSuccess = this->attackEnemy();
				}
			break;
			case ZMO::MOTION_SKILL_RANGED:
				if (this->isPlayer()) {
					Player* curPlayer = dynamic_cast<Player*>(this);
					if (curPlayer->isInDebugMode()) {
						ChatService::sendDebugMessage(curPlayer, "Attacking [%s] with skill %i at [%i of %i]\n", this->getTarget()->getName().c_str(), this->combat.skill->getId(), currentFrame, this->animation.getFrameAmount());
					}
				}
				//attackSuccess = this->isSkillForAllies() ? this->buffAllies() : this->attackEnemy();
			break;
		}
	}
	if (currentFrame >= this->animation.getFrameAmount()) {
		if (this->getTarget() == nullptr) {
			this->animation = nullptr;
		} else {
			float distance = this->getPositionCurrent().distanceTo(this->getTarget()->getPositionCurrent());
			if (distance >= this->getAttackRange()) {
				this->animation = nullptr;
			}
			else {
				this->animation.reset(this->getAttackSpeed(), framesToPlay);
			}
		}
	} else{
		this->animation.setFramesAlreadyPlayed(framesToPlay);
	}
	return attackSuccess;
}

byte_t Entity::movementRoutine() {
	if (this->animation != nullptr) {
		this->position.lastCheckTime.update();
		if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
			ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "Still in attack animation...\n");
		}
		return Movement::TARGET_REACHED;
	}
	byte_t result = Movement::IDLE;
	bool isAttacking = this->getTarget() != nullptr && !this->getTarget()->isDrop();
	if (isAttacking) {
		this->position.destination = this->getTarget()->getPositionCurrent();
	}
	float distance = this->position.current.distanceTo(this->position.destination);
	float threshold = isAttacking ? this->getAttackRange() : 10;
	if (distance <= threshold) {
		if (isAttacking) {
			if (this->animation == nullptr) {
				result = Movement::INITIAL_ATTACK;
			}
			else {
				result = Movement::TARGET_REACHED;
			}
#ifdef __ROSE_DEBUG__
			if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
				ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "isAttacking: %i, Distance: %f, Distance-Threshold: %f", isAttacking, distance, threshold);
			}
#endif
		}
		this->position.destination = this->position.current;
		this->position.lastCheckTime.update();
		return result;
	}
	float xDiff = this->position.destination.x - this->position.current.x;
	float yDiff = this->position.destination.y - this->position.current.y;

	long long timePassed = this->position.lastCheckTime.getDuration();
	this->position.lastCheckTime.update();
	float timeNecessary = (distance / static_cast<float>(this->getMovementSpeed()) * 1000.0f);
	if (timePassed >= timeNecessary) {
#ifdef __ROSE_DEBUG__
		if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
			ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "Distance: %f, Time passed: %ims, Time necessary: %ims", distance, timePassed, timeNecessary);
		}
#endif
		this->position.current = this->position.destination;
		return result;
	}
	float totalDiff = sqrt(xDiff*xDiff + yDiff*yDiff);
	float pathTraveled = timePassed * static_cast<float>(this->getMovementSpeed()) / 1000.0f;
	float xRatio = pathTraveled * (xDiff / totalDiff);
	float yRatio = pathTraveled * (yDiff / totalDiff);
	position_t newPos(xRatio + this->position.current.x,
		yRatio + this->position.current.y);

#ifdef __ROSE_DEBUG__
	if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
		ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "New Position: %.2f, %.2f", newPos.x, newPos.y);
	}
#endif

	this->position.current = newPos;
	return Movement::IS_MOVING;
}

bool Entity::attackEnemy(Entity* overrideTarget) { 
	Entity *enemy = this->getTarget() == nullptr ? overrideTarget : this->getTarget();
	if (!enemy) {
		return false;
	}
	word_t damage = 0x00;
	word_t defense = 0x00;
	if(this->getEntityType() == Entity::TYPE_PLAYER) {
		if((this->getAttackPower() + 20) > (enemy->getDefensePhysical() * 7 / 10 + 5))
			damage = (this->getAttackPower() + 20) - (enemy->getDefensePhysical() * 7 / 10 + 5);
		else
			damage = 5;
	}
	else {
		if((this->getAttackPower() * 0.8f) > (this->getDefensePhysical() * 0.8f))
			damage = static_cast<word_t>((this->getAttackPower() * 0.8f) - (this->getDefensePhysical() * 0.8f));
		else
			damage = 5;
	}

	damage += QuickInfo::round<word_t>(static_cast<float>(rand() / static_cast<float>(RAND_MAX)) * damage * 0.2f);
	word_t flag = 0x0000; //0x2000 = hit-animation; 0x4000 = crit, = 0x8000 = Dead
	
	bool success = enemy->addDamage(this, damage, flag);
	if (flag & 0x8000 && enemy->getEntityType() != Entity::TYPE_PLAYER) {
		this->setTarget(nullptr);

		delete enemy;
		enemy = nullptr;

		//this->setPositionDest(this->getPositionCurrent());
	}
	return success;
}

bool Entity::addDamage(Entity* damageDealer, const word_t damage, word_t& flag) {

	//Just in case...
	if (this->isNPC()) {
		return false;
	}
	this->onDamageReceived(damageDealer, damage);
	if (damage >= this->getCurrentHP()) {
		this->stats.curHP = 0x00;
		flag |= 0x8000;

		this->onDeath();
	}
	else {
		this->stats.curHP -= damage;
	}
	if (damageDealer->combat.type != Combat::NORMAL) {
		flag |= 0x2000;
		Packet pak(PacketID::World::Response::SKILL_DAMAGE);
		pak.addWord(this->getLocalId());
		pak.addWord(damageDealer->getLocalId());
		pak.addWord(damageDealer->combat.skill->getIdBasic());
		pak.addByte(0x00);
		pak.addWord((damage & 0x7FF) | flag);
		pak.addDWord(0x04); //?
		bool result = this->sendToVisible(pak);

		pak.newPacket(PacketID::World::Response::SKILL_ANIMATION);
		pak.addWord(damageDealer->getLocalId());
		pak.addWord(damageDealer->combat.skill->getIdBasic());
		return (result & this->sendToVisible(pak));
	}
	Packet pak(PacketID::World::Response::BASIC_ATTACK);
	pak.addWord(damageDealer->getLocalId());
	pak.addWord(this->getLocalId());
	pak.addWord((damage & 0x7FF) | flag);
	if (flag & 0x8000) {
		//in case the monster dies, send it to everybody
		return this->sendToVisible(pak);
	}
	this->sendToList(pak, this->combat.myAttackers, true);
	return true;
}

Map::Sector* Entity::checkForNewSector() {
	long long lastCheckTime = this->getLastSectorCheckTime();
	if (lastCheckTime >= Map::Sector::DEFAULT_CHECK_TIME) {
		Map::Sector* nearestSector =  mainServer->getMap(this->getMapId())->getSector(this->getPositionCurrent());
		if(nearestSector != this->entityInfo.getSector()) {
			return nearestSector;
		}
		this->position.lastSectorCheckTime.update();
	}
	return nullptr;
}

bool Entity::isSectorVisible(Map::Sector* sector) const {
	if (!sector)
		return false;
	UniqueSortedList<dword_t, Map::Sector*>& current = this->getVisibleSectors();
	if (current.find(sector->getId(), sector) >= 0) {
		return true;
	}
	return false;
}

void Entity::refreshSector(Map::Sector* sectorToRefresh) {
	if (this->isSectorVisible(sectorToRefresh)) {
		this->removeSectorVisually(sectorToRefresh);
	}
	this->addSectorVisually(sectorToRefresh);
}

void Entity::refreshVisuality() {
	UniqueSortedList<dword_t, Map::Sector*>& current = this->getVisibleSectors();
	for (dword_t i = 0; i < current.size(); i++) {
		Map::Sector* sector = current.getValueAtPosition(i);
		this->refreshSector(sector);
	}
}

void Entity::checkVisuality() {
	if(!this->getSector())
		return;
	Map* currentMap = mainServer->getMap(this->getMapId());
	UniqueSortedList<dword_t, Map::Sector*> newSectorList;

	//Get all the surrounding sectors; possible duplicates are eliminated
	//via internal checks in "UniqueSortedList"
	newSectorList.add(this->getSector()->getId(), this->getSector());
	for(unsigned int i=0;i<Map::Sector::SURROUNDING_MAX;i++) {
		Map::Sector* surSector = currentMap->getSurroundingSector( this->getSector(), i );
		newSectorList.add( surSector->getId(), surSector );
	}
	//In case everything remained the same, do nothing
	if(newSectorList == this->visibleSectors)
		return;
	//visualityLog.putString("NewSectorList != currentVisibleSectors!\n");
	Map::Sector* sector = nullptr;
	for(unsigned int i=0;i<this->visibleSectors.size();) {
		//Previously found sectors which are still in sight are not "new"
		if(newSectorList.findKey(this->visibleSectors.getKey(i)) != static_cast<size_t>(-1)) {
			sector = newSectorList.getValueAtPosition(newSectorList.findKey(this->visibleSectors.getKey(i)));
		//	visualityLog.putStringWithVarOnly("[ALREADY VISIBLE] Sector %i\n", sector->getId(), sector->getCenter().x, sector->getCenter().y);
#ifdef __ROSE_DEBUG__
			if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
				ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "Sector %i is already visible", sector->getId());
			}
#endif
			newSectorList.removeByKey(this->visibleSectors.getKey(i));
			i++;
		} else { 
			//Sectors, who are now out of sight are to be removed
			sector = this->visibleSectors.getValueAtPosition(i);
		//	visualityLog.putStringWithVarOnly("[OUT OF SIGHT] Sector %i [%f, %f]\n", sector->getId() , sector->getCenter().x, sector->getCenter().y);
#ifdef __ROSE_DEBUG__
			if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
				ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "Sector %i is now ", sector->getId());
			}
#endif
			this->removeSectorVisually(sector);
			this->visibleSectors.removeAt(i);
		}
	}
	//Remaining new sectors are now to be added
	for(unsigned int i=0;i<newSectorList.size();i++) {
		sector = newSectorList.getValueAtPosition(i);
#ifdef __ROSE_DEBUG__
		if (this->isPlayer() && dynamic_cast<Player*>(this)->isInDebugMode()) {
			ChatService::sendDebugMessage(dynamic_cast<Player*>(this), "Adding Sector with ID %i to visuality", sector->getId());
		}
#endif
		this->addSectorVisually(sector);
		this->visibleSectors.add(sector->getId(), sector);
	}
	//visualityLog.putStringWithVarOnly("-=-=-=-=-=-=-=-=-\n\n", sector->getId(), sector->getCenter().x, sector->getCenter().y);
}

std::vector<Entity*> Entity::getAllVisibleEntities() const {
	std::vector<Entity*> result;
	for (unsigned int i = 0; i < this->getVisibleSectors().size(); i++) {
		Map::Sector* sector = this->getVisibleSectors().getValueAtPosition(i);
		LinkedList<Entity*>::Node* currentNode = sector->getFirstEntity();
		while (currentNode != nullptr) {
			if (currentNode->getValue() != nullptr) {
				Entity* entity = currentNode->getValue();
				if (entity->isIngame()) {
					result.push_back(entity);
				}
			}
			currentNode->getNextNode();
		}
	}
	return result;
}

std::vector<Entity*> Entity::getAllVisibleEntitiesWithinDistance(const float distance, bool includeDrops) const {
	std::vector<Entity*> result;
	for (unsigned int i = 0; i < this->getVisibleSectors().size(); i++) {
		Map::Sector* sector = this->getVisibleSectors().getValueAtPosition(i);
		LinkedList<Entity*>::Node* currentNode = sector->getFirstEntity();
		while (currentNode != nullptr) {
			if (currentNode->getValue() != nullptr) {
				Entity* entity = currentNode->getValue();
				if (entity->isIngame() && this->getPositionCurrent().distanceTo(entity->getPositionCurrent()) <= distance) {
					if (entity->getEntityType() != Entity::TYPE_DROP || (entity->getEntityType() == Entity::TYPE_DROP && includeDrops)) {
						result.push_back(entity);
					}
				}
			}
			currentNode->getNextNode();
		}
	}
	return result;
}

/*
template<class _Ty> _Ty* Entity::getStatType(const word_t statType) {
	switch (statType) {
		case StatType::ATTACK_POWER:
			return &this->stats.attackPower;
		case StatType::ATTACK_SPEED:
			return &this->stats.attackSpeed;
		case StatType::CRIT_RATE:
			return &this->stats.critRate;
		case StatType::CURRENT_HP:
			return &this->stats.curHP;
		case StatType::CURRENT_MP:
			return &this->stats.curMP;
		case StatType::DEFENSE_MAGICAL:
			return &this->stats.defenseMagical;
		case StatType::DEFENSE_PHYSICAL:
			return &this->stats.defensePhysical;
		case StatType::DODGE_RATE:
			return &this->stats.dodgeRate;
		case StatType::EXPERIENCE_RATE:
			return nullptr;
		case StatType::HIT_RATE:
			return &this->stats.hitRate;
		case StatType::LEVEL:
			return &this->getLevel();
		case StatType::MAX_HP:
			return &this->stats.maxHP();
		case StatType::MAX_MP:
			return &this->stats.maxMP();
		case StatType::MOVEMENT_SPEED:
			return &this->stats.getMovementSpeed();
	}
	if (this->getEntityType() == Entity::TYPE_PLAYER) {
		Player* player = dynamic_cast<Player*>(this);
		return player->getSpecialStatType(statType);
	}
	return nullptr;
}
*/

void Entity::addSectorVisually(Map::Sector* newSector) {
	LinkedList<Entity*>::Node* pNode = newSector->getFirstEntity();
	for(;pNode;pNode = pNode->getNextNode()) {
		Entity* entity = pNode->getValue();	
		if(!entity || !entity->isIngame() || entity == this)
			continue;

		//Two-way  removal
		this->addEntityVisually(entity);
		entity->addEntityVisually(this);
	}
}

void Entity::removeSectorVisually(Map::Sector* toRemove) {
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
		Map::Sector* curSector = this->visibleSectors.getValueAtPosition(i);
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
		Map::Sector* curSector = currentMap->getSector(i);
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

bool Entity::sendToList(Packet& pak, const std::map<const word_t, Entity*>& list, bool includeThisEntity) {
	bool result = true;
	auto it = list.begin();
	for (; it != list.end(); it++) {
		Entity* current = it->second;
		if (!current || !current->isPlayer()) {
			continue;
		}
		result &= dynamic_cast<Player*>(current)->sendData(pak);
	}
	if (includeThisEntity && this->isPlayer()) {
		result &= dynamic_cast<Player*>(this)->sendData(pak);
	}
	return result;
}

bool Entity::sendToList(Packet& pak, const std::vector<Entity*>& list, bool includeThisEntity) {
	bool result = true;
	auto it = list.begin();
	for (; it != list.end(); it++) {
		Entity* current = (*it);
		if (!current || !current->isPlayer()) {
			continue;
		}
		result &= dynamic_cast<Player*>(current)->sendData(pak);
	}
	if (includeThisEntity && this->isPlayer()) {
		result &= dynamic_cast<Player*>(this)->sendData(pak);
	}
	return result;
}

LinkedList<Entity*>::Node* Entity::setSector(Map::Sector* newSector) {
#ifdef __MAPSECTOR_LL__
	LinkedList<Entity*>::Node* returnNode = nullptr;
	try {
#ifdef __ROSE_MULTI_THREADED__
		sectorMutex.lock();
#endif
		if (this->entityInfo.getSector() != nullptr) {
			returnNode = this->entityInfo.getSector()->removeEntity(this);
		}
		if (newSector) {
			newSector->addEntity(this);
			this->entityInfo.needsVisualityUpdate = true;
		}
		this->entityInfo.setSector(newSector);
#ifdef __ROSE_MULTI_THREADED__
		sectorMutex.unlock();
#endif
	}
	catch (std::exception& ex) {
		GlobalLogger::fatal("EXCEPTION: %s", ex.what());
	}
	return returnNode;
#else
	this->entityInfo.sector = newSector;
#endif
}