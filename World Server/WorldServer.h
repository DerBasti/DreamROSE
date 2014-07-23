#pragma once

#ifndef __ROSE_WORLD_SERVER__
#define __ROSE_WORLD_SERVER__

#include <cmath>

#include "..\Common\ServerSocket.h"
#include "Entity\Player.h"
#include "FileTypes\IFO.h"
#include "FileTypes\STB.h"
#include "FileTypes\AIP.h"
#include "Map.h"

extern class WorldServer* mainServer;

class WorldServer : public ServerSocket {
	private:
		NPCSTB *npcFile;
		AISTB *aiFile;
		STBFile *skillFile;
		STBFile *statusFile;
		STBFile *questFile;
		STBFile *equipmentFile[15]; //0 = NOT VALID
		STBFile *craftingFile;
		STBFile *sellFile;
		ZoneSTB *zoneFile;
		STBFile *dropFile;
		bool loadNPCData();
		bool loadTelegates(const WORD currentMapId, STBFile& warpFile, IFO& ifoFile);
		bool loadZones();
		bool loadIFOs(Map* mapData);
		bool loadAI();

		std::pair<WORD, Entity*> clientIDs[0x10000];
		std::vector<NPCData> npcData;
		FixedArray<class ZON*> zoneData;
		FixedArray<Telegate> teleGates;
		FixedArray<AIP> aiData;
		FixedArray<Map*> mapData;
		
		struct worldTime {
			time_t lastCheck;
			time_t currentTime;

			const static DWORD DEFAULT_WORLDTIME_CHECK = 10; /*seconds*/
		} WorldTime;
		
		
		int worldVar[0xFF];
		int economyVar[0xFF];

		void executeRequests();
		void checkWorldTime();
		void runMap(Map* curMap);
	public:
		class ChatService {
			private:
				ChatService() {}
				~ChatService() {}
			public:
				static bool sendMessage(Entity* entity, const char* msg);
				static bool sendWhisper(Player* from, Player* to, const char *msg);

				//for server internal stuff.
				static bool sendWhisper(const char* from, Player* to, const char *msg, ...);
				static bool sendShout(Entity* entity, const char* msg);
				static bool sendAnnouncement(Entity* entity, const char* msg);
		};
		class GMService {
			private:
				GMService() {}
				~GMService() {}
			public:
				static void executeCommand(Player* gm, Packet& chatCommand);
		};
		WorldServer(WORD port, MYSQL* mysql);
		~WorldServer();
		__inline ClientSocket* createClient(SOCKET sock) {
			return new Player(sock, this);
		}
		bool loadSTBs();
		bool loadMapData();
		WORD assignClientID(Entity*);
		void freeClientId(Entity*);

		bool isValidItem(const WORD itemType, const WORD itemId);

		Map* getMap(const WORD mapId) const {
			 if (mapId >= this->mapData.capacity())
				return nullptr;
			 return this->mapData.getValue(mapId);
		}
		void changeToMap(Entity* entity, const WORD newMapId);

		void convertTo(NPC* npc, WORD npcDataId);
		
		bool checkSpawns(Map* mapData);
		__inline int getWorldVariable(BYTE varIdx) { return this->worldVar[varIdx]; }
		__inline int getEconomyVariable(BYTE varIdx) { return this->economyVar[varIdx]; }
		__inline NPCData* getNPCData(const DWORD& id) { return &this->npcData.at(id); }
		__inline bool isValidNPCType(const DWORD id) { return this->npcData.at(id).getLevel() > 0; }

		__inline time_t getWorldTime() const { return this->WorldTime.currentTime; }
		DWORD getMapTime(const WORD mapId) {
			return this->mapData[mapId]->getLocalTime(this->getWorldTime());
		}

		__inline Telegate& getGate(const size_t gateId) {
			try {
				return this->teleGates.getValue(gateId);
			}
			catch (...) {
			}
			throw TraceableExceptionARGS("Wanted gateId(%i) > MAX_GATES(%i)", gateId, this->teleGates.size());
		}

		DWORD buildItemVisually(const Item& item);
		WORD buildItemHead(const Item& item);
		DWORD buildItemData(const Item& item);

		STBEntry& getEquipmentEntry(const BYTE itemType, const DWORD itemId);
		const WORD getQuality(const BYTE itemType, const DWORD itemId);
		const WORD getSubType(const BYTE itemType, const DWORD itemId);
		const WORD getWeaponAttackpower(const DWORD itemId);
		const int getWeaponAttackspeed(const DWORD itemId);

		__inline STBFile* getEquipmentSTB(const BYTE itemType) const { return (itemType <= ItemType::PAT ? this->equipmentFile[itemType] : nullptr); }
		__inline STBEntry& getDropTable(const WORD rowId) const { return this->dropFile->getRow(rowId); }

		__inline class ZON* getZON(const BYTE mapId) const { return this->zoneData[mapId]; }
		__inline ZoneSTB* getZoneSTB() const { return this->zoneFile; }
		__inline Entity* getEntity(const WORD clientId) const { return this->clientIDs[clientId].second; }

		__inline void dumpSectors(const WORD mapId, const char* filePath) {
			return this->mapData[mapId]->dumpSectors(filePath);
		}
		void dumpTelegates(const char *filePath);
};

typedef WorldServer::ChatService ChatService;
typedef WorldServer::GMService GMService;

#endif //__ROSE_WORLD_SERVER__