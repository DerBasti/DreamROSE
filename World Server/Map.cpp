#include "Map.h"
#include "WorldServer.h"

MapSector::MapSector() {
#ifdef __MAPSECTOR_LL__
	this->entitiesInSector.clear();
#endif
	this->center = Position(520000, 520000);
}

MapSector::MapSector(const MapSector& rhs) {
	(*this) = rhs;
}

MapSector::~MapSector() {
#ifdef __MAPSECTOR_LL__
	this->entitiesInSector.clear();
#endif
}

MapSector& MapSector::operator=(const MapSector& rhs) {
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
LinkedList<Entity*>::Node* MapSector::getNext(LinkedList<Entity*>::Node* curNode, BYTE entityType) {
	if(!curNode)
		return nullptr;
	while (curNode) {
		if (curNode->getValue() && curNode->getValue()->getEntityType() == entityType)
			return curNode;
		curNode = curNode->getNextNode();
	}
	return nullptr;
}

LinkedList<Entity*>::Node* MapSector::getFirstEntity() {
	return this->entitiesInSector.getHeadNode();
}
		
LinkedList<Entity*>::Node* MapSector::getNextEntity(LinkedList<Entity*>::Node* curNode) {
	if (curNode) {
		return curNode->getNextNode();
	}
	return nullptr;
}

LinkedList<Entity*>::Node* MapSector::getFirstPlayer() {
	return this->getNext(this->entitiesInSector.getHeadNode(), Entity::TYPE_PLAYER);
}
		
LinkedList<Entity*>::Node* MapSector::getNextPlayer(LinkedList<Entity*>::Node* curNode) {
	return this->getNext((curNode == nullptr ? nullptr : curNode->getNextNode()), Entity::TYPE_PLAYER);
}
	
LinkedList<Entity*>::Node* MapSector::getFirstNPC() {
	return this->getNext(this->entitiesInSector.getHeadNode(), Entity::TYPE_NPC);
}

LinkedList<Entity*>::Node* MapSector::getNextNPC(LinkedList<Entity*>::Node* curNode) {
	return this->getNext((curNode == nullptr ? nullptr : curNode->getNextNode()), Entity::TYPE_NPC);
}
#endif

Map::Map() {
	this->isPVPOn = false;
	this->dayCycle.length = 0x01;
	this->mapPath = std::string("");
	this->sectorSize = IFO::CUSTOMIZED_SECTOR_SIZE;
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


void Map::setDayCycle(const DWORD totalLen, const DWORD morningBegin, const DWORD noonBegin, const DWORD eveningBegin, const DWORD nightBegin) {
	this->dayCycle.length = totalLen;
	this->dayCycle.morning = morningBegin;
	this->dayCycle.day = noonBegin;
	this->dayCycle.evening = eveningBegin;
	this->dayCycle.night = nightBegin;
}

BYTE Map::getDayTime(const DWORD worldTime) const {
	if(this->dayCycle.nightOnly)
		return 0x03;
	DWORD currentTime = getLocalTime(worldTime);
	if(currentTime >= this->dayCycle.night)
		return Map::NIGHT;
	if(currentTime >= this->dayCycle.evening)
		return Map::EVENING;
	if(currentTime >= this->dayCycle.day)
		return Map::NOON;
	return Map::MORNING;
}		

MapSector* Map::getSector(const WORD sectorId) {
	try {
		DWORD subLevel = this->mapSectors.capacitySubLevel();
		WORD xSector = sectorId / subLevel;
		WORD ySector = sectorId % subLevel;
		return this->mapSectors.getValue(xSector, ySector);
	} catch(...) { }
	return nullptr;
}

MapSector* Map::getSector(const Position& pos) {
	MapSector* firstSector = this->mapSectors.getValue(0x00, 0x00);
	int firstIdTmp = QuickInfo::round<int>((pos.x - firstSector->getCenter().x) / this->getSectorWidthAndHeight());
	int secondIdTmp = QuickInfo::round<int>((pos.y - firstSector->getCenter().y) / this->getSectorWidthAndHeight());
	WORD realFirstId = 0x00;
	WORD realSecondId = 0x00;
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
		WORD sectorId = static_cast<WORD>(realFirstId * this->mapSectors.capacitySubLevel() + realSecondId);
		return this->getSector(sectorId);
	} catch(...) { }
	return nullptr;
}

MapSector* Map::getSectorBySpawn(const IFOSpawn* spawn) {
	return this->getSector(spawn->getPosition()); 
}

MapSector* Map::getSurroundingSector(MapSector* center, BYTE surroundingSectorType) {
	surroundingSectorType %= MapSector::SURROUNDING_MAX; //just to be safe
	int mapSectorId = center->getId();
	switch(surroundingSectorType) {
		case MapSector::UPPER_LEFT: //UPPER LEFT
		case MapSector::UPPER_CENTER: //UPPER
		case MapSector::UPPER_RIGHT: //UPPER RIGHT
			mapSectorId -= this->mapSectors.capacitySubLevel();
			mapSectorId += static_cast<int>(static_cast<int>(surroundingSectorType) - MapSector::UPPER_CENTER);
		break;
		case MapSector::CENTER_LEFT:
			mapSectorId--;
		break;
		case MapSector::CENTER_RIGHT: //CENTER RIGHT
			mapSectorId++;
		break;
		case MapSector::LOWER_LEFT: //LOWER LEFT
		case MapSector::LOWER_CENTER: //LOWER
		case MapSector::LOWER_RIGHT: //LOWER RIGHT
			mapSectorId += this->mapSectors.capacitySubLevel();
			mapSectorId += static_cast<int>(static_cast<int>(surroundingSectorType) - MapSector::LOWER_CENTER);
		break;
	}
	if(mapSectorId < 0x00)
		mapSectorId = 0x00;

	//can safely cast it as WORD, as it is certainly >= 0x00 -> unsigned
	else if(static_cast<WORD>(mapSectorId) >= this->mapSectors.capacity())
		mapSectorId = this->mapSectors.capacity() - 1;

	return this->getSector(static_cast<WORD>(mapSectorId));
}
void Map::findMinMax(std::vector<std::string>& files, WORD *x, WORD *y) {
	WORD curX = 0x00;
	WORD curY = 0x00;
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
	WORD minMaxX[2] = {0xFFFF, 0x00};
	WORD minMaxY[2] = {0xFFFF, 0x00};

	//Parse all *.ifo files to see which X/Y-Coordinates are the
	//local maxima/minima
	this->findMinMax(files, minMaxX, minMaxY);
	
	//calculate the center ((max - min) / 2) + min = IFO-X/Y
	WORD centerX = ((minMaxX[1] - minMaxX[0]) / 2) + minMaxX[0];
	WORD centerY = ((minMaxY[1] - minMaxY[0]) / 2) + minMaxY[0];
	
	//Just as precaution for having the same high and low values.
	//e.g. 30 for min and max -> 0 sectors difference.
	//Even if we get "unnecessary" sectors, it won't really take up
	//that much more memory.
	minMaxX[1]++;
	minMaxY[1]++;

	//520000 + offset from center * default_sector_size (16000)
	Position min(520000.0f + static_cast<float>(static_cast<float>(minMaxX[0] - centerX)) * IFO::DEFAULT_SECTOR_SIZE,
				 520000.0f + static_cast<float>(static_cast<float>(minMaxY[0] - centerY)) * IFO::DEFAULT_SECTOR_SIZE
				);

	Position max(520000.0f + static_cast<float>(static_cast<float>(minMaxX[1] - centerX)) * IFO::DEFAULT_SECTOR_SIZE,
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

			MapSector* newSector = new MapSector();
			newSector->id = i * sectorCountY + j;
			newSector->setCenter( Position(cX, cY) );
			
#ifdef __MAPSECTOR_DEBUG__
			newSector->mapId = this->id;
#endif
			this->mapSectors.addValue( i, j, newSector);
		}
	}
}

void Map::dumpSectors(const char* filePath) {
	CMyFile file(filePath, "a+");
	DWORD totalSectorCount = this->getSectorCount();
	file.putStringWithVarOnly("MapID: %i\n", this->getId());
	for(unsigned int i=0;i<totalSectorCount;i++) {
		MapSector* sector = this->getSector(i);
		file.putStringWithVarOnly("[Sector %i of %i]: %f, %f with verticelength %i\n", i, totalSectorCount, sector->getCenter().x, sector->getCenter().y, this->getSectorWidthAndHeight());
	}
	file.putString("\n\n");
}
