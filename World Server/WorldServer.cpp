#include "WorldServer.h"
#include "Entity\Player.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"

#include "ConfigReader\Config.h"

WorldServer* mainServer;

WorldServer::WorldServer(WORD port, MYSQL* mysql) {
	this->port = port;
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;

	for(unsigned int i=0;i<0x10000;i++)
		this->clientIDs[i] = std::pair<WORD, Entity*>(i, nullptr);

	config = new ConfigA();
	config->init( (workingPath + std::string("\\config.conf")).c_str() );
	this->ipString = config->getValueString("WorldIp");
	
	this->WorldTime.lastCheck = time(NULL);
	this->WorldTime.currentTime = 0x00;

	this->loadSTBs();
	this->loadNPCData();
	this->loadMapData();
	this->loadAI();
	this->loadZones();
	
	for(unsigned int i=0;i<this->mapData.size();i++) {
		this->loadIFOs(this->mapData[i]);
		this->checkSpawns(this->mapData[i]);
	}
}

WorldServer::~WorldServer() {
	this->sqlDataBase.disconnect();

	for (unsigned int i = 1; i < 14; i++) {
		delete this->equipmentFile[i];
	}
	for(unsigned int i=0;i<this->zoneData.size();i++) {
		ZON* file = this->zoneData.getValue(i);
		delete file;
		file = NULL;
	}
	for(unsigned int i=0;i<this->mapData.size();i++) {
		Map* map = this->mapData.getValue(i);
		delete map;
		map = NULL;
	}
	delete this->skillFile;
	delete this->npcFile;
	delete ::config;

	this->npcData.clear();
}

void WorldServer::executeRequests() {
	this->checkWorldTime();
	for(unsigned int i=0;i<this->mapData.size();i++) {
		Map* curMap = this->mapData[i];
		this->runMap(curMap);
	}
}

void WorldServer::checkWorldTime() {
	if((time(NULL) - this->WorldTime.lastCheck) < WorldServer::worldTime::DEFAULT_WORLDTIME_CHECK)
		return;
	this->WorldTime.currentTime++;
	this->WorldTime.lastCheck = time(NULL);
}

void WorldServer::changeToMap(Entity* entity, const WORD newMapId) {
	if(!entity || newMapId >= this->mapData.size())
		return;
	//Assign the new mapId and the nearest sector on the new map
	entity->setMapId(newMapId);
	entity->setSector(this->getMap(entity->getMapId())->getSector(entity->getPositionCurrent()));
}

void WorldServer::runMap(Map* curMap) {
	if(!curMap)
		return;
	Player* curPlayer = nullptr;
	for(unsigned int i=0;i<curMap->getSectorCount();i++) {
		MapSector* curSector = curMap->getSector(i);
		LinkedList<Entity*>::Node* entityNode = curSector->getFirstEntity();
		while (entityNode) {
			Entity* curEntity = entityNode->getValue();
			entityNode = entityNode->getNextNode();
			if (!curEntity || !curEntity->isIngame()) {
				continue;
			}
			//If the movementRoutine() returns "false", no movement happened
			//negate the false => true => entity idles
			bool isIdling = !curEntity->movementRoutine();
			switch (curEntity->getEntityType()) {
				case Entity::TYPE_PLAYER:
					curPlayer = dynamic_cast<Player*>(curEntity);
					//Check whether we entered a new sector
					//If so, change the visuality vector
					/*bool newSector = */
 					if(curEntity->checkForNewSector()) {
						curPlayer->checkVisuality();
					}
				break;
				case Entity::TYPE_NPC:
				case Entity::TYPE_MONSTER:
					if (isIdling && curEntity->getTarget() == NULL) {
						//In case the NPC/Monster is idling
						//run AI with state "IDLE"
						NPC* curNPC = dynamic_cast<NPC*>(curEntity);
						AIService::run(curNPC, AIP::ON_IDLE);
					}
				break;
			}
		}
	}
}

bool WorldServer::checkSpawns(Map* currentMap) {
	for(unsigned int i=0;i<currentMap->getSpawnCount();i++) {
		IFOSpawn* curSpawn = currentMap->getSpawn(i);
		//As long as we didn't reach the allowed max, keep spawning monsters
		while( curSpawn->getCurrentlySpawned() < curSpawn->getMaxSimultanouslySpawned() ) {
			DWORD curSpawnId = curSpawn->getCurrentSpawnId();
			IFOSpawnEntry* selectedSpawn = nullptr;
			//Calculate the wanted spawnId and select the fitting spawnEntry
			if(curSpawnId >= curSpawn->getBasicMobSpawnCount()) {
				curSpawnId -= curSpawn->getBasicMobSpawnCount();
				selectedSpawn = &curSpawn->getTacticalMobSpawn(curSpawnId);
			} else {
				selectedSpawn = &curSpawn->getBasicMobSpawn(curSpawnId);
			}
			//Get the NPCData of the monster and spawn it
			//(also, add it to the map and thus visuality of the players)
			NPCData* npcData = &this->npcData.at(selectedSpawn->getMobId());
			for(unsigned int i=0;i<selectedSpawn->getAmount();i++) {
				Monster* newMon = new Monster( npcData, &this->aiData.getValue(npcData->getAIId()), currentMap->getId(), curSpawn->getPosition());
			}
			//Add the amount of spawned monsters to the total size of the spawn
			curSpawn->setCurrentlySpawned(curSpawn->getCurrentlySpawned() + selectedSpawn->getAmount() );
			
			//Advance to the next spawnId; in case the max spawn was reached,
			//regress to the first basic spawn
			curSpawn->nextSpawnId();
		}
	}
	return true;
}

bool WorldServer::loadSTBs() {

	std::cout << "0% done reading\r";
	float percentDone = 100.0f / 17.0f;
	float totalPercent = 0.0f;

	this->aiFile = new AISTB( (workingPath + std::string("\\3DDATA\\STB\\FILE_AI.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->npcFile = new NPCSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_NPC.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->skillFile = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_SKILL.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->zoneFile = new ZoneSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_ZONE.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::HEADGEAR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_CAP.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::ARMOR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BODY.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::GLOVES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_ARMS.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::BACK] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BACK.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::FACE] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FACEITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::SHOES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FOOT.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::JEWELRY] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEMITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::JEWELS] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEWEL.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::OTHER] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_NATURAL.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::PAT] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_PAT.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::SHIELD] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_SUBWPN.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::CONSUMABLES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_USEITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[Inventory::QUEST] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_QUESTITEM.STB")).c_str());
	this->equipmentFile[Inventory::WEAPON] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_WEAPON.STB")).c_str());
	std::cout << "Finished reading all STBs!\r\n";

	
	return true;
}
bool WorldServer::loadNPCData() {
	for (unsigned int i = 0; i < this->npcFile->getRowCount(); i++) {
		NPCData newData(this->npcFile, i);
		this->npcData.push_back(newData);
	}
	return true;
}

bool WorldServer::loadMapData() {
	std::cout << "Loading MapData: 0%\r";
	this->mapData.reserve(this->zoneFile->getRowCount());
	for(unsigned int i=0;i<this->mapData.capacity();i++) {
		this->mapData.addValue(new Map());
		Map* curMap = this->mapData[i];
		curMap->setId(i);
		std::string mapPath = this->zoneFile->getZoneFile(i);

		//Every valid map has the basic path "3DDATA\"
		if(mapPath.find("3DDATA") == -1)
			mapPath = std::string("");
		else
			mapPath = mapPath.substr(0, mapPath.find_last_of("\\") == - 1 ? mapPath.length() : mapPath.find_last_of("\\")+1);
		curMap->setMapPath( mapPath.c_str() );
		//Set the entire day cycle (length, morning, day, evening, night)
		curMap->setDayCycle(this->zoneFile->getDayLength(i), this->zoneFile->getMorningTime(i), this->zoneFile->getNoonTime(i), this->zoneFile->getEveningTime(i), this->zoneFile->getNight(i));
		std::cout << "Loading MapData: " << i * 100.0f / this->mapData.capacity() << "%\r";
	}
	std::cout << "Finished loading MapData           \n";
	return true;
}

bool WorldServer::loadZones() {
	this->zoneData.reserve(this->mapData.size());
	std::string folderPath = "";
	std::vector<std::string> files;
	for(unsigned int i=0;i<this->mapData.size();i++) {
		Map* map = this->mapData.getValue(i);
		folderPath = map->getMapPath();
		if (folderPath.length() == 0) {
			this->zoneData.addValue(new ZON("", i));
			continue;
		}
		folderPath = workingPath + std::string("\\") + folderPath.substr(0, folderPath.find_last_of("\\") + 1);
		QuickInfo::getFilesFromDirectory(folderPath, std::string(".zon"), files);
		if (files.size() == 0) {
			//In case there are none (for whatever reason), don't do anything
			this->zoneData.addValue(new ZON("", i));
			continue;
		}
		this->zoneData.addValue(new ZON(files.at(0x00).c_str(), i));
		files.clear();
	}
	return true;
}

bool WorldServer::loadIFOs(Map* curMap) {
	std::string folderPath = std::string(curMap->getMapPath());
	if (folderPath.length() == 0)
		return true;
	folderPath = workingPath + std::string("\\") + folderPath.substr(0, folderPath.find_last_of("\\") + 1);
	std::vector<std::string> files;

	//Get all IFOs from the current map directory
	QuickInfo::getFilesFromDirectory(folderPath, std::string(".ifo"), files);
	if (files.size() == 0) {
		//In case there are none (for whatever reason), don't do anything
		return true;
	}
	STBFile warpFile((workingPath + std::string("\\3DDATA\\STB\\WARP.STB")).c_str());
	this->teleGates.reserve(warpFile.getRowCount());
	for(unsigned int i=0;i<this->teleGates.capacity();i++)
		this->teleGates.addValue(Telegate());

	/** ASSIGN SECTORS **/
	curMap->createSectors(files);

	//Iterate through all *.ifo files
	for (unsigned int j = 0; j < files.size(); j++) {
		IFO ifo(files.at(j).c_str());

		//Add all spawns of the current *.ifo to the map
		for (unsigned int k = 0; k < ifo.getSpawnAmount(); k++) {
			curMap->addSpawn( new IFOSpawn(ifo.getSpawn(k)) );
		}
		//Add all (IFO-)NPCs to the map
		for (unsigned int k = 0; k < ifo.getNPCAmount(); k++) {
			IFONPC& npc = ifo.getNPC(k);
			NPCData& npcData = this->npcData.at(npc.getObjectId());
			NPC *newNpc = new NPC(&npcData, &this->aiData.getValue(npcData.getAIId()), curMap->getId(), npc.getPosition());
			newNpc->setSector(curMap->getSector(newNpc->getPositionCurrent()));
		}
		//Create telegates from the previously read IFO-info
		this->loadTelegates(curMap->getId(), warpFile, ifo);
	}
	return true;
}

bool WorldServer::loadTelegates(const WORD currentMapId, STBFile& warpFile, IFO& ifoFile) {
	for (unsigned int i = 0; i < ifoFile.getTelegateAmount(); i++) {
		IFOTelegate& gate = ifoFile.getTelegate(i);
		std::string& gateName = warpFile.getRow(gate.getUnknown()).getColumn(0x02);

		WORD destMapId = warpFile.getRow(gate.getUnknown()).getColumn<WORD>(0x01);
		ZON* zone = this->zoneData.getValue(destMapId);
		ZON::EventInfo zonGate;
		try {
			//Try to find the suiting gate info (e.g. 2-way gate)
			//If found, create a proper gate
			zonGate = zone->findEventInfo(gateName);
		}
		catch (std::exception) {
			//In case there is no 2-way gate (e.g. Event-/GM-Maps)
			//Send the player to the "start"-point of the destinated map
			zonGate = zone->findEventInfo(std::string("start"));
			std::cout << "Couldn't find a suiting telegate for ZoneGate " << gateName.c_str() << "!\nSending it to the \"start\" position of Map " << warpFile.getRow(gate.getUnknown()).getColumn<WORD>(0x01) << "\n";
		}
		Telegate& telegate = this->teleGates.getValue(gate.getUnknown());
		telegate.init(gate.getPosition(), currentMapId, Position(zonGate.x, zonGate.y), destMapId);
	}
	return true;
}

bool WorldServer::loadAI() {
	//Reserve an array with the size of the AIFile's row count
	this->aiData.reserve(this->aiFile->getRowCount());
	for (unsigned int i = 0; i < this->aiFile->getRowCount(); i++) {
		//Read all AI-Files and store them in our array.
		AIP aiFile( (workingPath + std::string("\\") + this->aiFile->getFilePath(i)).c_str() );
		this->aiData.addValue(aiFile);
	}
	return true;
}

WORD WorldServer::assignClientID(Entity* newEntity) {
	if(!newEntity)																																																																																																																																
		return 0xFFFF;
	for(unsigned int i=0;i<0x10000;i++) {
		Entity*& entity = this->clientIDs[i].second;
		if(!entity) {
			this->clientIDs[i].second = newEntity;
			newEntity->setClientId(i);
			return i;
		}
	}
	return 0xFFFF;
}

void WorldServer::freeClientId(Entity* toDelete) {
	this->clientIDs[toDelete->getClientId()].second = nullptr;
	toDelete->setClientId(0xFFFF);
}


void WorldServer::convertTo(NPC* npc, WORD npcDataId) {
	
}


DWORD WorldServer::buildItemVisually(const Item& item) {
	DWORD basicResult = (item.id | item.refine * 0x10000);
	if (item.gem == 0) {
		return basicResult;
	}
	return ((0xd0000) + ((item.gem - 320) * 0x400) | basicResult);
}

WORD WorldServer::buildItemHead(const Item& item) {
	if (item.amount == 0x00)
		return 0;
	return static_cast<WORD>(((item.id & 0x7FFF) << 5) | (item.type & 0x1F));
}

DWORD WorldServer::buildItemData(const Item& item) {
	if ((item.type >= 10 && item.type <= 13) || item.amount == 0) {
		return item.amount;
	}
	DWORD refinePart = (item.refine >> 4) << 28;
	DWORD appraisePart = item.isAppraised << 27;
	DWORD socketPart = item.isSocketed << 26;
	DWORD lifeSpanPart = (item.lifespan * 10) << 16;
	DWORD durabilityPart = item.durability << 9;
	DWORD stats = item.stats;
	DWORD gem = item.gem;
	if (gem != 0x00)
		stats = 0x00;

	return (refinePart | appraisePart | socketPart | lifeSpanPart | durabilityPart | stats | gem);
}

bool ChatService::sendMessage(Entity* sender, const char* msg) {
	Packet pak(PacketID::World::Response::LOCAL_CHAT);
	pak.addWord(sender->getClientId());
	pak.addString(msg);
	pak.addByte(0x00);
	if (msg[0] == '/') {
		GMService::executeCommand(dynamic_cast<Player*>(sender), pak);
		return true;
	}
	return sender->sendToVisible(pak);
}

bool ChatService::sendShout(Entity* entity, const char* msg) {
	Packet pak(PacketID::World::Response::SHOUT_CHAT);
	pak.addString(entity->getName().c_str());
	pak.addByte(0x00);
	pak.addString( msg );
	pak.addByte(0x00);
	return entity->sendToMap(pak);
}


void GMService::executeCommand(Player* gm, Packet& chatCommand) {
	std::string msg = std::string(chatCommand.getString(0x03)); //ClientID + char '/'
	if (msg.length() == 0)
		return;
	std::string command = msg.substr(0, msg.find(" "));
	std::string curValue = msg.substr(0, (msg.find(" ") == -1 ? msg.length() : msg.find(" ")));

#define WANTED_COMMAND(c2) (_stricmp(command.c_str(), c2)==0)
#define SPLIT() msg = msg.substr(msg.find(" ")+1); curValue = msg.substr(0, (msg.find(" ") == -1 ? msg.length() : msg.find(" ")));

	SPLIT();
	if (WANTED_COMMAND("mon")) {
		DWORD monType = atoi(curValue.c_str());
		SPLIT();
		DWORD amount = atoi(curValue.c_str());
		//TODO
	}
	else if (WANTED_COMMAND("tele")) {
		WORD mapId = atoi(curValue.c_str()); SPLIT();
		if (mapId == 0 || mapId >= mainServer->mapData.size())
			return;
		float coordX = atoi(curValue.c_str()) * 100.0f; SPLIT();
		float coordY = atoi(curValue.c_str()) * 100.0f;

		gm->pakTelegate(mapId, Position(coordX, coordY));
	}
#undef STRCMP
#undef SPLIT
}

void WorldServer::dumpTelegates(const char* filePath) {
	CMyFile file(filePath, "a+");
	for(unsigned int i=0;i<this->teleGates.size();i++) {
		Telegate& gate = this->teleGates.getValue(i);
		file.putStringWithVarOnly("[Gate %i]: From[%i;%f,%f] - To[%i;%f,%f]\n", i, gate.getSourceMap(), gate.getSourcePosition().x, gate.getSourcePosition().y, gate.getDestMap(), gate.getDestPosition().x, gate.getDestPosition().y);
	}
}
