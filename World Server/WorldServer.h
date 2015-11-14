#pragma once

#ifndef __ROSE_WORLD_SERVER__
#define __ROSE_WORLD_SERVER__

#include <cmath>
#include <thread>
#include <mutex>

#include "..\Common\Definitions.h"
#include "..\Common\ServerSocket.h"
#include "Entity\Player.h"
#include "FileTypes\AIP.h"
#include "FileTypes\IFO.h"
#include "FileTypes\QSD.h"
#include "FileTypes\STB.h"
#include "FileTypes\VFS.h"
#include "FileTypes\CHR.h"
#include "Map.h"

extern class WorldServer* mainServer;

class WorldServer : public ServerSocket {
	private:
		InternalServerConnection charServer;

		VFS* vfs;
		CHR* npcAnimationInfos;
		NPCSTB *npcFile;
		AISTB *aiFile;
		SkillSTB *skillFile;
		StatusSTB *statusFile;
#ifdef __ROSE_DEBUG__
		std::vector<QSD*> qsdFiles;
#endif
		STBFile *questFile;
		STBFile *equipmentFile[15]; //0 = NOT VALID
		STBFile *craftingFile;
		STBFile *sellFile;
		ZoneSTB *zoneFile;
		STBFile *motionFile;
		STBFile *dropFile;
		bool loadNPCData();
		bool loadNPCAnimations();
		bool loadTelegates(const word_t currentMapId, STBFile& warpFile, IFO& ifoFile);
		bool loadZones();
		bool loadIFOs(Map* mapData, STBFile& warpFile);
		bool loadAI();
		bool loadQuests();
		bool loadAttackTimings();

		std::vector<NPCData> npcData;
		std::vector<NPC*> globalNPCs;
		std::vector<Player*> globalPlayers;
		std::map<const DWORD, QuestEntry*> questData;
		FixedArray<Skill*> skillData;
		FixedArray<class ZON*> zoneData;
		FixedArray<Telegate> teleGates;
		FixedArray<AIP> aiData;
		FixedArray<Map*> mapData;
		FixedArray<ZMO> playerAttackAnimations;
		
		struct worldTime {
			time_t lastCheck;
			time_t currentTime;

			const static dword_t DEFAULT_WORLDTIME_CHECK = 10; /*seconds*/
		} WorldTime;

#ifdef __ROSE_MULTI_THREADED__
		FixedArray<std::thread*> mapThreads;
#endif
		
		int worldVar[0xFF];
		int economyVar[0xFF];

		void registerServer(); 
		bool checkForServer(ClientSocket* client, std::string& ip);

		void setupBeforeMainLoop();
		void executeRequests();
		void onClientDisconnect(ClientSocket* client);
		void checkWorldTime();
#ifndef __ROSE_MULTI_THREADED__
		void runMap(Map* curMap);
#endif

	public:
		class ChatService {
			private:
				ChatService() {}
				~ChatService() {}
			public:
				static bool sendDebugMessage(Player* entity, const char* msg, ...);
				static bool sendMessage(Entity* entity, const char* msg);
				static bool sendWhisper(Player* from, Player* to, const char *msg);

				//for server internal stuff.
				static bool sendWhisper(const char* from, Player* to, const char *msg, ...);
				static bool sendShout(Entity* entity, const char* msg);
				static bool sendShout(Entity* entity, const char* msg, ...);
				static bool sendAnnouncement(Entity* entity, const char* msg, ...);
		};
		class GMService {
			private:
				GMService() {}
				~GMService() {}
			public:
				static void executeCommand(Player* gm, Packet& chatCommand);
		};
		WorldServer(word_t port, MYSQL* mysql);
		~WorldServer();
		ClientSocket* createClient(SOCKET sock);
		bool notifyCharServer(const word_t packetId, const dword_t optionalId = std::numeric_limits<dword_t>::max());
		bool sendToAll(Packet& pak);
		bool loadSTBs();
		bool loadMapData();

		bool isValidItem(const byte_t itemType, const word_t itemId);

		__inline dword_t getMapAmount() const { return this->mapData.size(); }
		Map* getMap(const word_t mapId) const {
			 if (mapId >= this->mapData.capacity())
				return nullptr;
			 return this->mapData.getValue(mapId);
		}
		void changeToMap(Entity* entity, const word_t newMapId);
		
		__inline dword_t getPlayerAmount() const { return this->globalPlayers.size(); }
		__inline Player* getGlobalPlayer(const dword_t num) { return this->globalPlayers.at(num); }

		__inline int getWorldVariable(byte_t varIdx) { return this->worldVar[varIdx]; }
		__inline int getEconomyVariable(byte_t varIdx) { return this->economyVar[varIdx]; }
		__inline NPCData* getNPCData(const DWORD& id) { return &this->npcData.at(id); }
		NPCData* getNPCData(const AIP* ai);
		NPC* getNPCGlobal(const word_t npcId);
		AIP* getAIData(const dword_t monId);
		__inline bool isValidNPCType(const dword_t id) { return this->npcData.at(id).getLevel() > 0; }

		Skill* getSkill(const word_t skillId);
		Skill* getSkill(const word_t skillIdBasic, const byte_t level);

		__inline const STBEntry* getConsumable(const Item& item) { return this->getConsumable(item.id); }
		__inline const STBEntry* getConsumable(const word_t itemId) { return &this->equipmentFile[ItemType::CONSUMABLES]->getRow(itemId); }
		__inline const STBEntry* getStatus(const byte_t statusType) { return &this->statusFile->getRow(statusType); }

		//TODO
		QuestEntry* getQuest(const dword_t questHash) {
			return this->questData[questHash];
		}
		QuestEntry* getQuestById(const dword_t questId);

		__inline time_t getWorldTime() const { return this->WorldTime.currentTime; }
		dword_t getMapTime(const word_t mapId) {
			return this->mapData[mapId]->getLocalTime(this->getWorldTime());
		}

		__inline Telegate& getGate(const size_t gateId) {
			try {
				return this->teleGates.getValue(gateId);
			}
			catch (...) {
			}
			throw TraceableException("Wanted gateId(%i) > MAX_GATES(%i)", gateId, this->teleGates.size());
		}

		STBEntry& getEquipmentEntry(const byte_t itemType, const dword_t itemId);
		const word_t getQuality(const byte_t itemType, const dword_t itemId);
		const word_t getSubType(const byte_t itemType, const dword_t itemId);
		const word_t getWeaponAttackpower(const dword_t itemId);
		const dword_t getSellPrice(const Item& item) const;
		__inline const dword_t getSellPrice(const byte_t type, const word_t id) const {
			return this->getSellPrice(Item(type, id));
		}
		const int getWeaponAttackspeed(const dword_t itemId);

		const word_t getMotionId(const word_t basicMotionId, const byte_t motionTypeForEquippedWeapon);
		const byte_t getMotionTypeForWeapon(const dword_t itemId);

		__inline ZMO* getAttackMotionPlayer(const word_t motionId) const { return &this->playerAttackAnimations.getValue(motionId); }
		__inline ZMO* getAttackMotionNPC(const word_t npcId) const { return this->npcAnimationInfos->getAttackMotion(npcId); }

		__inline STBFile* getEquipmentSTB(const byte_t itemType) const { return (itemType <= ItemType::PAT ? this->equipmentFile[itemType] : nullptr); }
		__inline STBEntry& getDropTable(const word_t rowId) const { return this->dropFile->getRow(rowId); }

		__inline class ZON* getZON(const byte_t mapId) const { return this->zoneData[mapId]; }
		__inline ZoneSTB* getZoneSTB() const { return this->zoneFile; }
		__inline Entity* getEntity(const word_t map, const word_t localId) const { return this->getMap(map)->getEntity(localId); }

		__inline void dumpSectors(const word_t mapId, const char* filePath) {
			return this->mapData[mapId]->dumpSectors(filePath);
		}

		void dumpQuest(const char* totalFilePath, bool asHashFalse_Or_asQuestNameTrue);

		void dumpAICombined(const char* totalFilePath);
		void dumpAISeparated(std::string filePath);

		void dumpTelegates(std::string filePath);
};

typedef WorldServer::ChatService ChatService;
typedef WorldServer::GMService GMService;

#ifdef __ROSE_MULTI_THREADED__
extern std::mutex sectorMutex;
void runMap(Map* curMap);
#endif

#endif //__ROSE_WORLD_SERVER__