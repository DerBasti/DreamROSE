#pragma once

#include "Structures.h"

#define __MAPSECTOR_LL__
#define __MAPSECTOR_DEBUG__

class MapSector {
	private:
		friend class Map;
#ifdef __MAPSECTOR_DEBUG__
		WORD mapId;
#endif //__MAPSECTOR_DEBUG__
		struct sectorSize {
			Position center;
			DWORD widthAndHeight;
		} SectorSize;
		DWORD id;

#ifdef __MAPSECTOR_LL__
		LinkedList<Entity*> entitiesInSector;
		LinkedList<Entity*>::Node* getNext(LinkedList<Entity*>::Node* curNode, BYTE entityType);
#endif

	public:
		const static BYTE UPPER_LEFT = 0x00;
		const static BYTE UPPER_CENTER = 0x01;
		const static BYTE UPPER_RIGHT = 0x02;

		const static BYTE CENTER_LEFT = 0x03;
		const static BYTE CENTER_RIGHT = 0x04;
		
		const static BYTE LOWER_LEFT = 0x05;
		const static BYTE LOWER_CENTER = 0x06;
		const static BYTE LOWER_RIGHT = 0x07;
		const static BYTE SURROUNDING_MAX = 0x08;
		
		const static WORD DEFAULT_CHECK_TIME = 2000; //MS
		MapSector();
		MapSector(const MapSector& rhs);
		~MapSector();

		MapSector& operator=(const MapSector& rhs);
#ifdef __MAPSECTOR_LL__
		LinkedList<Entity*>::Node* getFirstEntity();
		LinkedList<Entity*>::Node* getNextEntity(LinkedList<Entity*>::Node* curNode);
		LinkedList<Entity*>::Node* getFirstPlayer();
		LinkedList<Entity*>::Node* getNextPlayer(LinkedList<Entity*>::Node* curNode);
		LinkedList<Entity*>::Node* getFirstNPC();
		LinkedList<Entity*>::Node* getNextNPC(LinkedList<Entity*>::Node* curNode);

		__inline void addEntity(Entity* newEntity) {
			this->entitiesInSector.add(newEntity);
		}
		__inline void removeEntity(Entity* toRemove) {
			this->entitiesInSector.remove(toRemove);
		}
#endif
		__inline DWORD getId() const { return this->id; }
		__inline Position getCenter() const { return Position(this->SectorSize.center); }
		__inline void setCenter(const Position& pos) { this->SectorSize.center = pos; }

		__inline DWORD getZoneWidthAndHeight() const { return this->SectorSize.widthAndHeight; }
		__inline void setZoneWidthAndHeight(const DWORD newWH) { this->SectorSize.widthAndHeight = newWH; }
};

class Map {
	private:
		WORD id;
		bool isPVPOn;
		struct _dayCycle {
			bool nightOnly;
			DWORD length;
			DWORD morning; //Column 14
			DWORD day;
			DWORD evening;
			DWORD night; //Column 17
		} dayCycle;
		std::string mapPath;
		FixedArray2D<MapSector*> mapSectors;
		std::vector<class IFOSpawn*> monsterSpawns;
		LinkedList<Entity*> entitiesOnMap;

		void findMinMax(std::vector<std::string>& files, WORD* x, WORD* y);
	public:
		const static DWORD MIN_DAYCYCLE = 0x64;
		const static BYTE MAX_MAPS = 0xFF;

		const static BYTE MORNING = 0x00;
		const static BYTE NOON = 0x01;
		const static BYTE EVENING = 0x02;
		const static BYTE NIGHT = 0x03;

		Map();
		Map(const Map& rhs);
		const Map& operator=(const Map& rhs);
		
		void createSectors(std::vector<std::string>& ifoFiles);
		void dumpSectors(const char* filePath);
		MapSector* getSurroundingSector(MapSector* center, BYTE surroundingSectorType);
	
		__inline const WORD getId() const { return this->id; }
		__inline void setId(const WORD newId) { this->id = newId; }

		__inline const char* getMapPath() const { return this->mapPath.c_str(); }
		__inline void setMapPath(const char* newPath) { this->mapPath = newPath; }

		void setDayCycle(const DWORD totalLen, const DWORD morningBegin, const DWORD noonBegin, const DWORD eveningBegin, const DWORD nightBegin);
		BYTE getDayTime(const DWORD worldTime) const;
		__inline DWORD getLocalTime(const time_t worldTime) const { 
			return worldTime % this->dayCycle.length; 
		}
		__inline void setNightOnly(bool isTrueOrNot) { this->dayCycle.nightOnly = isTrueOrNot; }
		
		__inline void addSpawn(IFOSpawn* spawn) { this->monsterSpawns.push_back(spawn);	}
		__inline const DWORD getSpawnCount() const { return this->monsterSpawns.size(); }
		__inline IFOSpawn* getSpawn(const size_t id) { return this->monsterSpawns.at(id); }
		
		MapSector* getSectorBySpawn(const class IFOSpawn* spawn);
		MapSector* getSector(const WORD sectorId);
		MapSector* getSector(const Position& pos);
		__inline const DWORD getSectorCount() const { 
			return this->mapSectors.capacity(); 
		}
	
		__inline LinkedList<Entity*>::Node* getFirstEntity() {
			return this->entitiesOnMap.getHeadNode();
		}
		__inline LinkedList<Entity*>::Node* getNextEntity(LinkedList<Entity*>::Node* curNode) {
			return (curNode == nullptr ? nullptr : curNode->getNextNode());
		}
		__inline const DWORD getEntityCount() const { 
			return this->entitiesOnMap.getNodeCount();
		}
};

