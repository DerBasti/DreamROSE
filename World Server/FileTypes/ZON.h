#pragma once

#ifndef __ROSE_ZON__
#define __ROSE_ZON__

#include <string>
#include <vector>

#include "..\Structures.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

class ZON {
	public:
		struct EventInfo {
			BYTE id;
			float x;
			float y;
			float z;
			std::string name;
		};

		//Combined id of the IFO which is the center
		struct mapCenterInfo {
			WORD centerIFOX;
			WORD centerIFOY;

			Position position;
		} MapCenter;

		struct mapInfo {
			float totalMapSize;
			WORD sectorCountX;
			WORD sectorCountY;
			WORD sectorBeginX;
			WORD sectorBeginY;

			float offsetPerX;
			float offsetPerY;
		} MapInfo;
	private:
		std::string filePath;
		WORD mapId;
		bool loadInfos();

		bool loadZoneInfos(class CMyFile& file);
		bool loadEventInfos(class CMyFile& file);
		bool loadEconomyInfos(class CMyFile& file);

		std::vector<EventInfo> eventInfos;
	public:
		const static BYTE ZONE_INFO = 0x00;
		const static BYTE EVENT_INFO = 0x01;
		//0x02 = ZoneTile, 0x03 = TileType = unimportant
		const static BYTE ECONOMY_INFO = 0x04;

		const static BYTE ECONOMY_ITEMTYPE_MIN = 0x01; //Masks
		const static BYTE ECONOMY_ITEMTYPE_MAX = 0x0B; //Jewelry
		ZON(const char* filePath, const WORD mapId) {
			this->filePath = filePath;
			this->mapId = mapId;
			this->loadInfos();
		}
		~ZON() {
			this->eventInfos.clear();
		}
		__inline const size_t getEventInfoAmount() const { return this->eventInfos.size(); }
		__inline EventInfo& getEventInfo(const size_t pos) { return this->eventInfos.at(pos); }
		__inline EventInfo findEventInfo(const std::string& name) {
			for (unsigned int i = 0; i < this->eventInfos.size(); i++) {
				if (_stricmp(this->eventInfos.at(i).name.c_str(), name.c_str()) == 0)
					return this->eventInfos.at(i);
			}
			throw std::exception();
		}
		
		__inline DWORD getCenterIFOX() const { return this->MapCenter.centerIFOX; }
		__inline DWORD getCenterIFOY() const { return this->MapCenter.centerIFOY; }
		void setZoneInfo(WORD minX, WORD maxX, WORD minY, WORD maxY);
		Position calculateCenter(const WORD sectorId);
};

#endif //