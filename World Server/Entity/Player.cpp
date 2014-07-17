#include "Player.h"
#include "Monster.h"
#include "..\WorldServer.h"
#include <stdlib.h>

CMyFile visualityLog;

Player::Player(SOCKET sock, ServerSocket* server){ 
	visualityLog.openFile("D:\\Games\\iROSE Online Server\\Visuality.log", "a+");
	this->socket = sock;
	this->serverDelegate = server;
	this->entityInfo.type = Entity::TYPE_PLAYER;
	for (unsigned int i = 0; i < Inventory::MAXIMUM; i++) {
		this->inventory[i].clear();
	}
}

Player::~Player() {
	Map* map = mainServer->getMap(this->getMapId());
	if(map) {
		MapSector* sector = map->getSector(this->getPositionCurrent());
		if(sector) sector->removeEntity(this);
	}
	this->entityInfo.ingame = false;
}

void Player::setPositionCurrent(const Position& newPos) {
	this->position.current = newPos;
	this->setPositionVisually(newPos);
}

void Player::setPositionDest(const Position& newPos) {
	this->position.destination = newPos;
	this->setPositionVisually(newPos);
}

bool Entity::setPositionVisually(const Position& newPos) {
	Entity* target = this->combat.getTarget();

	Packet pak(PacketID::World::Response::MOVEMENT);
	pak.addWord( this->getClientId() );
	pak.addWord( target != nullptr ? target->getClientId() : 0x00 );
	pak.addWord( this->getMovementSpeed() );
	pak.addFloat( newPos.x );
	pak.addFloat( newPos.y );
	pak.addWord( 0xcdcd ); //Z
	
	this->position.lastCheckTime = clock();

	return this->sendToVisible(pak);
}

void Player::addSectorVisually(MapSector* sector) {
	LinkedList<Entity*>::Node* eNode = sector->getFirstEntity();
	//visualityLog.putStringWithVarOnly("New Sector %i: [%f, %f][%f, %f]\n", this->getSector()->getId(), this->getSector()->getCenter().x, this->getSector()->getCenter().x, this->getCurrentX(), this->getCurrentY());
	while(eNode) {
		Entity *curEntity = eNode->getValue();
		eNode = eNode->getNextNode();				//??? just in case
		if(!curEntity || !curEntity->isIngame() || curEntity == this) {
			continue;
		}
		//visualityLog.putStringWithVarOnly("Add Monster[%s]: [%i @Sector %i [%f, %f]][%f, %f]\n", curEntity->getName().c_str(), curEntity->getClientId(), curEntity->getSector()->getId(), curEntity->getSector()->getCenter().x, curEntity->getSector()->getCenter().x,  curEntity->getCurrentX(), curEntity->getCurrentY());
		this->addEntityVisually(curEntity);
	}
}

void Player::removeSectorVisually(MapSector* sector) {
	LinkedList<Entity*>::Node* eNode = sector->getFirstEntity();
	while(eNode) {
		Entity *curEntity = eNode->getValue();
		eNode = eNode->getNextNode();
		if(!curEntity || !curEntity->isIngame() || curEntity == this)
			continue;
		//visualityLog.putStringWithVarOnly("Remove Monster[%s]: [%i @Sector %i [%f, %f]][%f, %f]\n", curEntity->getName().c_str(), curEntity->getClientId(), curEntity->getSector()->getId(), curEntity->getSector()->getCenter().x, curEntity->getSector()->getCenter().x,  curEntity->getCurrentX(), curEntity->getCurrentY());
		this->pakRemoveEntityVisually(curEntity);
	}
}

void Player::addEntityVisually(Entity* entity) {
	if(!entity || entity == this)
		return;
	Player* player = nullptr; 
	switch(entity->getEntityType()) {
		case Entity::TYPE_PLAYER:
			player = dynamic_cast<Player*>(entity);
			//this->pakSpawnPlayer(player);
			//player->pakSpawnPlayer(this);
		break;
		case Entity::TYPE_NPC:
			this->pakSpawnNPC(dynamic_cast<NPC*>(entity));
		break;
		case Entity::TYPE_MONSTER:
			this->pakSpawnMonster(dynamic_cast<Monster*>(entity));
		break;
	}
}
		
bool Player::pakRemoveEntityVisually(Entity* entity) {
	Packet pak(PacketID::World::Response::REMOVE_VISIBLE_PLAYER);
	pak.addWord(entity->getClientId());
	return this->sendData(pak);
}


void Player::updateAttackpower() {
	this->stats.attackPower = 15;
}

void Player::updateDefense() {
	this->stats.defensePhysical = 100;
}

void Player::updateMagicDefense() {
	this->stats.defenseMagical = 100;
}

void Player::updateHitrate() {
	this->stats.hitRate = 100;

}
void Player::updateDodgerate() {
	this->stats.dodgeRate = 100;
}

void Player::updateCritrate() {
	this->stats.critRate = 10;
}
void Player::updateMovementSpeed() {
	this->stats.movementSpeed = 425;
}

bool Player::isAllied( Entity* entity ) {
	Player* player = dynamic_cast<Player*>(entity);
	if(player)
		return this->isAllied(player);
	NPC* npc = dynamic_cast<NPC*>(entity);
	if(npc)
		return this->isAllied(npc);
	return this->isAllied(dynamic_cast<Monster*>(entity)); 
}

bool Player::pakExit() {
	Packet pak(PacketID::World::Response::EXIT);
	pak.addWord(0x00);
	this->sendData(pak);
	this->setIsActive(false);
	return true;
}

bool Player::pakPlayerInfos() {
	this->position.current.x = 520000;
	this->position.current.y = 520000;
	this->position.destination = this->position.current;
	this->entityInfo.mapId = 22;

	mainServer->changeToMap(this, this->getMapId());
	
	Packet pak(PacketID::World::Response::PLAYER_INFOS);
	pak.addByte(this->getVisualTraits().sex);
	pak.addWord(this->entityInfo.mapId);
	pak.addFloat(this->position.current.x);
	pak.addFloat(this->position.current.y);
	pak.addWord(0x00); //SAVED POSITION
	pak.addDWord(this->getVisualTraits().faceStyle);
	pak.addDWord(this->getVisualTraits().hairStyle);

	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::HEADGEAR]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::ARMOR]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::GLOVES]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::SHOES]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::FACE]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::BACK]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::WEAPON]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::SHIELD]));

	pak.addByte(0x00); //BIRTHSTONE
	pak.addWord(0x00); //BIRTHPLACE?
	pak.addWord(this->getJob());
	pak.addByte(0x00); //UNION
	pak.addByte(0x00); //RANK
	pak.addByte(0x00); //FAME

	pak.addWord(this->attributes.strength);
	pak.addWord(this->attributes.dexterity);
	pak.addWord(this->attributes.intelligence);
	pak.addWord(this->attributes.concentration);
	pak.addWord(this->attributes.charm);
	pak.addWord(this->attributes.sensibility);
	pak.addWord(this->stats.curHP);
	pak.addWord(this->stats.curMP);

	pak.addDWord(this->charInfo.experience);
	pak.addWord(this->charInfo.level);
	pak.addWord(this->charInfo.statPoints);
	pak.addWord(this->charInfo.skillPoints);
	pak.addByte(0x64); //BodySize
	pak.addByte(0x64); //HeadSize
	pak.addDWord(0x00);
	pak.addDWord(0x00);

	//UNION POINTS for each fraction
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	//END: UNION POINTS for each fraction

	pak.addDWord(0x00);
	pak.addDWord(0x00);
	pak.addByte(0x00);
	pak.addWord(this->stats.stamina);
	for (short i = 0; i < 0x146; i++) {
		pak.addByte(0x00); //?
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(this->basicSkills[i].id); //BASIC SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //ACTIVE SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //PASSIVE SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //NOT USED
	}
	for (unsigned int i = 0; i < 32; i++) {
		pak.addWord(0x00); //QUICK BAR
	}

	pak.addDWord(this->charInfo.id);
	pak.addString(this->charInfo.name.c_str());
	pak.addWord(0x00);
	return this->sendData(pak);
}

bool Player::pakInventory() {
	Packet pak(PacketID::World::Response::PLAYER_INVENTORY);
	pak.addQWord(this->charInfo.zulies);

	for (unsigned int i = 0; i < Inventory::MAXIMUM; i++) {
		pak.addWord(mainServer->buildItemHead(this->inventory[i]));
		pak.addDWord(mainServer->buildItemData(this->inventory[i]));
	}
	return this->sendData(pak);
}

bool Player::pakQuestData() {
	Packet pak(PacketID::World::Response::QUEST_DATA);
	
	for (unsigned int i = 0; i < 5; i++) //EPISODE
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 3; i++) //JOB
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 7; i++) //PLANET
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 10; i++) //UNION
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 10; i++) {
		pak.addWord(0x00); //QuestID
		pak.addDWord(0x00); //QuestTime

		for (unsigned int j = 0; j < 10; j++) 
			pak.addWord(0x00); //VARS
		
		pak.addDWord(0x00); //SWITCHES
		for (unsigned int j = 0; j < 6; j++) {
			pak.addWord(0x00); //ITEM HEAD
			pak.addDWord(0x00); //ITEM DATA
		}
	}
	for (unsigned int i = 0; i < 0x40; i++)
		pak.addByte(0x00);

	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //ITEM HEAD
		pak.addDWord(0x00);
	}
	
	return this->sendData(pak);
}

bool Player::loadInfos() {
	if (!mainServer->sqlRequest("SELECT lastChar_Id, accesslevel FROM accounts WHERE id=%i", this->accountInfo.id)) {
		return false;
	}
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	this->charInfo.id = atoi(row[0]);
	this->accountInfo.accessLevel = static_cast<BYTE>(atoi(row[1]));
	mainServer->sqlFinishQuery();

	if (!mainServer->sqlRequest("SELECT name, level, experience, job, face, hair, sex, zulies FROM characters WHERE id=%i", this->charInfo.id))
		return false;

	row = mainServer->sqlGetNextRow();
	this->charInfo.name = std::string(row[0]);
	this->charInfo.level = atoi(row[1]);
	this->charInfo.experience = atol(row[2]);
	this->charInfo.job = atoi(row[3]);
	this->charInfo.visualTraits.faceStyle = atoi(row[4]);
	this->charInfo.visualTraits.hairStyle = atoi(row[5]);
	this->charInfo.visualTraits.sex = atoi(row[6]);
	this->charInfo.zulies = static_cast<QWORD>(::atol(row[7]));
	this->charInfo.skillPoints = this->charInfo.statPoints = 0x00;

	if (!mainServer->sqlRequest("SELECT basicSkills FROM character_skills WHERE id=%i", this->charInfo.id))
		return false;
	if (mainServer->sqlGetRowCount() != 1)
		return false;
	std::string str = std::string(mainServer->sqlGetNextRow()[0]);
	DWORD idx = 0x00;
	while (str.find(",") != -1) {
		this->basicSkills[idx].id = atoi(str.substr(0, str.find(",")).c_str());
		idx++;

		//min_value of str.find is 0 -> no check necessary
		str = str.substr(str.find(",")+1);
	}
	this->basicSkills[idx].id = atoi(str.c_str());

	this->entityInfo.mapId = 22;

	mainServer->sqlFinishQuery();

	return true;
}

bool Player::pakPing() {
	Packet pak(PacketID::World::Response::PING);
	pak.addByte(0x00);
	return this->sendData(pak);
}

bool Player::pakIdentify() {
	this->accountInfo.id = this->packet.getDWord(0x00);

	Packet pak(PacketID::World::Response::IDENFITY);
	pak.addByte(0x00); //??
	pak.addDWord(0x87654321); //Encryption
	pak.addDWord(0x00);
	if (!this->sendData(pak))
		return false;

	if (!this->loadInfos())
		return false;

	this->pakPlayerInfos();
	this->pakInventory();
	this->pakQuestData();

	pak.newPacket(PacketID::World::Response::GAMING_PLAN);
	pak.addWord(0x1001); //?
	pak.addDWord(0x02); //Unlimited plan
	return this->sendData(pak);
}

bool Player::pakReturnToCharServer() {
	Packet pak(PacketID::World::Response::EXIT);
	pak.addWord(0x00);
	if(!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::MESSAGE_MANAGER);
	pak.addByte(0xFA);
	pak.addDWord(this->accountInfo.id);
	if(!this->sendData(pak))
		return false;

	this->setIsActive(false);
	return true;
}

bool Player::pakAssignID(){
	//????
	this->updateStats();
	mainServer->assignClientID(this);

	Packet pak(PacketID::World::Response::UNKNOWN);
	pak.addWord(0x22);
	pak.addWord(0x02);
	pak.addWord(0x00);
	if(!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::SPAWN_ON_MAP);
	pak.addWord(0x05);

	//Friendly map
	pak.addDWord(1085514317);
	if(!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::ASSIGN_ID);
	pak.addWord(this->entityInfo.id);
	pak.addWord(this->stats.curHP);
	pak.addWord(this->stats.curMP);
	pak.addDWord(this->charInfo.experience);
	pak.addDWord(0x00); //UNUSED

	//WORLD RATES
	pak.addWord(0x64);
	pak.addDWord(0x0C1F4B79);
	pak.addWord(0x64);
	pak.addDWord(0x3232cd50);
	pak.addDWord(0x32323235);
	pak.addDWord(0x35323232);

	//PVP-MAP (0 = false, 1 = true)
	pak.addWord(0x00);

	//MAP TIME
	pak.addDWord(mainServer->getMapTime(this->getMapId()));

	//White icon (friendly)
	pak.addWord(0x02);
	pak.addWord(0x00);

	if (!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::WEIGHT);
	pak.addWord(this->entityInfo.id);
	pak.addByte(0x00); //WEIGHT PERCENT
	if (!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::CHANGE_STANCE);
	pak.addWord(this->entityInfo.id);
	pak.addByte(this->getStance().asBYTE());
	pak.addWord(this->getMovementSpeed());

	this->position.lastCheckTime = clock();
	this->entityInfo.ingame = true;
	bool result = this->sendToVisible(pak);

	this->visibleSectors.clear();
	this->checkVisuality();
	return result;
}

bool Player::pakTerrainCollision() {
	this->position.destination = Position( this->packet.getFloat(0x00), this->packet.getFloat(0x04) );
	this->position.current = this->position.destination;
	Packet pak(PacketID::World::Response::TERRAIN_COLLISION);
	pak.addWord(this->entityInfo.id);
	pak.addFloat( this->packet.getFloat(0x00) );
	pak.addFloat( this->packet.getFloat(0x04) );
	pak.addWord( this->packet.getWord(0x0A) );
	return this->sendToVisible(pak);
}

bool Player::pakSetEmotion() {
	Packet pak(PacketID::World::Response::SET_EMOTION);
	pak.addDWord( this->packet.getDWord(0x00) );
	pak.addWord( this->entityInfo.id );
	return this->sendToVisible( pak );
}

bool Player::pakMoveCharacter() {
	this->combat.setTarget(nullptr);
	this->setPositionDest( Position(this->packet.getFloat(0x02), this->packet.getFloat(0x06)) );
	return true;
}

bool Player::pakChangeStance() {
	BYTE stanceType = this->packet.getByte(0x00);
	switch(stanceType) {
		case 0x00:
			if(this->getStance().asBYTE() == Stance::WALKING) {
				this->setStance(Stance::RUNNING);
			} else if(this->getStance().asBYTE() == Stance::RUNNING) {
				this->setStance(Stance::WALKING);
			}
		break;
		case 0x01:
			if(this->getStance().asBYTE() == Stance::SITTING) {
				this->setStance(Stance::RUNNING);
				break;
			}
			if(this->getStance().asBYTE() != Stance::DRIVING && this->getStance().asBYTE() != Stance::SITTING) {
				this->setStance(Stance::SITTING);
			}
		break;
		case 0x02:
			//DRIVING
		break;
	}
	stanceType = this->getStance().asBYTE();

	Packet pak(PacketID::World::Response::CHANGE_STANCE);
	pak.addWord( this->getClientId() );
	pak.addByte( stanceType );
	pak.addWord( this->getMovementSpeed() );
	return this->sendData(pak);
}

bool Player::pakLocalChat() {
	ChatService::sendMessage(this, this->packet.getString(0x00));
	return true;
}

bool Player::pakShoutChat() {
	ChatService::sendShout(this, this->packet.getString(0x00));
	return true;
}

bool Player::pakSpawnPlayer(Player* player) {
	Packet pak(PacketID::World::Response::SPAWN_PLAYER);
	pak.addWord( player->getClientId() );
	pak.addFloat( player->getCurrentX() );
	pak.addFloat( player->getCurrentY() );
	pak.addFloat( player->getDestinationX() );
	pak.addFloat( player->getDestinationY() );
	if(player->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(player->getPositionCurrent() != player->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(player->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( (player->getTarget() == nullptr ? 0x00 : player->getTarget()->getClientId() ) );
	switch(player->getStance().asBYTE()) {
		case Stance::WALKING:
			pak.addByte( EntitySpawnsVisually::STANCE_WALKING );
		break;
		case Stance::RUNNING:
			pak.addByte( EntitySpawnsVisually::STANCE_RUNNING );
		break;
		case Stance::DRIVING:
			pak.addByte( EntitySpawnsVisually::STANCE_DRIVING );
		break;
		default:
			pak.addByte( EntitySpawnsVisually::STANCE_HITCHHIKER );
	}
	pak.addDWord( player->getBuffsVisuality() );
	BYTE pvpMapType = 0x00; //mainServer->isPVPMap(this->getMapId())
	if(pvpMapType) {
		//TODO:
		pak.addWord( EntitySpawnsVisually::IS_FRIENDLY );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_FRIENDLY );
	}
	pak.addWord(0x00);
	pak.addDWord(0x00);
	pak.addByte( player->getVisualTraits().sex );
	pak.addWord( player->getMovementSpeed() );
	pak.addWord( player->getAttackSpeed() );
	pak.addByte( 0x01 ); //Weight?

	pak.addDWord( player->getVisualTraits().faceStyle );
	pak.addDWord( player->getVisualTraits().hairStyle );
	
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::HEADGEAR] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::ARMOR] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::GLOVES] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::SHOES] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::FACE] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::BACK] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::WEAPON] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::SHIELD] ) );
	
	for(unsigned int i=Inventory::ARROWS;i<=Inventory::CANNONSHELLS;i++)
		pak.addWord( mainServer->buildItemHead( player->inventory[i] ) );

	pak.addWord( player->getJob() );
	pak.addByte( player->getLevel() );

	for(unsigned int i=Inventory::CART_FRAME;i<=Inventory::CART_ABILITY;i++)
		pak.addDWord( player->inventory[i].id | 0x400 );

	pak.addWord( 0xcdcd ); //Z-Coord?
	pak.addDWord( 0x00 ); //weird DWORD, additional buffs/status?

	pak.addString( player->getName().c_str() );
	pak.addByte(0x00);
	//TODO: IF player has clan
	/*
		if(player->getClan()) {
			pak.addDWord( player->getClan()->getId() );
			pak.addWord( player->getClan()->getBackground() );
			pak.addWord( player->getClan()->getLogo() );
			pak.addByte( player->getClan()->getGrade() );
			pak.addByte( player->getClan()->getRank(player) );
			pak.addString( player->getClan()->getName().c_str() );
			pak.addByte( 0x00 );
		}
	*/

	return player->sendData(pak);
}

bool Player::pakSpawnNPC( NPC* npc ) {
	Packet pak(PacketID::World::Response::SPAWN_NPC);
	pak.addWord(npc->getClientId());
	pak.addFloat(npc->getCurrentX());
	pak.addFloat(npc->getCurrentY());
	pak.addFloat(npc->getDestinationX()); 
	pak.addFloat(npc->getDestinationY()); 
	if(npc->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(npc->getPositionCurrent() != npc->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(npc->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( npc->getTarget() != nullptr ? npc->getTarget()->getClientId() : 0x00 );
	switch(npc->getStance().asBYTE()) {
		case Stance::WALKING:
			pak.addByte( EntitySpawnsVisually::STANCE_WALKING );
		break;
		case Stance::RUNNING:
			pak.addByte( EntitySpawnsVisually::STANCE_RUNNING );
		break;
		case Stance::DRIVING:
			pak.addByte( EntitySpawnsVisually::STANCE_DRIVING );
		break;
		default:
			pak.addByte( EntitySpawnsVisually::STANCE_HITCHHIKER );
	}
	pak.addDWord( npc->getCurrentHP() );
	pak.addDWord( 0x00 ); //FRIENDLY
	pak.addDWord( npc->getBuffsVisuality() );
	pak.addWord(npc->getTypeId());
	if(npc->hasDialogId())
		pak.addWord(npc->getDialogId());
	else
		pak.addWord(npc->getTypeId() - 900);
	
	pak.addFloat(npc->getDirection());
	pak.addByte(0x00); //CLANFIELD OPEN/CLOSED FOR BURLAND (NPC: 1115)
	pak.addByte(0x00);

	return this->sendData(pak);
}

bool Player::pakSpawnMonster(Monster* monster) {
	Packet pak(PacketID::World::Response::SPAWN_MONSTER);
	pak.addWord(monster->getClientId());
	pak.addFloat(monster->getCurrentX());
	pak.addFloat(monster->getCurrentY());
	pak.addFloat(monster->getDestinationX());
	pak.addFloat(monster->getDestinationY());
	if(monster->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(monster->getPositionCurrent() != monster->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(monster->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( monster->getTarget() != nullptr ? monster->getTarget()->getClientId() : 0x00 );
	pak.addByte( monster->getStance().asBYTE() );
	pak.addDWord(monster->getCurrentHP());
	pak.addDWord(0x64); //ENEMY
	pak.addDWord( monster->getBuffsVisuality() );
	pak.addWord(monster->getTypeId());
	pak.addWord(0x00); //Quest?
	return this->sendData(pak);
}

bool Player::pakTelegate() {
	WORD telegateId = this->packet.getWord(0x00);
	Telegate& gate = mainServer->getGate(telegateId);

	float distanceToGate = this->getPositionCurrent().distanceTo(gate.getSourcePosition());

	this->setPositionCurrent(gate.getDestPosition());
	return this->pakTelegate(gate.getDestMap(), gate.getDestPosition());
}

bool Player::pakTelegate(const WORD mapId, const Position& pos) {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValue(i));
	}
	this->visibleSectors.clear();

	Packet pak(PacketID::World::Response::TELEGATE);
	pak.addWord(this->getClientId());
	pak.addWord(mapId);
	pak.addFloat(pos.x);
	pak.addFloat(pos.y);
	pak.addWord(0x01);
	if(!this->sendData(pak))
		return false;

	this->entityInfo.ingame = false;

	this->setPositionCurrent(pos);
	this->setPositionDest(pos);
	mainServer->changeToMap(this, mapId);

	return true;
}

bool Player::handlePacket() {
	std::cout << "[IN] New Packet: " << std::hex << this->packet.getCommand() << " with Length " << std::dec << this->packet.getLength() << "\n";
	switch (this->packet.getCommand()) {
		case PacketID::World::Request::PING:
			return true;

		case PacketID::World::Request::CHANGE_STANCE:
			return this->pakChangeStance();

		case PacketID::World::Request::EXIT:
			return false; //DISCONNECT

		case PacketID::World::Request::GET_ID:
			return this->pakAssignID();

		case PacketID::World::Request::IDENFITY:
			return this->pakIdentify();

		case PacketID::World::Request::LOCAL_CHAT:
			return this->pakLocalChat();

		case PacketID::World::Request::MOVEMENT:
			return this->pakMoveCharacter();

		case PacketID::World::Request::RETURN_TO_CHARSERVER:
			return this->pakReturnToCharServer();

		case PacketID::World::Request::SET_EMOTION:
			return this->pakSetEmotion();

		case PacketID::World::Request::SHOUT_CHAT:
			return this->pakShoutChat();

		case PacketID::World::Request::TELEGATE:
			return this->pakTelegate();

		case PacketID::World::Request::TERRAIN_COLLISION:
			return this->pakTerrainCollision();
	}
	return true;
}