#include "WorldServer.h"
#include "Entity\Player.h"
#include "Entity\Drop.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"

#include "..\ConfigReader\Config.h"

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

	for (unsigned int i = 1; i <= 14; i++) {
		delete this->equipmentFile[i];
		this->equipmentFile[i] = nullptr;
	}

	delete this->aiFile;
	this->aiFile = nullptr;
	
	delete this->skillFile;
	this->skillFile = nullptr;

	delete this->npcFile;
	this->npcFile = nullptr;

	delete ::config;
	::config = nullptr;

	delete this->zoneFile;
	this->zoneFile = nullptr;

	delete this->dropFile;
	this->dropFile = nullptr;

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
	NPC* curNPC = nullptr;

	LinkedList<Entity*> entityToRemove;
	for(unsigned int i=0;i<curMap->getSectorCount();i++) {
		MapSector* curSector = curMap->getSector(i);
		LinkedList<Entity*>::Node* entityNode = curSector->getFirstEntity();
		for(;entityNode;entityNode = entityNode->getNextNode()) {
			Entity* curEntity = entityNode->getValue();
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
					curPlayer->checkRegeneration();
 					if(curEntity->checkForNewSector() != nullptr) {
						entityToRemove.add(curEntity);
					}
				break;
				case Entity::TYPE_NPC:
				case Entity::TYPE_MONSTER:
					curNPC = dynamic_cast<NPC*>(curEntity);
					if (isIdling) {
						//In case the NPC/Monster is idling
						//run AI with state "IDLE"
						if(curEntity->getTarget() == NULL) {
							AIService::run(curNPC, AIP::ON_IDLE);
						} else {
							AIService::run(curNPC, AIP::ON_ATTACK);
						}
					} else {
						//In case it's walking and has no target
						//i.e. walking around/away
						if(curEntity->getTarget() == NULL)
							curNPC->setTimeAICheck();
						else
							AIService::run(curNPC, AIP::ON_ATTACK);
					}
				break;
			}
			//In case we're idling and have a target --> attack!
			if(isIdling && curEntity->getTarget() != nullptr) {
				if(!curEntity->isAllied(curEntity->getTarget())) {
					curEntity->attackRoutine();
				}
			}
		}
		while(entityToRemove.getNodeCount()>0) {
			entityNode = entityToRemove.getHeadNode();
			Entity* curEntity = entityNode->getValue();
			curEntity->setSector( curMap->getSector( curEntity->getPositionCurrent() ) );
			curEntity->checkVisuality();

			entityNode = entityNode->getNextNode();
			entityToRemove.removeAt(0x00);
		}
	}
	this->checkSpawns(curMap);
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
	this->dropFile = new STBFile((workingPath + std::string("\\3DDATA\\STB\\ITEM_DROP.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->zoneFile = new ZoneSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_ZONE.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::HEADGEAR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_CAP.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::ARMOR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BODY.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::GLOVES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_ARMS.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::BACK] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BACK.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::FACE] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FACEITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::SHOES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FOOT.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::JEWELS] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEMITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::JEWELRY] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEWEL.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::OTHER] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_NATURAL.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::PAT] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_PAT.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::SHIELD] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_SUBWPN.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::CONSUMABLES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_USEITEM.STB")).c_str());
	totalPercent += percentDone;
	std::cout << totalPercent << "% done reading\r";

	this->equipmentFile[ItemType::QUEST] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_QUESTITEM.STB")).c_str());
	this->equipmentFile[ItemType::WEAPON] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_WEAPON.STB")).c_str());
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
	curMap->setSectorWidthAndHeight(this->zoneFile->getZoneSize(curMap->getId()));
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
			IFONPC& npcINFO = ifo.getNPC(k);
			NPCData& npcData = this->npcData.at(npcINFO.getObjectId());
			NPC *newNpc = new NPC(&npcData, &this->aiData.getValue(npcData.getAIId()), curMap->getId(), npcINFO.getPosition());
			newNpc->setDirection( npcINFO.getDirection() );
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
	for (WORD i = 0; i < this->aiFile->getRowCount(); i++) {
		//Read all AI-Files and store them in our array.
		AIP aiFile( i, (workingPath + std::string("\\") + this->aiFile->getFilePath(i)).c_str() );
		this->aiData.addValue(aiFile);
	}
	return true;
}

WORD WorldServer::assignClientID(Entity* newEntity) {
	if(!newEntity)																																																																																																																																
		return 0;

	//ClientID 0 = invalid state.
	for(unsigned int i=1;i<0x10000;i++) {
		Entity*& entity = this->clientIDs[i].second;
		if(!entity) {
			this->clientIDs[i].second = newEntity;
			newEntity->setClientId(i);
			return i;
		}
	}
	return 0;
}

void WorldServer::freeClientId(Entity* toDelete) {
	this->clientIDs[toDelete->getClientId()].second = nullptr;
	toDelete->setClientId(0x00);
}


void WorldServer::convertTo(NPC* npc, WORD npcDataId) {
	
}


bool WorldServer::isValidItem(const WORD itemType, const WORD itemId) {
	if(itemType == 0 || itemType > ItemType::PAT || itemId >= this->equipmentFile[itemType]->getRowCount())
		return false;

	//I don't know what that column means, but it is a legit indicator
	//("Reversed" via STB<->STL entry comparison)
	return (this->equipmentFile[itemType]->getRow(itemId).getColumnAsInt(0x09) > 0);
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
	WORD result = static_cast<WORD>((item.id << 5) & 0xFFE0);
	return static_cast<WORD>(result | (item.type & 0x1F));
}

DWORD WorldServer::buildItemData(const Item& item) {
	if ((item.type >= ItemType::CONSUMABLES && item.type <= ItemType::QUEST) || item.type == ItemType::MONEY || item.amount == 0) {
		return item.amount;
	}

	//0101 1111 1001 0000
	DWORD refinePart = (item.refine >> 4) << 28;
	DWORD appraisePart = item.isAppraised << 27;
	DWORD socketPart = item.isSocketed << 26;
	DWORD lifeSpanPart = item.lifespan << 16;
	DWORD durabilityPart = item.durability << 9;
	DWORD stats = item.stats;
	DWORD gem = item.gem;
	if (gem != 0x00)
		stats = 0x00;

	return (refinePart | appraisePart | socketPart | lifeSpanPart | durabilityPart | stats | gem);
}

const WORD WorldServer::getQuality(const BYTE itemType, const DWORD itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry(itemType, itemId);
		WORD result = entry.getColumn<WORD>(0x08);
		return result;
	} catch(std::exception& e) {
		std::cout << e.what() << "\n";
	}
	return 0x00;
}

const WORD WorldServer::getSubType(const BYTE itemType, const DWORD itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( itemType, itemId );
		WORD atkPower = entry.getColumn<WORD>(0x04);
		return atkPower;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

const WORD WorldServer::getWeaponAttackpower(const DWORD itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( ItemType::WEAPON, itemId );
		WORD atkPower = entry.getColumn<WORD>(0x23);
		return atkPower;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

const int WorldServer::getWeaponAttackspeed(const DWORD itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( ItemType::WEAPON, itemId );
		int atkSpeed = entry.getColumn<int>(0x24);
		return atkSpeed;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

STBEntry& WorldServer::getEquipmentEntry(const BYTE itemType, const DWORD itemId) {
	if(itemType == 0x00 || itemType > ItemType::PAT)
		throw TraceableExceptionARGS("Invalid ItemType: %i", itemType);
	STBFile *eqFile = this->equipmentFile[itemType];
	if(itemId >= eqFile->getRowCount())
		throw TraceableExceptionARGS("Invalid Item [%i, %i]!", itemType, itemId);
	return eqFile->getRow(itemId);
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

bool ChatService::sendWhisper(Player* from, Player* to, const char *msg) {
	if(from == nullptr)
		return false;
	Packet pak(PacketID::World::Response::WHISPER_CHAT);
	pak.addString(from->getName()); pak.addByte(0x00);
	if(!to) {
		pak.addByte(0x00);
		return from->sendData(pak);
	}
	pak.addString(msg); 
	pak.addByte(0x00);
	return to->sendData(pak);
}


bool ChatService::sendWhisper(const char* from, Player* to, const char *aMsg, ...) {
	if(to == nullptr)
		return false;
	ArgConverterA(msg, aMsg);
	Packet pak(PacketID::World::Response::WHISPER_CHAT);
	pak.addString(from); pak.addByte(0x00);
	pak.addString(msg); pak.addByte(0x00);
	return to->sendData(pak);
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
#define SPLIT() \
	msg = (msg.find(" ") == -1 ? "" : msg.substr(msg.find(" ")+1)); \
	curValue = msg.length() > 0 ? msg.substr(0, msg.find(" ")) : msg;

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
	} else if(WANTED_COMMAND("stats")) {
		ChatService::sendWhisper("Server", gm, "HP: %i/%i\n", gm->getCurrentHP(), gm->getMaxHP());
		ChatService::sendWhisper("Server", gm, "AtkPower: %i\n", gm->getAttackPower());
		ChatService::sendWhisper("Server", gm, "Defense: %i\n", gm->getDefensePhysical());
		ChatService::sendWhisper("Server", gm, "Hitrate: %i\n", gm->getHitrate());
		ChatService::sendWhisper("Server", gm, "AttackSpeed: %i\n", gm->getAttackSpeed());
		ChatService::sendWhisper("Server", gm, "AttackRange: %f\n", gm->getAttackRange());
	}
	else if(WANTED_COMMAND("heal")) {
		gm->setCurrentHP(gm->getMaxHP());
		gm->setCurrentMP(gm->getMaxMP());
		gm->pakUpdateLifeStats();
	} else if(WANTED_COMMAND("equip")) {
		WORD itemType = atoi(curValue.c_str()); 
		if(itemType == 0 && curValue.length() > 0) {
			curValue = msg;
			for(unsigned int k=ItemType::FACE;k<=ItemType::SHIELD;k++) {
				STBFile* file = mainServer->getEquipmentSTB(k);
				for(unsigned int m=0;m<file->getRowCount();m++) {
					STBEntry& entry = file->getRow(m);
					std::string name = entry.getColumn(0x00);
					if(name.find(curValue.c_str()) != -1) {
						Item item;
						item.amount = 1; item.durability = 120;
						item.id = m; item.type = k;
						item.lifespan = 1000;
						item.refine = 0;

						//In case the wanted item is valid (should always apply)
						if(mainServer->isValidItem(k, m))
							gm->equipItem(item);

						return;
					}
				}
			}
		}
		SPLIT();
	}
	else if(WANTED_COMMAND("drop")) {
		unsigned long itemType = static_cast<unsigned long>(atoi(curValue.c_str())); SPLIT();
		if(curValue.length() == 0) {
			//Drop money
			new Drop(gm, itemType, false);
		} else {
			//Drop item
			WORD itemNum = atoi(curValue.c_str()); SPLIT();
			if(itemNum == 0)
				return;
			WORD amount = curValue.length() == 0 ? 0x01 : atoi(curValue.c_str());

			Item item; 
			item.type = static_cast<BYTE>(itemType);
			item.id = itemNum;
			item.amount = amount;
			item.durability = 120;
			item.isAppraised = true;
			item.lifespan = 1000;
			if(mainServer->isValidItem(item.type, item.id))
				new Drop(gm, item, false);
		}
	}
#undef STRCMP
#undef SPLIT
}


NPCData* WorldServer::getNPCData(const AIP* ai) {
	for(unsigned int i=0;i<this->npcData.size();i++) {
		NPCData* npcData = &this->npcData.at(i);
		if(npcData->getAIId() == ai->getId())
			return npcData;
	}
	return nullptr;
}

void WorldServer::dumpAICombined(const char* filePath) {
	CMyFile file(filePath, "a+");
	for(unsigned int i=0;i<this->aiData.size();i++) {
		AIP* aip = &this->aiData.getValue(i);
		if(aip->getCheckInterval() == 0x00)
			continue;
		file.putStringWithVarOnly("%s [%i]\n", aip->getFilePath().c_str(), i);
		file.putStringWithVarOnly("CheckInterval: %i | DmgTrigger: %i\n", aip->getCheckInterval(), aip->getTriggerDamageAmount());
		file.putStringWithVarOnly("BlockCount: %i\n", aip->getBlockCount());
		for(unsigned int j=0;j<aip->getBlockCount();j++) {
			const std::vector<AIP::Record> records = aip->getRecords(i);
			file.putStringWithVarOnly("\tCurrent Block[%i] records: %i\n", j, records.size());
			for(unsigned int k=0;k<records.size();k++) {
				const AIP::Record& curRec = records.at(k);
				file.putStringWithVarOnly("\t\tConditionCount: %i\n", j, curRec.getConditionCount());
				for(unsigned int m=0;m<curRec.getConditionCount();m++) {
					const char* data = curRec.getCondition(i).getData();
					AIConditions cond(data);
					file.putStringWithVarOnly("\t\t\t%s\n", cond.toString());
				}
				file.putStringWithVarOnly("\t\tActionCount: %i\n", j, curRec.getActionCount());
				for(unsigned int m=0;m<curRec.getActionCount();m++) {
					const char* data = curRec.getAction(i).getData();
					AIActions act(data);
					file.putStringWithVarOnly("\t\t\t%s\n", act.toString());
				}
			}
		}
		file.putString("\n\n");
	}
}


void WorldServer::dumpAISeparated(std::string basicFilePath) {
	for(unsigned int i=0;i<this->aiData.size();i++) {
		AIP* aip = &this->aiData.getValue(i);
		NPCData* npcData = this->getNPCData(aip);
		if(!npcData)
			continue;
		CMyFile file( (basicFilePath + "\\" + npcData->getName() + ".log").c_str(), "a+");
		if(aip->getCheckInterval() == 0x00)
			continue;
		file.putStringWithVarOnly("%s [%i]:\n", aip->getFilePath().c_str(), i);
		file.putStringWithVarOnly("CheckInterval: %i | DmgTrigger: %i\n", aip->getCheckInterval(), aip->getTriggerDamageAmount());
		file.putStringWithVarOnly("BlockCount: %i\n", aip->getBlockCount());
		for(unsigned int j=0;j<aip->getBlockCount();j++) {
			const std::vector<AIP::Record> records = aip->getRecords(i);
			file.putStringWithVarOnly("\tCurrent Block[%i] records: %i\n", j, records.size());
			for(unsigned int k=0;k<records.size();k++) {
				const AIP::Record& curRec = records.at(k);
				file.putStringWithVarOnly("\t\tConditionCount: %i\n", j, curRec.getConditionCount());
				for(unsigned int m=0;m<curRec.getConditionCount();m++) {
					const char* data = curRec.getCondition(i).getData();
					AIConditions cond(data);
					file.putStringWithVarOnly("\t\t\t%s\n", cond.toString());
				}
				file.putStringWithVarOnly("\t\tActionCount: %i\n", j, curRec.getActionCount());
				for(unsigned int m=0;m<curRec.getActionCount();m++) {
					const char* data = curRec.getAction(i).getData();
					AIActions act(data);
					file.putStringWithVarOnly("\t\t\t%s\n", act.toString());
				}
			}
		}
		file.putString("\n\n");
	}
}

void WorldServer::dumpTelegates(const char* filePath) {
	CMyFile file(filePath, "a+");
	for(unsigned int i=0;i<this->teleGates.size();i++) {
		Telegate& gate = this->teleGates.getValue(i);
		if(gate.getSourceMap() == std::numeric_limits<WORD>::max() ||
			gate.getDestMap() == std::numeric_limits<WORD>::max())
			continue;
		file.putStringWithVarOnly("[Gate %i]: From[%i;%f,%f] - To[%i;%f,%f]\n", i, gate.getSourceMap(), gate.getSourcePosition().x, gate.getSourcePosition().y, gate.getDestMap(), gate.getDestPosition().x, gate.getDestPosition().y);
	}
}

	