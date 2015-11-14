#pragma once

#define __MAPSECTOR_LL__
#define __MAPSECTOR_DEBUG__

#include <algorithm>
#include "D:\Programmieren\QuickInfos\LinkedList.h"
#include "Structures.h"
#include "FileTypes\ZON.h"
#include "FileTypes\IFO.h"

class Entity;

class Map {
	public:
		class Sector {
		private:
			friend class Map;
	#ifdef __MAPSECTOR_DEBUG__
			byte_t mapId;
	#endif //__MAPSECTOR_DEBUG__
			position_t center;
			dword_t id;

	#ifdef __MAPSECTOR_LL__
			LinkedList<Entity*> entitiesInSector;
			LinkedList<Entity*>::Node* getNext(LinkedList<Entity*>::Node* curNode, byte_t entityType) const;
	#endif

		public:
			const static byte_t UPPER_LEFT = 0x00;
			const static byte_t UPPER_CENTER = 0x01;
			const static byte_t UPPER_RIGHT = 0x02;

			const static byte_t CENTER_LEFT = 0x03;
			const static byte_t CENTER_RIGHT = 0x04;

			const static byte_t LOWER_LEFT = 0x05;
			const static byte_t LOWER_CENTER = 0x06;
			const static byte_t LOWER_RIGHT = 0x07;
			const static byte_t SURROUNDING_MAX = 0x08;

			const static word_t DEFAULT_CHECK_TIME = 2000; //MS
			Sector();
			Sector(const Sector& rhs);
			~Sector();

			Sector& operator=(const Sector& rhs);
	#ifdef __MAPSECTOR_LL__
			LinkedList<Entity*>::Node* getFirstEntity() const;
			LinkedList<Entity*>::Node* getNextEntity(LinkedList<Entity*>::Node* curNode) const;
			LinkedList<Entity*>::Node* getFirstPlayer() const;
			LinkedList<Entity*>::Node* getNextPlayer(LinkedList<Entity*>::Node* curNode) const;
			LinkedList<Entity*>::Node* getFirstNPC() const;
			LinkedList<Entity*>::Node* getNextNPC(LinkedList<Entity*>::Node* curNode) const;

			bool exists(const Entity* entity) const;
			Entity* find(const word_t id) const;

			__inline void addEntity(Entity* newEntity) {
				this->entitiesInSector.add(newEntity);
			}
			__inline LinkedList<Entity*>::Node* removeEntity(Entity* toRemove) {
				return this->entitiesInSector.remove(toRemove);
			}
	#endif
			__inline dword_t getId() const { return this->id; }
			__inline position_t getCenter() const { return position_t(this->center); }
			__inline void setCenter(const position_t& pos) { this->center = pos; }
		};
	private:
		byte_t id;
		bool isPVPOn;
		struct _dayCycle {
			bool nightOnly;
			dword_t length;
			dword_t morning; //Column 14
			dword_t day;
			dword_t evening;
			dword_t night; //Column 17
		} dayCycle;
		std::string mapPath;
		float sectorSize;
		FixedArray2D<Map::Sector*> mapSectors;
		std::vector<class IFOSpawn*> monsterSpawns;
		LinkedList<Entity*> entitiesOnMap;
		void findMinMax(std::vector<std::string>& files, WORD* x, WORD* y);
		ZON::EventInfo* getRespawn(position_t& pos);

		Entity* clientIDs[0x10000];
	public:
		const static dword_t MIN_DAYCYCLE = 0x64;
		const static byte_t MAX_MAPS = 0xFF;

		const static byte_t MORNING = 0x00;
		const static byte_t NOON = 0x01;
		const static byte_t EVENING = 0x02;
		const static byte_t NIGHT = 0x03;

		Map();
		Map(const Map& rhs);
		const Map& operator=(const Map& rhs);

		word_t assignLocalId(Entity*);
		void freeLocalId(Entity*);
		__inline Entity* getEntity(const word_t localId) const { return this->clientIDs[localId]; }

		bool hasActivePlayers() const;
		
		void createSectors(std::vector<std::string>& ifoFiles);
		void dumpSectors(const char* filePath);
		Map::Sector* getSurroundingSector(Map::Sector* center, byte_t surroundingSectorType);
	
		__inline const byte_t getId() const { return this->id; }
		__inline void setId(const byte_t newId) { this->id = newId; }

		__inline const char* getMapPath() const { return this->mapPath.c_str(); }
		__inline void setMapPath(const char* newPath) { this->mapPath = newPath; }

		void setDayCycle(const dword_t totalLen, const dword_t morningBegin, const dword_t noonBegin, const dword_t eveningBegin, const dword_t nightBegin);
		byte_t getDayTime(const dword_t worldTime) const;
		__inline dword_t getLocalTime(const time_t worldTime) const { 
			return worldTime % this->dayCycle.length; 
		}
		__inline void setNightOnly(bool isTrueOrNot) { this->dayCycle.nightOnly = isTrueOrNot; }
		
		__inline void addSpawn(IFOSpawn* spawn) { this->monsterSpawns.push_back(spawn);	}
		__inline const dword_t getSpawnCount() const { return this->monsterSpawns.size(); }
		__inline IFOSpawn* getSpawn(const size_t id) { return this->monsterSpawns.at(id); }
		
		Map::Sector* getSectorBySpawn(const class IFOSpawn* spawn) const;
		Map::Sector* getSector(const word_t sectorId) const;
		Map::Sector* getSector(const position_t& pos) const;
		__inline const word_t getSectorCount() const { 
			return static_cast<word_t>(this->mapSectors.capacity()); 
		}

		const position_t getRespawnPoint(const char* spawnName);
		const position_t getRespawnPoint(position_t& pos);
		const position_t getRespawnPoint(const size_t pos);
		const word_t getRespawnPointId(position_t& pos);

		bool checkSpawns();
	
		__inline LinkedList<Entity*>::Node* getFirstEntity() const {
			return this->entitiesOnMap.getHeadNode();
		}
		__inline LinkedList<Entity*>::Node* getNextEntity(LinkedList<Entity*>::Node* curNode) const {
			return (curNode == nullptr ? nullptr : curNode->getNextNode());
		}
		__inline const dword_t getEntityCount() const { 
			return this->entitiesOnMap.getNodeCount();
		}
		class NPC* getNPC(const word_t type) const ;
		class Player* getPlayer(const word_t localId) const;
		class Player* getPlayer(std::string& name) const;
		__inline class Player* getPlayer(const char* name) const {
			return this->getPlayer(std::string(name));
		}

		__inline float getSectorWidthAndHeight() const { return this->sectorSize; }
		__inline void setSectorWidthAndHeight(const dword_t newWH) { 
			this->sectorSize = static_cast<float>((newWH < IFO::CUSTOMIZED_SECTOR_SIZE) ? newWH : IFO::CUSTOMIZED_SECTOR_SIZE);
		}

};

