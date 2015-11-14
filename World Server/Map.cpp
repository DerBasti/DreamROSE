#include "Map.h"
#include "Entity\NPC.h"
#include "Entity\Monster.h"
#include "WorldServer.h"
#include "D:\Programmieren\QuickInfos\LinkedList.h"

Map::Sector::Sector() {
#ifdef __MAPSECTOR_LL__
	this->entitiesInSector.clear();
#endif
	this->center = position_t(520000.0f, 520000.0f);
}

Map::Sector::Sector(const Map::Sector& rhs) {
	(*this) = rhs;
}

Map::Sector::~Sector() {
#ifdef __MAPSECTOR_LL__
	this->entitiesInSector.clear();
#endif
}

Map::Sector& Map::Sector::operator=(const Map::Sector& rhs) {
	if((&rhs) == this) {
		return (*this);
	}
#ifdef __MAPSECTOR_LL__
	this->entitiesInSector.clear();
#endif
	this->center = rhs.center;
	return (*this);
}

#ifdef __MAPSECTOR_LL__
LinkedList<Entity*>::Node* Map::Sector::getNext(LinkedList<Entity*>::Node* curNode, byte_t entityType) const {
	if(!curNode)
		return nullptr;
	while (curNode) {
		if (curNode->getValue() && curNode->getValue()->getEntityType() == entityType)
			return curNode;
		curNode = curNode->getNextNode();
	}
	return nullptr;
}

LinkedList<Entity*>::Node* Map::Sector::getFirstEntity() const {
	return this->entitiesInSector.getHeadNode();
}
		
LinkedList<Entity*>::Node* Map::Sector::getNextEntity(LinkedList<Entity*>::Node* curNode) const {
	if (curNode) {
		return curNode->getNextNode();
	}
	return nullptr;
}

LinkedList<Entity*>::Node* Map::Sector::getFirstPlayer() const {
	return this->getNext(this->entitiesInSector.getHeadNode(), Entity::TYPE_PLAYER);
}
		
LinkedList<Entity*>::Node* Map::Sector::getNextPlayer(LinkedList<Entity*>::Node* curNode) const {
	return this->getNext((curNode == nullptr ? nullptr : curNode->getNextNode()), Entity::TYPE_PLAYER);
}
	
LinkedList<Entity*>::Node* Map::Sector::getFirstNPC() const {
	return this->getNext(this->entitiesInSector.getHeadNode(), Entity::TYPE_NPC);
}

LinkedList<Entity*>::Node* Map::Sector::getNextNPC(LinkedList<Entity*>::Node* curNode) const {
	return this->getNext((curNode == nullptr ? nullptr : curNode->getNextNode()), Entity::TYPE_NPC);
}

bool Map::Sector::exists(const Entity* entity) const {
	return (this->find(entity->getLocalId()) != nullptr);
}

Entity* Map::Sector::find(const word_t id) const {
	LinkedList<Entity*>::Node* current = this->getFirstEntity();
	do {
		Entity* value = current->getValue();
		if (value->getLocalId() == id) {
			return value;
		}
	} while ((current = this->getNextEntity(current)) != nullptr);
	return nullptr;
}
#endif

Map::Map() {
	this->isPVPOn = false;
	this->dayCycle.length = 0x01;
	this->mapPath = std::string("");
	this->sectorSize = IFO::CUSTOMIZED_SECTOR_SIZE;

	for (unsigned int i = 0; i<0x10000; i++)
		this->clientIDs[i] = nullptr;
}

Map::Map(const Map& rhs) {
	(*this) = rhs;
}
	
const Map& Map::operator=(const Map& rhs) {
	this->isPVPOn = rhs.isPVPOn;
	this->dayCycle.length = rhs.dayCycle.length;
	this->mapPath = rhs.mapPath;
	return (*this);
}
word_t Map::assignLocalId(Entity* newEntity) {
	if (!newEntity)
		return 0;

	//ClientID 0 = invalid state.
	for (unsigned int i = 1; i<0x10000; i++) {
		Entity*& entity = this->clientIDs[i];
		if (!entity) {
			this->clientIDs[i] = newEntity;
			newEntity->setLocalId(i);
			return i;
		}
	}
	return 0;
}

void Map::freeLocalId(Entity* toDelete) {
	this->clientIDs[toDelete->getLocalId()] = nullptr;
	toDelete->setLocalId(0x00);
}


void Map::setDayCycle(const dword_t totalLen, const dword_t morningBegin, const dword_t noonBegin, const dword_t eveningBegin, const dword_t nightBegin) {
	this->dayCycle.length = totalLen;
	this->dayCycle.morning = morningBegin;
	this->dayCycle.day = noonBegin;
	this->dayCycle.evening = eveningBegin;
	this->dayCycle.night = nightBegin;
}

byte_t Map::getDayTime(const dword_t worldTime) const {
	if(this->dayCycle.nightOnly)
		return 0x03;
	dword_t currentTime = getLocalTime(worldTime);
	if(currentTime >= this->dayCycle.night)
		return Map::NIGHT;
	if(currentTime >= this->dayCycle.evening)
		return Map::EVENING;
	if(currentTime >= this->dayCycle.day)
		return Map::NOON;
	return Map::MORNING;
}		

Map::Sector* Map::getSector(const word_t sectorId) const {
	try {
		dword_t subLevel = this->mapSectors.capacitySubLevel();
		word_t xSector = static_cast<WORD>(sectorId / subLevel);
		word_t ySector = static_cast<WORD>(sectorId % subLevel);
		return this->mapSectors.getValue(xSector, ySector);
	} catch(...) { }
	return nullptr;
}

Map::Sector* Map::getSector(const position_t& pos) const {
	Map::Sector* firstSector = this->mapSectors.getValue(0x00, 0x00);
	int firstIdTmp = QuickInfo::round<int>((pos.x - firstSector->getCenter().x) / this->getSectorWidthAndHeight());
	int secondIdTmp = QuickInfo::round<int>((pos.y - firstSector->getCenter().y) / this->getSectorWidthAndHeight());
	word_t realFirstId = 0x00;
	word_t realSecondId = 0x00;
	if(firstIdTmp < 0)
		firstIdTmp = 0;
			
	realFirstId = firstIdTmp;
	if(realFirstId >= this->mapSectors.capacityTopLevel())
		realFirstId = static_cast<WORD>(this->mapSectors.capacityTopLevel() - 1);

	if(secondIdTmp < 0)
		secondIdTmp = 0;

	realSecondId = secondIdTmp;
  	if(realSecondId >= this->mapSectors.capacitySubLevel())
		realSecondId = static_cast<WORD>(this->mapSectors.capacitySubLevel() - 1);
	try {
		word_t sectorId = static_cast<WORD>(realFirstId * this->mapSectors.capacitySubLevel() + realSecondId);
		return this->getSector(sectorId);
	} catch(...) { }
	return nullptr;
}

Player* Map::getPlayer(const word_t localId) const {
	LinkedList<Entity*>::Node* nNode = this->getFirstEntity();
	for (; nNode; nNode = nNode->getNextNode()) {
		Entity* entity = nNode->getValue();
		if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER)
			continue;
		Player* curPlayer = dynamic_cast<Player*>(entity);
		if (curPlayer->getLocalId() == localId)
			return curPlayer;
	}
	return nullptr;
}

NPC* Map::getNPC(const word_t type) const {
	LinkedList<Entity*>::Node* nNode = this->getFirstEntity();
	for(;nNode;nNode = nNode->getNextNode()) {
		Entity* entity = nNode->getValue();
		if(!entity || entity->getEntityType() != Entity::TYPE_NPC)
			continue;
		NPC* curNpc = dynamic_cast<NPC*>(entity);
		if(curNpc->getTypeId() == type)
			return curNpc;
	}
	return nullptr;
}

Player* Map::getPlayer(std::string& name) const {
	LinkedList<Entity*>::Node* nNode = this->getFirstEntity();
	for (; nNode; nNode = nNode->getNextNode()) {
		Entity* entity = nNode->getValue();
		if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER) {
			continue;
		}
		Player* curPlayer = dynamic_cast<Player*>(entity);
		if (curPlayer->getName().find(name) >= 0) {
			return curPlayer;
		}
	}
	return nullptr;
}

Map::Sector* Map::getSectorBySpawn(const IFOSpawn* spawn) const {
	return this->getSector(spawn->getPosition()); 
}

ZON::EventInfo* Map::getRespawn(position_t& pos) {
	ZON* zon = mainServer->getZON(this->getId());
	position_t nearestPos; float distance = 999999.9f;

	ZON::EventInfo* bestRespawn = nullptr;
	for(unsigned int i=0;i<zon->getEventInfoAmount();i++) {
		ZON::EventInfo& info = zon->getEventInfo(i);
		position_t infoPos = position_t(info.x, info.y);
		if(_stricmp(info.name.c_str(), "restore")==0) {
			if(distance > pos.distanceTo(infoPos)) {
				nearestPos = infoPos;
				distance = pos.distanceTo(infoPos);
				bestRespawn = &info;
			}
		}
	}
	return bestRespawn;
}

const position_t Map::getRespawnPoint(const char* spawnName) {
	ZON* zon = mainServer->getZON(this->getId());

	ZON::EventInfo* respawn = nullptr;
	for (unsigned int i = 0; i < zon->getEventInfoAmount(); i++) {
		if (_stricmp(zon->getEventInfo(i).name.c_str(), spawnName) == 0)
			return position_t(zon->getEventInfo(i).x, zon->getEventInfo(i).y);
	}
	return position_t(0.0f, 0.0f);
}

const word_t Map::getRespawnPointId(position_t& pos) {
	ZON::EventInfo* respawn = this->getRespawn(pos);
	if(respawn)
		return respawn->id;
	return 0x00;
}

const position_t Map::getRespawnPoint(const size_t pos) {
	ZON* zon = mainServer->getZON(this->getId());

	ZON::EventInfo* bestRespawn = nullptr;
	for(unsigned int i=0;i<zon->getEventInfoAmount();i++) {
		if(i == pos)
			return position_t(zon->getEventInfo(i).x, zon->getEventInfo(i).y);
	}
	return position_t(0.0f, 0.0f);
}

const position_t Map::getRespawnPoint(position_t& pos) {
	position_t retPos;
	ZON::EventInfo* respawn = this->getRespawn(pos);
	if(respawn) {
		retPos.x = respawn->x;
		retPos.y = respawn->y;
	}
	return retPos;
}

Map::Sector* Map::getSurroundingSector(Map::Sector* center, byte_t surroundingSectorType) {
	surroundingSectorType %= Map::Sector::SURROUNDING_MAX; //just to be safe
	int mapSectorId = center->getId();
	switch(surroundingSectorType) {
		case Map::Sector::UPPER_LEFT: //UPPER LEFT
		case Map::Sector::UPPER_CENTER: //UPPER
		case Map::Sector::UPPER_RIGHT: //UPPER RIGHT
				mapSectorId -= this->mapSectors.capacitySubLevel();
				mapSectorId += static_cast<int>(static_cast<int>(surroundingSectorType)-Map::Sector::UPPER_CENTER);
		break;
		case Map::Sector::CENTER_LEFT:
				mapSectorId--;
		break;
		case Map::Sector::CENTER_RIGHT: //CENTER RIGHT
				mapSectorId++;
		break;
		case Map::Sector::LOWER_LEFT: //LOWER LEFT
		case Map::Sector::LOWER_CENTER: //LOWER
		case Map::Sector::LOWER_RIGHT: //LOWER RIGHT
				mapSectorId += this->mapSectors.capacitySubLevel();
				mapSectorId += static_cast<int>(static_cast<int>(surroundingSectorType)-Map::Sector::LOWER_CENTER);
			break;
	}
	if(mapSectorId < 0x00)
		mapSectorId = 0x00;

	//can safely cast it as WORD, as it is certainly >= 0x00 -> unsigned
	else if(static_cast<WORD>(mapSectorId) >= this->mapSectors.capacity())
		mapSectorId = this->mapSectors.capacity() - 1;

	return this->getSector(static_cast<WORD>(mapSectorId));
}
void Map::findMinMax(std::vector<std::string>& files, word_t *x, word_t *y) {
	word_t curX = 0x00;
	word_t curY = 0x00;
	std::string curIFO = "";

	for(unsigned int i=0;i<files.size();i++) {
		curIFO = files.at(i);

		//Get the current X/Y-Id of the IFO 
		//e.g.: 31_30.ifo -> 31X; 30Y
		curX = atoi(curIFO.substr(curIFO.find_last_of("_")+1).c_str());
		curY = atoi(curIFO.substr(curIFO.find_last_of("\\")+1).c_str());

		//See whether the current X-Coordinate is the smallest sample size
		if(curX < x[0]) {
			x[0] = curX;
		} if(curX > x[1]) { //The other way around: the biggest sample size
			x[1] = curX;
		}

		//Just like above, just with Y-Coordinates
		if(curY < y[0]) {
			y[0] = curY;
		} if(curY > y[1]) {
			y[1] = curY;
		}
	}
}

void Map::createSectors(std::vector<std::string>& files) {
	if(files.empty())
		return;
	word_t minMaxX[2] = {0xFFFF, 0x00};
	word_t minMaxY[2] = {0xFFFF, 0x00};

	//Parse all *.ifo files to see which X/Y-Coordinates are the
	//local maxima/minima
	this->findMinMax(files, minMaxX, minMaxY);
	
	//calculate the center ((max - min) / 2) + min = IFO-X/Y
	word_t centerX = ((minMaxX[1] - minMaxX[0]) / 2) + minMaxX[0];
	word_t centerY = ((minMaxY[1] - minMaxY[0]) / 2) + minMaxY[0];
	
	//Just as precaution for having the same high and low values.
	//e.g. 30 for min and max -> 0 sectors difference.
	//Even if we get "unnecessary" sectors, it won't really take up
	//that much more memory.
	minMaxX[1]++;
	minMaxY[1]++;

	//520000 + offset from center * default_sector_size (16000)
	position_t min(520000.0f + static_cast<float>(static_cast<float>(minMaxX[0] - centerX)) * IFO::DEFAULT_SECTOR_SIZE,
				 520000.0f + static_cast<float>(static_cast<float>(minMaxY[0] - centerY)) * IFO::DEFAULT_SECTOR_SIZE
				);

	position_t max(520000.0f + static_cast<float>(static_cast<float>(minMaxX[1] - centerX)) * IFO::DEFAULT_SECTOR_SIZE,
				 520000.0f + static_cast<float>(static_cast<float>(minMaxY[1] - centerY)) * IFO::DEFAULT_SECTOR_SIZE
				);
	
	//New sector count = Default_Sector_Offsets / Custom_sector_size (8000)
	int sectorCountX = static_cast<int>((max.x - min.x) / this->getSectorWidthAndHeight())+1;
	int sectorCountY = static_cast<int>((max.y - min.y) / this->getSectorWidthAndHeight())+1;
	this->mapSectors.reserve(sectorCountX, sectorCountY);

	//Calculate all sector center positions
	for(int i=0;i<sectorCountX;i++) {
		for(int j=0;j<sectorCountY;j++) {
			float cX = min.x + i * this->getSectorWidthAndHeight();
			float cY = min.y + j * this->getSectorWidthAndHeight();

			Map::Sector* newSector = new Map::Sector();
			newSector->id = i * sectorCountY + j;
			newSector->setCenter( position_t(cX, cY) );
			
#ifdef __MAPSECTOR_DEBUG__
			newSector->mapId = this->id;
#endif
			this->mapSectors.addValue( i, j, newSector);
		}
	}
}

void Map::dumpSectors(const char* filePath) {
	CMyFileWriter<char> file(filePath, true);
	dword_t totalSectorCount = this->getSectorCount();
	file.putStringWithVar("MapID: %i\n", this->getId());
	for (unsigned int i = 0; i < totalSectorCount; i++) {
		Map::Sector* sector = this->getSector(i);
		file.putStringWithVar("[Sector %i of %i]: %f, %f with verticelength %i\n", i, totalSectorCount, sector->getCenter().x, sector->getCenter().y, this->getSectorWidthAndHeight());
	}
	file.putString("\n\n");
}
	
bool Map::hasActivePlayers() const {
	for (word_t i = 0; i < this->getSectorCount(); i++) {
		Map::Sector* sector = this->getSector(i);
		if (sector->getFirstPlayer() != nullptr) {
			return true;
		}
	}
	return false;
}

bool Map::checkSpawns() {
	for (unsigned int i = 0; i<this->getSpawnCount(); i++) {
		IFOSpawn* curSpawn = this->getSpawn(i);
		//As long as we didn't reach the allowed max, keep spawning monsters
		time_t lastCheckTime = time(nullptr) - curSpawn->getLastCheckTime();
		while (curSpawn->getCurrentlySpawned() < curSpawn->getMaxSimultanouslySpawned() && lastCheckTime >= curSpawn->getRespawnInterval()) {
			dword_t curSpawnId = curSpawn->getCurrentSpawnId();
			IFOSpawnEntry* selectedSpawn = nullptr;
			//Calculate the wanted spawnId and select the fitting spawnEntry
			if (curSpawnId >= curSpawn->getBasicMobSpawnCount()) {
				curSpawnId -= curSpawn->getBasicMobSpawnCount();
				selectedSpawn = &curSpawn->getTacticalMobSpawn(curSpawnId);
			}
			else {
				selectedSpawn = &curSpawn->getBasicMobSpawn(curSpawnId);
			}
			//Get the NPCData of the monster and spawn it
			//(also, add it to the map and thus visuality of the players)
			NPCData* npcData = mainServer->getNPCData(selectedSpawn->getMobId());
			for (unsigned int i = 0; i<selectedSpawn->getAmount(); i++) {
				position_t spawnPos(curSpawn->getPosition().x + QuickInfo::fRand(curSpawn->getAllowedSpawnDistance(), true),
					curSpawn->getPosition().y + QuickInfo::fRand(curSpawn->getAllowedSpawnDistance(), true));
				new Monster(npcData, mainServer->getAIData(selectedSpawn->getMobId()), this->getId(), spawnPos, curSpawn);
			}
			//Add the amount of spawned monsters to the total size of the spawn
			curSpawn->setCurrentlySpawned(curSpawn->getCurrentlySpawned() + selectedSpawn->getAmount());

			//Advance to the next spawnId; in case the max spawn was reached,
			//regress to the first basic spawn
			curSpawn->nextSpawnId();
		}
	}
	return true;
}
