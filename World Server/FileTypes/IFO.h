#pragma once

#ifndef __IFO__
#define __IFO__

#pragma warning(disable:4996)

#include <string>
#include <vector>

#include "..\Structures.h"
#include "..\..\Common\Definitions.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t;

class IFOType {
	private:
		IFOType() {}
		~IFOType() {}
	public:
		const static dword_t NPC_ENTRY = 0x02;
		const static dword_t SPAWN_ENTRY = 0x08;
		const static dword_t TELEGATE_ENTRY = 0x0A;
		const static dword_t EVENT_ENTRY = 0x0C;
};

class _basicIFOEntry {
	protected:
		friend class IFO;
		std::string strData;
		word_t unknown1;
		word_t eventId;
		dword_t objType;
		dword_t objId;
		dword_t mapPosX;
		dword_t mapPosY;
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

		position_t pos;
		
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
			this->pos = position_t(posX, posY);
		}
		_basicIFOEntry() {}
	public:
		_basicIFOEntry(char* strData,
			word_t unknown1,
			word_t eventId,
			dword_t objType,
			dword_t objId,
			dword_t mapPosX,
			dword_t mapPosY,
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
		__inline position_t getPosition() const { return this->pos; }
		__inline virtual std::string getStringData() const { return this->strData; }
		__inline virtual const dword_t getUnknown() const { return this->unknown1; }
		__inline virtual const dword_t getEventId() const { return this->eventId; }
		__inline virtual const dword_t getObjectId() const { return this->objId; }
		__inline virtual const dword_t getObjectType() const { return this->objType; }
};

class IFONPC : public _basicIFOEntry {
	private:
		friend class IFO;
		dword_t unknown;
		float dir;
		std::string conFile;
	public:
		IFONPC() {

		}
		IFONPC(_basicIFOEntry& newEntry) {
			this->setBasicInfos(newEntry);
		}
		void setOtherInfo(dword_t unknown, float direction, const char* conFile) {
			this->unknown = unknown;
			this->dir = direction;
			this->conFile = std::string(conFile);
		}
		__inline dword_t getUnknownSecond() const { return this->unknown; }
		__inline float getDirection() const { return this->dir; }
		__inline std::string getCONFile() const { return this->conFile; }
};

class IFOSpawnEntry {
	private:
		friend class IFOSpawn;
		friend class IFO;
		std::string mobName;
		dword_t mobId;
		dword_t amount;
	public:
		IFOSpawnEntry() {}

		__inline const dword_t getMobId() const { return this->mobId; };
		__inline const dword_t getAmount() const { return this->amount; }
};

class IFOSpawn : public _basicIFOEntry {
	private:
		friend class IFO;
		std::string spawnName;
		dword_t basicMobAmount;
		std::vector<IFOSpawnEntry> basicSpawn;

		dword_t tacticalMobAmount;
		std::vector<IFOSpawnEntry> tacticalSpawn;

		dword_t respawnInterval; 
		dword_t currentlySpawned;
		dword_t limit;
		float allowedSpawnDistance;
		dword_t tacPoints;
		dword_t currentSpawnId;
		time_t lastCheckTime;
public:
		IFOSpawn() {
			this->currentlySpawned = 0x00;
			this->currentSpawnId = 0x00;
			this->lastCheckTime = time(nullptr);
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
			this->lastCheckTime = time(nullptr);
		}
		IFOSpawn(_basicIFOEntry& newEntry) : IFOSpawn() {
			this->setBasicInfos(newEntry);
		}
		~IFOSpawn() {
			basicSpawn.clear();
			tacticalSpawn.clear();
		}
		__inline dword_t getBasicMobSpawnCount() const { return this->basicMobAmount; }
		__inline IFOSpawnEntry& getBasicMobSpawn(const dword_t spawnId) { return this->basicSpawn.at(spawnId); }
		__inline dword_t getTacticalMobSpawnCount() const { return this->tacticalMobAmount; }
		__inline IFOSpawnEntry& getTacticalMobSpawn(const dword_t spawnId) { return this->tacticalSpawn.at(spawnId); }
		__inline dword_t getRespawnInterval() const { return this->respawnInterval; }
		__inline dword_t getCurrentlySpawned() const { return this->currentlySpawned; }
		__inline void setCurrentlySpawned(const dword_t newVal) { 
			this->currentlySpawned = newVal; 
		}
		__inline dword_t getCurrentSpawnId() const { return this->currentSpawnId; }
		void nextSpawnId() { 
			this->currentSpawnId++; 
			if(this->currentSpawnId >= (this->getBasicMobSpawnCount() + this->getTacticalMobSpawnCount()))
				this->currentSpawnId = 0;
		}
		__inline dword_t getMaxSimultanouslySpawned() const { return this->limit; }
		__inline float getAllowedSpawnDistance() const { return this->allowedSpawnDistance; }
		__inline time_t getLastCheckTime() const { return this->lastCheckTime; }
		__inline void updateLastCheckTime() { this->lastCheckTime = time(nullptr); }
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
		std::string filePath;
		std::vector<IFONPC> npcs;
		std::vector<IFOTelegate> telegates;
		std::vector<IFOSpawn> spawns;
#ifdef __ROSE_USE_VFS__
		template<class _FileType> bool loadInfos(_FileType& file);
#else
		bool loadInfos();
#endif
	public:
		const static dword_t DEFAULT_SECTOR_SIZE = 16000;
		const static dword_t CUSTOMIZED_SECTOR_SIZE = 4000;
#ifdef __ROSE_USE_VFS__
		IFO(const VFSData* file);
#else
		IFO(const char* filePath);
#endif
		~IFO();
		__inline const char* getFilePath() const { return this->filePath.c_str(); }

		__inline IFONPC& getNPC(const dword_t& spawnId) {
			return this->npcs.at(spawnId);
		}
		__inline const dword_t getNPCAmount() const { return this->npcs.size(); }

		__inline IFOSpawn& getSpawn(const dword_t& spawnId) {
			return this->spawns.at(spawnId);
		}
		__inline const dword_t getSpawnAmount() const { return this->spawns.size(); }

		__inline IFOTelegate& getTelegate(const dword_t& spawnId) {
			return this->telegates.at(spawnId);
		}
		__inline const dword_t getTelegateAmount() const { return this->telegates.size(); }
};

#endif //__IFO__