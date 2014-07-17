#pragma once

#ifndef __IFO__
#define __IFO__

#pragma warning(disable:4996)

#include <string>
#include <vector>

#include "..\Structures.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

class IFOType {
	private:
		IFOType() {}
		~IFOType() {}
	public:
		const static DWORD NPC_ENTRY = 0x02;
		const static DWORD SPAWN_ENTRY = 0x08;
		const static DWORD TELEGATE_ENTRY = 0x0A;
		const static DWORD EVENT_ENTRY = 0x0C;
};

class _basicIFOEntry {
	protected:
		friend class IFO;
		std::string strData;
		WORD unknown1;
		WORD eventId;
		DWORD objType;
		DWORD objId;
		DWORD mapPosX;
		DWORD mapPosY;
		float quatW;
		float quatX;
		float quatY;
		float quatZ;
		float posX;
		float posY;
		float posZ;
		float scaleX;
		float scaleY;
		float scaleZ;

		Position pos;
		
		void setBasicInfos(_basicIFOEntry newEntry) {
			this->strData = newEntry.strData;
			this->unknown1 = newEntry.unknown1;
			this->eventId = newEntry.eventId;
			this->objType = newEntry.objType;
			this->objId = newEntry.objId;
			this->mapPosX = newEntry.mapPosX;
			this->mapPosY = newEntry.mapPosY;
			this->quatW = newEntry.quatW;
			this->quatX = newEntry.quatX;
			this->quatY = newEntry.quatY;
			this->quatZ = newEntry.quatZ;
			this->posX = newEntry.posX;
			this->posY = newEntry.posY;
			this->posZ = newEntry.posZ;
			this->scaleX = newEntry.scaleX;
			this->scaleY = newEntry.scaleY;
			this->scaleZ = newEntry.scaleZ;
		
			float posX = this->posX + 520000.f;
			float posY = this->posY + 520000.f;
			this->pos = Position(posX, posY);
		}
		_basicIFOEntry() {}
	public:
		_basicIFOEntry(char* strData,
			WORD unknown1,
			WORD eventId,
			DWORD objType,
			DWORD objId,
			DWORD mapPosX,
			DWORD mapPosY,
			float quatW,
			float quatX,
			float quatY,
			float quatZ,
			float posX,
			float posY,
			float posZ,
			float scaleX,
			float scaleY,
			float scaleZ) {
				this->strData = std::string(strData);
				this->unknown1 = unknown1;
				this->eventId = eventId;
				this->objType = objType;
				this->objId = objId;
				this->mapPosX = mapPosX;
				this->mapPosY = mapPosY;
				this->quatW = quatW;
				this->quatX = quatX;
				this->quatY = quatY;
				this->quatZ = quatZ;
				this->posX = posX;
				this->posY = posY;
				this->posZ = posZ;
				this->scaleX = scaleX;
				this->scaleY = scaleY;
				this->scaleZ = scaleZ;
		}
		__inline Position getPosition() const { return this->pos; }
		__inline virtual std::string getStringData() const { return this->strData; }
		__inline virtual const DWORD getUnknown() const { return this->unknown1; }
		__inline virtual const DWORD getEventId() const { return this->eventId; }
		__inline virtual const DWORD getObjectId() const { return this->objId; }
		__inline virtual const DWORD getObjectType() const { return this->objType; }
};

class IFONPC : public _basicIFOEntry {
	private:
		friend class IFO;
		DWORD unknown;
		float dir;
		std::string conFile;
	public:
		IFONPC() {

		}
		IFONPC(_basicIFOEntry& newEntry) {
			this->setBasicInfos(newEntry);
		}
		void setOtherInfo(DWORD unknown, float direction, const char* conFile) {
			this->unknown = unknown;
			this->dir = direction;
			this->conFile = std::string(conFile);
		}
		__inline DWORD getUnknownSecond() const { return this->unknown; }
		__inline float getDirection() const { return this->dir; }
		__inline std::string getCONFile() const { return this->conFile; }
};

class IFOSpawnEntry {
	private:
		friend class IFOSpawn;
		friend class IFO;
		std::string mobName;
		DWORD mobId;
		DWORD amount;
	public:
		IFOSpawnEntry() {}

		__inline const DWORD getMobId() const { return this->mobId; };
		__inline const DWORD getAmount() const { return this->amount; }
};

class IFOSpawn : public _basicIFOEntry {
	private:
		friend class IFO;
		std::string spawnName;
		DWORD basicMobAmount;
		std::vector<IFOSpawnEntry> basicSpawn;

		DWORD tacticalMobAmount;
		std::vector<IFOSpawnEntry> tacticalSpawn;

		DWORD respawnInterval; 
		DWORD currentlySpawned;
		DWORD limit;
		DWORD allowedSpawnDistance;
		DWORD tacPoints;
		DWORD currentSpawnId;
public:
		IFOSpawn() {
			this->currentlySpawned = 0x00;
			this->currentSpawnId = 0x00;
		}
		IFOSpawn(const IFOSpawn& spawn) {
			this->setBasicInfos(spawn);
			this->allowedSpawnDistance = spawn.allowedSpawnDistance;
			this->basicMobAmount = spawn.basicMobAmount;
			for(unsigned int i=0;i<this->basicMobAmount;i++) {
				IFOSpawnEntry newEntry;
				newEntry.amount = spawn.basicSpawn[i].amount;
				newEntry.mobId = spawn.basicSpawn[i].mobId;
				newEntry.mobName = spawn.basicSpawn[i].mobName;
				this->basicSpawn.push_back( newEntry );
			}
			this->currentlySpawned = 0x00;
			this->currentSpawnId = 0x00;
			this->limit = spawn.limit;
			this->respawnInterval = spawn.respawnInterval;
			this->spawnName = spawn.spawnName;
			this->tacPoints = spawn.tacPoints;
			this->tacticalMobAmount = spawn.tacticalMobAmount;
			for(unsigned int i=0;i<this->tacticalMobAmount;i++) {
				IFOSpawnEntry entry;
				entry.amount = spawn.tacticalSpawn[i].amount;
				entry.mobId = spawn.tacticalSpawn[i].mobId;
				entry.mobName = spawn.tacticalSpawn[i].mobName;
				this->tacticalSpawn.push_back(entry);
			}
		}
		IFOSpawn(_basicIFOEntry& newEntry) {
			this->setBasicInfos(newEntry);
		}
		~IFOSpawn() {
			basicSpawn.clear();
			tacticalSpawn.clear();
		}
		__inline DWORD getBasicMobSpawnCount() const { return this->basicMobAmount; }
		__inline IFOSpawnEntry& getBasicMobSpawn(const DWORD spawnId) { return this->basicSpawn.at(spawnId); }
		__inline DWORD getTacticalMobSpawnCount() const { return this->tacticalMobAmount; }
		__inline IFOSpawnEntry& getTacticalMobSpawn(const DWORD spawnId) { return this->tacticalSpawn.at(spawnId); }
		__inline DWORD getRespawnInterval() const { return this->respawnInterval; }
		__inline DWORD getCurrentlySpawned() const { return this->currentlySpawned; }
		__inline void setCurrentlySpawned(const DWORD newVal) { 
			this->currentlySpawned = newVal; 
		}
		__inline DWORD getCurrentSpawnId() const { return this->currentSpawnId; }
		void nextSpawnId() { 
			this->currentSpawnId++; 
			if(this->currentSpawnId >= (this->getBasicMobSpawnCount() + this->getTacticalMobSpawnCount()))
				this->currentSpawnId = 0;
		}
		//01523 435 86 42 christian hüttenberger
		//Brockenbergstrasse 5, Holzgerlingen 71088
		__inline DWORD getMaxSimultanouslySpawned() const { return this->limit; }
		__inline DWORD getAllowedSpawnDistance() const { return this->allowedSpawnDistance; }
		/*
		bool operator==(const IFOSpawn& ifo) {
			return (this->id == ifo.id);
		}
		bool operator!=(const IFOSpawn& ifo) {
			return !(this->operator==(ifo));
		}
		bool operator<(const IFOSpawn& ifo) {
			return (this->id < ifo.id);
		}
		bool operator<=(const IFOSpawn& ifo) {
			return (this->id <= ifo.id);
		}
		bool operator>=(const IFOSpawn& ifo) {
			return !(this->operator<(ifo));
		}
		bool operator>(const IFOSpawn& ifo) {
			return !(this->operator<=(ifo));
		}
		*/
};

class IFOTelegate : public _basicIFOEntry {
	public:
		IFOTelegate() {

		}
		IFOTelegate(_basicIFOEntry& newEntry) {
			this->setBasicInfos(newEntry);
		}
};

#define __STRING_READ__(len, buffer) fread(&len, 1, 1, fh); if(len>0) fread(buffer, 1, len, fh);
#define __READ__(buffer, size) fread(buffer, size, 1, fh);

class IFO {
	private:
		const char* filePath;
		std::vector<IFONPC> npcs;
		std::vector<IFOTelegate> telegates;
		std::vector<IFOSpawn> spawns;
		bool loadInfos();
	public:
		const static DWORD DEFAULT_SECTOR_SIZE = 16000;
		const static DWORD CUSTOMIZED_SECTOR_SIZE = 4000;
		IFO(const char* filePath) {
			this->filePath = filePath;
			this->loadInfos();
		}
		__inline const char* getFilePath() const { return this->filePath; }

		__inline IFONPC& getNPC(const DWORD& spawnId) {
			return this->npcs.at(spawnId);
		}
		__inline const DWORD getNPCAmount() const { return this->npcs.size(); }

		__inline IFOSpawn& getSpawn(const DWORD& spawnId) {
			return this->spawns.at(spawnId);
		}
		__inline const DWORD getSpawnAmount() const { return this->spawns.size(); }

		__inline IFOTelegate& getTelegate(const DWORD& spawnId) {
			return this->telegates.at(spawnId);
		}
		__inline const DWORD getTelegateAmount() const { return this->telegates.size(); }
};

#endif //__IFO__