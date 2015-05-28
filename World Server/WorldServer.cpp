#include "WorldServer.h"
#include "Entity\Player.h"
#include "Entity\Drop.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"
#include "FileTypes\VFS.h"
#include "FileTypes\ZMO.h"

#include "D:\Programmieren\ConfigReader\Config.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

WorldServer* mainServer; 
std::map<DWORD, QuestEntry*> questData;
#ifdef __ROSE_MULTI_THREADED__
std::mutex sectorMutex;
#endif

WorldServer::WorldServer(word_t port, MYSQL* mysql) {
	this->port = port;
	this->sqlDataBase.init(__DATABASE_HOST__, __DATABASE_USERNAME__, __DATABASE_PASSWORD__, __DATABASE_DBNAME__, __DATABASE_PORT__, mysql);
	mainServer = this;

	this->ipString = config->getValueString("WorldIp");
	
	this->WorldTime.lastCheck = time(NULL);
	this->WorldTime.currentTime = 0x00;
#ifdef __ROSE_MULTI_THREADED__
	GlobalLogger::info("Starting up as multi threaded world server.\n");
#endif
#ifdef __ROSE_USE_VFS__
	this->vfs = new VFS(config->getValueString("GameFolder"));
	STBFile warpFile(this->vfs, "3DDATA\\STB\\WARP.STB");
#else
	STBFile warpFile((workingPath + std::string("\\3DDATA\\STB\\WARP.STB")).c_str());
#endif

	this->loadSTBs();
	this->loadNPCData();
	this->loadNPCAnimations();
	this->loadMapData();
	this->loadAI();
	this->loadZones();
	this->loadQuests();
	this->loadAttackTimings();
	
	float num = 0.0f;

	for(unsigned int i=0;i<this->mapData.size();i++) {
		std::cout << "Loading Map Infos: " << num << "%        \r";
		this->loadIFOs(this->mapData[i], warpFile);
		this->mapData[i]->checkSpawns();
		num = i * 100.0f / static_cast<float>(this->mapData.size());
	}
	std::cout << "Finished loading all map information!\n";
}

WorldServer::~WorldServer() {
	this->sqlDataBase.disconnect();

	closesocket(this->charServer.socket);
	this->charServer.socket = SOCKET_ERROR;

#ifdef __ROSE_MULTI_THREADED__
	GlobalLogger::info("Closing all map threads...\n");
	for (unsigned int i = 0; i < this->mapThreads.capacity(); i++) {
		this->mapThreads[i]->join();
	}
	this->mapThreads.clear();
#endif

	delete this->vfs;
	this->vfs = nullptr;

	for (unsigned int i = ItemType::FACE; i <= ItemType::PAT; i++) {
		delete this->equipmentFile[i];
		this->equipmentFile[i] = nullptr;
	}

	delete this->statusFile;
	this->statusFile = nullptr;

	delete this->questFile;
	this->questFile = nullptr;

	delete this->npcAnimationInfos;
	this->npcAnimationInfos = nullptr;

	delete this->aiFile;
	this->aiFile = nullptr;
	
	delete this->skillFile;
	this->skillFile = nullptr;

	delete this->npcFile;
	this->npcFile = nullptr;

	delete this->zoneFile;
	this->zoneFile = nullptr;

	delete this->dropFile;
	this->dropFile = nullptr;

	std::map<const DWORD, QuestEntry*>::iterator questIterator = this->questData.begin();
	for (; questIterator != this->questData.end(); questIterator++) {
		std::pair<const DWORD, QuestEntry*> data = *questIterator;
		delete data.second;
		data.second = nullptr;
	}
	this->questData.clear();

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

ClientSocket* WorldServer::createClient(SOCKET sock) {
	Player* newPlayer = new Player(sock, this);
	return newPlayer;
}

bool WorldServer::checkForServer(ClientSocket* client, std::string& ip) {
	if (this->charServer.clientHandle == nullptr && _stricmp(::config->getValueString("ChannelIp"), ip.c_str()) == 0) {
		this->charServer.clientHandle = client;
		std::cout << "CharServer fully connected: " << ip.c_str() << "\n";
		return true;
	}
	return false;
}

//TODO: Implement a message stre
bool WorldServer::notifyCharServer(const word_t packetId, const dword_t optionalId) {
	try {
		throw TraceableException("notifyCharServer(%i, %i) is not implemented yet!", packetId, optionalId);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
		return false;
	}
	return true;
}

void WorldServer::registerServer() {
	this->charServer.port = ::config->getValueInt("ChannelPort");
	this->charServer.ip = std::string(::config->getValueString("ChannelIp"));

	sockaddr_in ain;
	ain.sin_family = AF_INET;
	ain.sin_addr.S_un.S_addr = ::inet_addr(this->charServer.ip.c_str());
	ain.sin_port = ::htons(this->charServer.port);

	this->charServer.socket = ::socket(AF_INET, SOCK_STREAM, 0x00);
	if (this->charServer.socket == SOCKET_ERROR)
		throw TraceableException("Socket couldn't be established.\nReason: %s\n", QuickInfo::getErrorDescription(WSAGetLastError()));
	while (::connect(this->charServer.socket, (const sockaddr*)&ain, sizeof(sockaddr)) != 0) {
		Sleep(1);
	}
}

void WorldServer::setupBeforeMainLoop() {
#ifdef __ROSE_MULTI_THREADED__
	mapThreads.reserve(this->mapData.size());
	for (dword_t i = 0; i < this->mapData.capacity(); i++) {
		this->mapThreads.addValue(new std::thread(&(::runMap), this->mapData[i]));
	}
#endif
}

void WorldServer::executeRequests() {
	this->checkWorldTime();
#ifndef __ROSE_MULTI_THREADED__
	for(unsigned int i=0;i<this->mapData.size();i++) {
		Map* curMap = this->mapData[i];
		this->runMap(curMap);
		curMap->checkSpawns();
	}
#endif
}

void WorldServer::onClientDisconnect(ClientSocket* client) {
#ifdef __ROSE_MULTI_THREADED__
	Player* player = dynamic_cast<Player*>(client);
	sectorMutex.lock();
	player->setSector(nullptr);
	sectorMutex.unlock();
#endif
}

void WorldServer::checkWorldTime() {
	if((time(NULL) - this->WorldTime.lastCheck) < WorldServer::worldTime::DEFAULT_WORLDTIME_CHECK)
		return;
	this->WorldTime.currentTime++;
	this->WorldTime.lastCheck = time(NULL);
}

void WorldServer::changeToMap(Entity* entity, const word_t newMapId) {
	if(!entity || newMapId >= this->mapData.size())
		return;
	Map* oldMap = this->getMap(entity->getMapId());
#ifdef __ROSE_MULTI_THREADED__
	sectorMutex.lock();
#endif
	oldMap->freeLocalId(entity);

	//Assign the new mapId and the nearest sector on the new map
	entity->setMapId(newMapId);
	Map* newMap = this->getMap(entity->getMapId());
	entity->setSector(newMap->getSector(entity->getPositionCurrent()));
	newMap->assignLocalId(entity);
#ifdef __ROSE_MULTI_THREADED__
	sectorMutex.unlock();
#endif
}

#ifndef __ROSE_MULTI_THREADED__
void WorldServer::runMap(Map* curMap) {
	if (!curMap)
		return;
#else
void runMap(Map* curMap) {
	if (!curMap)
		return;
	do {
		Sleep(10);
		if (!curMap->hasActivePlayers()) {
			continue;
		}
#endif
	Player* curPlayer = nullptr;
	NPC* curNPC = nullptr;
	Map::Sector* newSector = nullptr;

	LinkedList<Entity*> entityToRemove;
	for(unsigned int i=0;i<curMap->getSectorCount();i++) {
		Map::Sector* curSector = curMap->getSector(i);
		LinkedList<Entity*>::Node* entityNode = curSector->getFirstEntity();
		bool updateNextNode = false;
		while(entityNode) {
			if(updateNextNode) {
				entityNode = entityNode->getNextNode();
				if(!entityNode)
					break;
			} 
			updateNextNode = true;
			Entity* curEntity = entityNode->getValue();
			if (!curEntity || !curEntity->isIngame() || curEntity->getEntityType() == Entity::TYPE_DROP) {
				continue;
			}
			byte_t result = curEntity->movementRoutine();
			if (result & Entity::Movement::INITIAL_ATTACK) {
				curEntity->getAttackAnimation();
			}
			if (result & Entity::Movement::TARGET_REACHED) {
				bool isProcFrameReached = curEntity->playAnimation();
			}
			if ((newSector = curEntity->checkForNewSector()) != nullptr) {
				sectorMutex.lock();
				entityNode = curEntity->setSector(newSector);
				sectorMutex.unlock();
				curEntity->checkVisuality();
				if (entityNode) {//in case we have a valid succeeding node
					updateNextNode = false;
				}
			}
			switch (curEntity->getEntityType()) {
				case Entity::TYPE_PLAYER:
					curPlayer = dynamic_cast<Player*>(curEntity);
					curPlayer->checkRegeneration();
				break;
				case Entity::TYPE_NPC:
				case Entity::TYPE_MONSTER:
					curNPC = dynamic_cast<NPC*>(curEntity);
					if (result == Entity::Movement::IDLE) {
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
		}
	}
#ifdef __ROSE_MULTI_THREADED__
	} while (true);
#endif
	
}

bool WorldServer::loadSTBs() {

	std::cout << "Reading all STBs...\r";
#ifdef __ROSE_USE_VFS__	
	VFSData fileBuf;
	this->aiFile = new AISTB(this->vfs, "3DDATA\\STB\\FILE_AI.STB");
	this->npcFile = new NPCSTB(this->vfs, "3DDATA\\STB\\LIST_NPC.STB");
	this->skillFile = new SkillSTB(this->vfs, "3DDATA\\STB\\LIST_SKILL.STB");
	this->dropFile = new STBFile(this->vfs, "3DDATA\\STB\\ITEM_DROP.STB");
	this->zoneFile = new ZoneSTB(this->vfs, "3DDATA\\STB\\LIST_ZONE.STB");
	this->questFile = new STBFile(this->vfs, "3DDATA\\STB\\LIST_QUEST.STB", false);
	this->statusFile = new StatusSTB(this->vfs, "3DDATA\\STB\\LIST_STATUS.STB");
	this->motionFile = new STBFile(this->vfs, "3DDATA\\STB\\TYPE_MOTION.STB", false);

	this->equipmentFile[ItemType::HEADGEAR] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_CAP.STB");
	this->equipmentFile[ItemType::ARMOR] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_BODY.STB");
	this->equipmentFile[ItemType::GLOVES] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_ARMS.STB");
	this->equipmentFile[ItemType::BACK] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_BACK.STB");
	this->equipmentFile[ItemType::FACE] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_FACEITEM.STB");
	this->equipmentFile[ItemType::SHOES] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_FOOT.STB");
	this->equipmentFile[ItemType::JEWELS] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_JEMITEM.STB");
	this->equipmentFile[ItemType::JEWELRY] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_JEWEL.STB");
	this->equipmentFile[ItemType::OTHER] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_NATURAL.STB");
	this->equipmentFile[ItemType::PAT] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_PAT.STB");
	this->equipmentFile[ItemType::SHIELD] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_SUBWPN.STB");
	this->equipmentFile[ItemType::CONSUMABLES] = new ConsumeSTB(this->vfs, "3DDATA\\STB\\LIST_USEITEM.STB");
	this->equipmentFile[ItemType::QUEST] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_QUESTITEM.STB");
	this->equipmentFile[ItemType::WEAPON] = new STBFile(this->vfs, "3DDATA\\STB\\LIST_WEAPON.STB");

#else
	this->aiFile = new AISTB((workingPath + std::string("\\3DDATA\\STB\\FILE_AI.STB")).c_str());
	this->npcFile = new NPCSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_NPC.STB")).c_str());
	this->skillFile = new SkillSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_SKILL.STB")).c_str());
	this->dropFile = new STBFile((workingPath + std::string("\\3DDATA\\STB\\ITEM_DROP.STB")).c_str());
	this->zoneFile = new ZoneSTB((workingPath + std::string("\\3DDATA\\STB\\LIST_ZONE.STB")).c_str());
	this->equipmentFile[ItemType::HEADGEAR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_CAP.STB")).c_str());
	this->equipmentFile[ItemType::ARMOR] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BODY.STB")).c_str());
	this->equipmentFile[ItemType::GLOVES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_ARMS.STB")).c_str());
	this->equipmentFile[ItemType::BACK] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_BACK.STB")).c_str());
	this->equipmentFile[ItemType::FACE] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FACEITEM.STB")).c_str());
	this->equipmentFile[ItemType::SHOES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_FOOT.STB")).c_str());
	this->equipmentFile[ItemType::JEWELS] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEMITEM.STB")).c_str());
	this->equipmentFile[ItemType::JEWELRY] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_JEWEL.STB")).c_str());
	this->equipmentFile[ItemType::OTHER] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_NATURAL.STB")).c_str());
	this->equipmentFile[ItemType::PAT] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_PAT.STB")).c_str());
	this->equipmentFile[ItemType::SHIELD] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_SUBWPN.STB")).c_str());
	this->equipmentFile[ItemType::CONSUMABLES] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_USEITEM.STB")).c_str());
	this->equipmentFile[ItemType::QUEST] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_QUESTITEM.STB")).c_str());
	this->equipmentFile[ItemType::WEAPON] = new STBFile((workingPath + std::string("\\3DDATA\\STB\\LIST_WEAPON.STB")).c_str());
#endif
	std::cout << "Finished reading all STBs!\n";
	return true;
}

bool WorldServer::loadNPCData() {
	for (unsigned int i = 0; i < this->npcFile->getRowCount(); i++) {
		NPCData newData(this->npcFile, i);
		this->npcData.push_back(newData);
	}
	return true;
}

bool WorldServer::loadNPCAnimations() {
	this->npcAnimationInfos = new CHR(this->vfs, "3DDATA\\NPC\\LIST_NPC.CHR");
	if (this->npcAnimationInfos)
		return true;
	return false;
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
			this->zoneData.addValue(new ZON(nullptr, i));
			continue;
		}
#ifdef __ROSE_USE_VFS__
		VFSData currentZONFile;

		//Remove the trailing '\', because otherwise getting the .ZON filename is artificially high.
		//Once the name was retrieved, the removed slash is easy to reappend.
		std::string zonFileName = folderPath.substr(0, folderPath.find_last_of("\\"));
		zonFileName = zonFileName + std::string("\\") + zonFileName.substr(zonFileName.find_last_of("\\") + 1) + std::string(".zon");
		this->vfs->readFile(zonFileName.c_str(), currentZONFile);
		if (currentZONFile.data.size() == 0) {
			this->zoneData.addValue(new ZON(nullptr, i));
			continue;
		}
		this->zoneData.addValue(new ZON(&currentZONFile, i));
#else
		folderPath = workingPath + std::string("\\") + folderPath.substr(0, folderPath.find_last_of("\\") + 1);
		QuickInfo::getFilesFromDirectoryA(folderPath, std::string(".zon"), files);
		if (files.size() == 0) {
			//In case there are none (for whatever reason), don't do anything
			this->zoneData.addValue(new ZON(nullptr, i));
			continue;
		}
		this->zoneData.addValue(new ZON(files.at(0x00).c_str(), i));
#endif
		files.clear();
	}
	return true;
}

bool WorldServer::loadIFOs(Map* curMap, STBFile& warpFile) {
	std::string folderPath = std::string(curMap->getMapPath());
	if (folderPath.length() == 0)
		return true;
#ifdef __ROSE_USE_VFS__
	//dword_t fileAmount = this->vfs->getFileCountFromFolder(folderPath.c_str());
	std::vector<std::string> files = this->vfs->getFileNamesFromFolder(folderPath.c_str() , ".ifo");
#else
	folderPath = workingPath + std::string("\\") + folderPath.substr(0, folderPath.find_last_of("\\") + 1);

	//Get all IFOs from the current map directory
	QuickInfo::getFilesFromDirectoryA(folderPath, std::string(".ifo"), files);
	if (files.size() == 0) {
		//In case there are none (for whatever reason), don't do anything
		return true;
	}
#endif
 	this->teleGates.reserve(warpFile.getRowCount());
	for (unsigned int i = 0; i < this->teleGates.capacity(); i++)
		this->teleGates.addValue(Telegate());

	/** ASSIGN SECTORS **/
	curMap->setSectorWidthAndHeight(this->zoneFile->getZoneSize(curMap->getId()));

	//As precaution, as some Maps seem to have 0 as ZoneSize
	if (curMap->getSectorWidthAndHeight() == 0) {
		curMap->setSectorWidthAndHeight(IFO::CUSTOMIZED_SECTOR_SIZE);
	}
	curMap->createSectors(files);

	//Iterate through all *.ifo files
	GlobalLogger::debug("Loading IFOs from path %s...\n", folderPath.c_str());
	for (unsigned int j = 0; j < files.size(); j++) {
		VFSData fileData; this->vfs->readFile(files.at(j).c_str(), fileData);
		IFO ifo(&fileData);

		//Add all spawns of the current *.ifo to the map
		for (unsigned int k = 0; k < ifo.getSpawnAmount(); k++) {
			curMap->addSpawn(new IFOSpawn(ifo.getSpawn(k)));
		}
		//Add all (IFO-)NPCs to the map
		for (unsigned int k = 0; k < ifo.getNPCAmount(); k++) {
			IFONPC& npcINFO = ifo.getNPC(k);
			NPCData& npcData = this->npcData.at(npcINFO.getObjectId());
			NPC *newNpc = new NPC(&npcData, &this->aiData.getValue(npcData.getAIId()), curMap->getId(), npcINFO.getPosition());
			newNpc->setDirection(npcINFO.getDirection());
			newNpc->setSector(curMap->getSector(newNpc->getPositionCurrent()));
			this->globalNPCs.push_back(newNpc);
		}
		//Create telegates from the previously read IFO-info
		this->loadTelegates(curMap->getId(), warpFile, ifo);
	}
	return true;
}

bool WorldServer::loadTelegates(const word_t currentMapId, STBFile& warpFile, IFO& ifoFile) {
	for (unsigned int i = 0; i < ifoFile.getTelegateAmount(); i++) {
		IFOTelegate& gate = ifoFile.getTelegate(i);
		const std::string& gateName = warpFile.getRow(gate.getUnknown()).getColumn(0x02);

		word_t destMapId = warpFile.getRow(gate.getUnknown()).getColumn<WORD>(0x01);
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
			std::cout << "Couldn't find a suiting telegate for ZoneGate " << gateName.c_str() << "!\nSending it to the \"start\" position_t of Map " << warpFile.getRow(gate.getUnknown()).getColumn<WORD>(0x01) << "\n";
		}
		Telegate& telegate = this->teleGates.getValue(gate.getUnknown());
		telegate.init(gate.getPosition(), currentMapId, position_t(zonGate.x, zonGate.y), destMapId);
	}
	return true;
}

bool WorldServer::loadAI() {
	//Reserve an array with the size of the AIFile's row count
	this->aiData.reserve(this->aiFile->getRowCount());
	for (word_t i = 0; i < this->aiFile->getRowCount(); i++) {
		//Read all AI-Files and store them in our array.
#ifdef __ROSE_USE_VFS__
		VFSData data;
		this->vfs->readFile(this->aiFile->getFilePath(i).c_str(), data);
		AIP aiFile(i, data);
#else
		AIP aiFile(i, (workingPath + std::string("\\") + this->aiFile->getFilePath(i)).c_str());
#endif
		this->aiData.addValue(aiFile);
	}
	return true;
}

bool WorldServer::loadQuests() {
#ifdef __ROSE_USE_VFS__
	STBFile file(this->vfs, "3DDATA\\STB\\LIST_QUESTDATA.STB");
	for (unsigned int i = 0; i < file.getRowCount(); i++) {
		STBEntry& entry = file.getRow(i);
		std::string questPath = entry.getColumn(0x00);
		if (questPath.find(".qsd") == -1)
			continue;

#ifdef __ROSE_DEBUG__
		QSD* newQSD = new QSD(i, this->vfs, questPath.c_str());
		this->qsdFiles.push_back(newQSD);
		std::map<const DWORD, QuestEntry*>& questsToAdd = newQSD->getQuests();
#else
		QSD newQSD(this->vfs, questPath.c_str());

		std::map<const DWORD, QuestEntry*>& questsToAdd = newQSD.getQuests();
#endif //__ROSE_DEBUG__
		std::map<const DWORD, QuestEntry*>::iterator it = questsToAdd.begin();
		while (it != questsToAdd.end()) {
			this->questData.insert(*it);
			it++;
		}
	}
#else

#endif
	return true;
}

bool WorldServer::loadAttackTimings() {
#ifdef __ROSE_USE_VFS__
	VFSData data;
	/*
	STBFile_Template<STBEntry_INT> typeMotionSTB(this->vfs, "3DDATA\\STB\\TYPE_MOTION.STB");
	STBEntry_INT attackMotion = typeMotionSTB.getRow(0x08);

	std::vector<DWORD> attackMotionEntries;
	for (unsigned int i = 0; i < attackMotion.getColumnCount(); i++) {
		attackMotionEntries.push_back(attackMotion.getColumn<DWORD>(i));
	}*/
	STBFile fileMotionSTB(this->vfs, "3DDATA\\STB\\FILE_MOTION.STB");

	this->playerAttackAnimations.reserve(fileMotionSTB.getRowCount());
	GlobalLogger::debug("Reading %i animations...\n", fileMotionSTB.getRowCount());
	for (unsigned int j = 0; j < fileMotionSTB.getRowCount(); j++) {
		const std::string& currentMotion = fileMotionSTB.getRow(j).getColumn(1);
		this->playerAttackAnimations.addValue(ZMO(this->vfs, currentMotion.c_str()));
	}
#endif
	return true;
}

bool WorldServer::sendToAll(Packet& pak) {
	bool success = true;
	for (unsigned int i = 0; i < this->mapData.size(); i++) {
		Map* currentMap = this->mapData[i];
		for (unsigned int j = 0; j < currentMap->getSectorCount(); j++) {
			Map::Sector* sector = currentMap->getSector(j);
			LinkedList<Entity*>::Node* eNode = sector->getFirstPlayer();
			while (eNode) {
				Player* player = dynamic_cast<Player*>(eNode->getValue());
				if (player->isIngame())
					success &= player->sendData(pak);
				eNode = sector->getNextPlayer(eNode);
			}
		}
	}
	return success;
}

NPC* WorldServer::getNPCGlobal(const word_t npcId) {
	for (unsigned int i = 0;i<this->globalNPCs.size(); i++) {
		NPC* npc = this->globalNPCs[i];
		if (npc->getTypeId() == npcId)
			return npc;
	}
	return nullptr;
}

bool WorldServer::isValidItem(const byte_t itemType, const word_t itemId) {
	if(itemType == 0 || itemType > ItemType::PAT || itemId >= this->equipmentFile[itemType]->getRowCount())
		return false;

	//I don't know what that column means, but it is a legit indicator
	//("Reversed" via STB<->STL entry comparison)
	return (this->equipmentFile[itemType]->getRow(itemId).getColumnAsInt(0x09) > 0);
}

const word_t WorldServer::getQuality(const byte_t itemType, const dword_t itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry(itemType, itemId);
		word_t result = entry.getColumn<WORD>(EquipmentSTB::QUALITY);
		return result;
	} catch(std::exception& e) {
		std::cout << e.what() << "\n";
	}
	return 0x00;
}

const word_t WorldServer::getSubType(const byte_t itemType, const dword_t itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( itemType, itemId );
		word_t itemType = entry.getColumn<WORD>(0x04);
		return itemType;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

const word_t WorldServer::getWeaponAttackpower(const dword_t itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( ItemType::WEAPON, itemId );
		word_t atkPower = entry.getColumn<WORD>(EquipmentSTB::ATTACK_POWER_PHYSICAL);
		return atkPower;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

const int WorldServer::getWeaponAttackspeed(const dword_t itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry( ItemType::WEAPON, itemId );
		int atkSpeed = entry.getColumn<int>(EquipmentSTB::ATTACK_SPEED);
		return atkSpeed;
	} catch(std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

const byte_t WorldServer::getMotionTypeForWeapon(const dword_t itemId) {
	try {
		STBEntry& entry = this->getEquipmentEntry(ItemType::WEAPON, itemId);
		int motionType = entry.getColumn<int>(EquipmentSTB::MOTION_COLUMN);
		return motionType;
	} catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

STBEntry& WorldServer::getEquipmentEntry(const byte_t itemType, const dword_t itemId) {
	if(itemType == 0x00 || itemType > ItemType::PAT)
		throw TraceableException("Invalid ItemType: %i", itemType);
	STBFile *eqFile = this->equipmentFile[itemType];
	if(itemId >= eqFile->getRowCount())
		throw TraceableException("Invalid Item [%i, %i]!", itemType, itemId);
	return eqFile->getRow(itemId);
}

const word_t WorldServer::getMotionId(const word_t basicMotionId, const byte_t motionType) {
	if (basicMotionId >= this->motionFile->getRowCount()) {
		throw TraceableException("Invalid Motion: %i", basicMotionId);
	}
	STBEntry& entry = this->motionFile->getRow(basicMotionId);
	if (motionType >= entry.getColumnCount()) {
		throw TraceableException("Invalid weaponType: %i", motionType);
	}
	return entry.getColumn<word_t>(motionType);
}

QuestEntry* WorldServer::getQuestById(const dword_t questId) {
	std::map<const DWORD, QuestEntry*>::iterator i = this->questData.begin();
	while (i != this->questData.end()) {
		QuestEntry* entry = (*i).second;
#ifdef __ROSE_DEBUG__
		dword_t currentQuestId = entry->getQuestId();
		if(currentQuestId == questId) {
#else
		if (entry->getQuestId() == questId) {
#endif
			return entry;
		}
		i++;
	}
	return nullptr;
}

bool ChatService::sendDebugMessage(Player* receiver, const char* msg, ...) {
	va_list args; va_start(args, msg);
	std::string aMsg = QuickInfo::convertVarsToString(msg, args);
	va_end(args);
	Packet pak(PacketID::World::Response::LOCAL_CHAT);
	pak.addWord(receiver->getLocalId());
	pak.addString("[DEBUG] ");
	pak.addString(aMsg.c_str());
	pak.addByte(0x00);
	return receiver->sendData(pak);
}

bool ChatService::sendMessage(Entity* sender, const char* msg) {
	Packet pak(PacketID::World::Response::LOCAL_CHAT);
	pak.addWord(sender->getLocalId());
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
	va_list args; va_start(args, aMsg);
	std::string msg = QuickInfo::convertVarsToString(aMsg, args);
	va_end(args);
	Packet pak(PacketID::World::Response::WHISPER_CHAT);
	pak.addString(from); pak.addByte(0x00);
	pak.addString(msg); pak.addByte(0x00);
	return to->sendData(pak);
}

bool ChatService::sendShout(Entity* entity, const char* aMsg) {
	return ChatService::sendShout(entity, aMsg, nullptr);
}

bool ChatService::sendShout(Entity* entity, const char* aMsg, ...) {
	va_list args; va_start(args, aMsg);
	std::string msg = QuickInfo::convertVarsToString(aMsg, args);
	va_end(args);
	Packet pak(PacketID::World::Response::SHOUT_CHAT);
	pak.addString(entity->getName().c_str());
	pak.addByte(0x00);
	pak.addString( msg.c_str() );
	pak.addByte(0x00);
	return entity->sendToMap(pak);
}

bool ChatService::sendAnnouncement(Entity* entity, const char* aMsg, ...) {
	va_list args; va_start(args, aMsg);
	std::string msg = QuickInfo::convertVarsToString(aMsg, args);
	va_end(args);
	Packet pak(PacketID::World::Response::ANNOUNCEMENT);
	pak.addString(msg.c_str());
	pak.addByte(0x00);
	return mainServer->sendToAll(pak);
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
		dword_t monType = atoi(curValue.c_str());
		SPLIT();
		dword_t amount = atoi(curValue.c_str());
		for (unsigned int i = 0; i < amount; i++) {
			new Monster(mainServer->getNPCData(monType), mainServer->getAIData(monType), gm->getMapId(), gm->getPositionCurrent().calcNewPositionWithinRadius(amount * 20.0f));
		}
		ChatService::sendWhisper("Server", gm, "%i %s's were spawned.\n", amount, mainServer->getNPCData(monType)->getName().c_str());
	}
	else if (WANTED_COMMAND("debugmsg")) {
		if (curValue.length() == 0 || _stricmp(curValue.c_str(), "off")==0) {
			gm->setDebugMode(false);
			ChatService::sendWhisper("Server", gm, "Debug-Mode was disabled.\n");
		} 
		if (_stricmp(curValue.c_str(), "on") == 0) {
			gm->setDebugMode(true);
			ChatService::sendWhisper("Server", gm, "Debug-Mode was enabled.\n");
		}
	}
	else if (WANTED_COMMAND("where")) {
		ChatService::sendWhisper("Server", gm, "You are at %4.2f, %4.2f", gm->getPositionCurrent().x, gm->getPositionCurrent().y);
		ChatService::sendWhisper("Server", gm, "and heading towards %4.2f, %4.2f (%im per second)", gm->getPositionDest().x, gm->getPositionDest().y, gm->getMovementSpeed() / 100);
	}
	else if (WANTED_COMMAND("tele")) {
		word_t mapId = atoi(curValue.c_str()); SPLIT();
		if (mapId == 0 || mapId >= mainServer->mapData.size())
			return;
		float coordX = atoi(curValue.c_str()) * 100.0f; SPLIT();
		float coordY = atoi(curValue.c_str()) * 100.0f;

		gm->pakTelegate(mapId, position_t(coordX, coordY));
	} else if(WANTED_COMMAND("stats")) {
		ChatService::sendWhisper("Server", gm, "HP: %i/%i\n", gm->getCurrentHP(), gm->getMaxHP());
		ChatService::sendWhisper("Server", gm, "AtkPower: %i\n", gm->getAttackPower());
		ChatService::sendWhisper("Server", gm, "Defense: %i\n", gm->getDefensePhysical());
		ChatService::sendWhisper("Server", gm, "Hitrate: %i\n", gm->getHitrate());
		ChatService::sendWhisper("Server", gm, "AttackSpeed: %i\n", gm->getAttackSpeed());
		ChatService::sendWhisper("Server", gm, "AttackRange: %f\n", gm->getAttackRange());
	}
	else if (WANTED_COMMAND("level")) {
		byte_t newLevel = static_cast<BYTE>(atoi(curValue.c_str()) & 0xFF);
		if (newLevel == 0)
			newLevel = 1;
		gm->setLevel(newLevel);
	}
	else if(WANTED_COMMAND("heal")) {
		gm->setCurrentHP(gm->getMaxHP());
		gm->setCurrentMP(gm->getMaxMP());
		gm->pakUpdateLifeStats();
		ChatService::sendWhisper("Server", gm, "You were successfully healed: %i HP/ %i MP", gm->getCurrentHP(), gm->getCurrentMP());
	} else if(WANTED_COMMAND("equip")) {
		word_t itemType = atoi(curValue.c_str()); 
		if(itemType == 0 && curValue.length() > 0) {
			curValue = msg;
			for(unsigned int k=ItemType::FACE;k<=ItemType::SHIELD;k++) {
				STBFile* file = mainServer->getEquipmentSTB(k);
				for(unsigned int m=0;m<file->getRowCount();m++) {
					STBEntry& entry = file->getRow(m);
					std::string name = entry.getColumn(0x00);
					if(name.find(curValue.c_str()) != -1) {
						Item item(k, m);

						//In case the wanted item is valid (should always apply)
						if (mainServer->isValidItem(k, m)) {
							gm->addItemToInventory(item, PlayerInventory::fromItemType(item.type));
						}
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
			word_t itemNum = atoi(curValue.c_str()); SPLIT();
			if(itemNum == 0)
				return;
			word_t amount = curValue.length() == 0 ? 0x01 : atoi(curValue.c_str());

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
#undef WANTED_COMMAND
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

AIP* WorldServer::getAIData(const dword_t monId) {
	NPCData* data = this->getNPCData(monId);
	if (data) {
		return &this->aiData[data->getAIId()];
	}
	return nullptr;
}

void WorldServer::dumpAICombined(const char* filePath) {
	CMyFileWriter<char> file(filePath, true);
	for(unsigned int i=0;i<this->aiData.size();i++) {
		AIP* aip = &this->aiData.getValue(i);
		if(aip->getCheckInterval() == 0x00)
			continue;
		file.putStringWithVar("%s [%i]\n", aip->getFilePath().c_str(), i);
		file.putStringWithVar("CheckInterval: %i | DmgTrigger: %i\n", aip->getCheckInterval(), aip->getTriggerDamageAmount());
		file.putStringWithVar("BlockCount: %i\n", aip->getBlockCount());
		for(unsigned int j=0;j<aip->getBlockCount();j++) {
			const std::vector<AIP::Record> records = aip->getRecords(j);
			file.putStringWithVar("\tCurrent Block[%i] records: %i\n", j, records.size());
			for(unsigned int k=0;k<records.size();k++) {
				const AIP::Record& curRec = records.at(k);
				file.putStringWithVar("\t\tConditionCount: %i\n", curRec.getConditionCount());
				for(unsigned int m=0;m<curRec.getConditionCount();m++) {
					const char* data = curRec.getCondition(m).getData();
					AIConditions cond(data);
					file.putStringWithVar("\t\t\t%s\n", cond.toString());
				}
				file.putStringWithVar("\t\tActionCount: %i\n", curRec.getActionCount());
				for(unsigned int m=0;m<curRec.getActionCount();m++) {
					const char* data = curRec.getAction(m).getData();
					AIActions act(data);
					file.putStringWithVar("\t\t\t%s\n", act.toString());
				}
				file.putString("\n\n");
			}
		}
		file.putString("\n\n");
	}
}

Skill* WorldServer::getSkill(const word_t skillId) {
	if (skillId > 0x00 && skillId < this->skillFile->getRowCount())
		return &this->skillFile->getRow(skillId);
	return nullptr;
}

Skill* WorldServer::getSkill(const word_t skillIdBasic, const byte_t level) {
	word_t totalId = (skillIdBasic + level);
	if (skillIdBasic > 0x00 && totalId < this->skillFile->getRowCount()) {
		Skill* skill = &this->skillFile->getRow(skillIdBasic);
		if (skill->getLevel() == 0x00)
			return skill;
		return this->getSkill(skillIdBasic + level - 1);
	}
	return nullptr;
}

void WorldServer::dumpQuest(const char* totalFilePath, bool asHashFalse_Or_asQuestNameTrue) {
	MakeSureDirectoryPathExists(totalFilePath);
	char fileBuf[0x180] = { 0x00 };
	char questHash[0x10] = { 0x00 };

	auto it = this->questData.begin();
	while (it != this->questData.end()) {
		QuestEntry* quest = (*it).second;
		if (!quest)
			continue;
		if (!asHashFalse_Or_asQuestNameTrue)
			sprintf(fileBuf, "%s0x%x.log", totalFilePath, quest->getQuestHash());
		else
			sprintf(fileBuf, "%s%s.log", totalFilePath, quest->getQuestName().c_str());

		CMyFileWriter<char> file(fileBuf, true);
		file.clear();

		file.putStringWithVar("Basic Infos - QuestName: %s, QuestHash: %i\n", quest->getQuestName().c_str(), quest->getQuestHash());
		file.putString("=========================================\n");
		const FixedArray<Trackable<char>>& conditions = quest->getConditions();
		std::string curString = "";
		for (unsigned int j = 0; j < conditions.size(); j++) {
			const Trackable<char>& data = conditions[j];
			curString = QuestService::conditionToString(data.getData());

			file.putString(curString);
		}
		file.putString("=========================================\n");
		const FixedArray<Trackable<char>>& actions = quest->getActions();
		for (unsigned int j = 0; j < actions.size(); j++) {
			const Trackable<char>& data = actions[j];
			curString = QuestService::actionToString(data.getData());

			file.putString(curString);
		}
		file.putString("=========================================");
		if (quest->getNextQuest()) {
			file.putStringWithVar("[CHECK NEXT QUEST_TRIGGER %s [0x%x]]\n", quest->getNextQuest()->getQuestName().c_str(), quest->getNextQuest()->getQuestHash());
		}
		file.close();
		it++;
	}
}


void WorldServer::dumpAISeparated(std::string basicFilePath) {
	MakeSureDirectoryPathExists(basicFilePath.c_str());
	for(unsigned int i=0;i<this->npcData.size();i++) {
		NPCData* npcData = &this->npcData[i];
		AIP* aip = &this->aiData.getValue(npcData->getAIId());
		if (aip == nullptr)
			continue;
		CMyFileWriter<char> file( (basicFilePath + npcData->getName() + ".log").c_str(), "a+");
		file.clear();
		if(aip->getCheckInterval() == 0x00)
			continue;
		file.putStringWithVar("%s [%i]:\n", aip->getFilePath().c_str(), i);
		file.putStringWithVar("CheckInterval: %i | DmgTrigger: %i\n", aip->getCheckInterval(), aip->getTriggerDamageAmount());
		file.putStringWithVar("BlockCount: %i\n", aip->getBlockCount());
		for(unsigned int j=0;j<aip->getBlockCount();j++) {
			const std::vector<AIP::Record> records = aip->getRecords(j);
			file.putStringWithVar("\tCurrent Block[%i] records: %i\n", j, records.size());
			for(unsigned int k=0;k<records.size();k++) {
				const AIP::Record& curRec = records.at(k);
				file.putStringWithVar("\t\tConditionCount: %i\n", curRec.getConditionCount());
				for(unsigned int m=0;m<curRec.getConditionCount();m++) {
					const char* data = curRec.getCondition(m).getData();
					AIConditions cond(data);
					file.putStringWithVar("\t\t\t%s\n", cond.toString().c_str());
				}
				file.putStringWithVar("\t\tActionCount: %i\n", curRec.getActionCount());
				for(unsigned int m=0;m<curRec.getActionCount();m++) {
					const char* data = curRec.getAction(m).getData();
					AIActions act(data);
					file.putStringWithVar("\t\t\t%s\n", act.toString().c_str());
				}
				file.putString("\n\n");
			}
		}
		file.putString("\n\n");
	}
}

void WorldServer::dumpTelegates(std::string filePath) {
	CMyFileWriter<char> file(filePath.c_str(), true);
	file.clear();
	for (unsigned int i = 0; i<this->teleGates.size(); i++) {
		Telegate& gate = this->teleGates.getValue(i);
		if(gate.getSourceMap() == std::numeric_limits<WORD>::max() ||
			gate.getDestMap() == std::numeric_limits<WORD>::max())
			continue;
		file.putStringWithVar("[Gate %i]: From[%i;%f,%f] - To[%i;%f,%f]\n", i, gate.getSourceMap(), gate.getSourcePosition().x, gate.getSourcePosition().y, gate.getDestMap(), gate.getDestPosition().x, gate.getDestPosition().y);
	}
}

	