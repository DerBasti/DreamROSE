#pragma once

#ifndef __ROSE_ZON__
#define __ROSE_ZON__

#include <string>
#include <vector>

#include "..\Structures.h"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "..\..\Common\Definitions.h"
#include "VFS.h"

class ZON {
	public:
		struct EventInfo {
			byte_t id;
			float x;
			float y;
			float z;
			std::string name;
		};

		//Combined id of the IFO which is the center
		struct mapCenterInfo {
			word_t centerIFOX;
			word_t centerIFOY;

			position_t position;
		} MapCenter;

		struct mapInfo {
			float totalMapSize;
			word_t sectorCountX;
			word_t sectorCountY;
			word_t sectorBeginX;
			word_t sectorBeginY;

			float offsetPerX;
			float offsetPerY;
		} MapInfo;
	private:
		std::string filePath;
		word_t mapId;
#ifndef __ROSE_USE_VFS__
		bool loadInfos();
#else
		bool loadInfos(const VFSData* file);
#endif

		template<class _FileReader> bool loadZoneInfos(_FileReader& file);
		template<class _FileReader> bool loadEventInfos(_FileReader& file);
		template<class _FileReader> bool loadEconomyInfos(_FileReader& file);

		std::vector<EventInfo> eventInfos;
	public:
		const static byte_t ZONE_INFO = 0x00;
		const static byte_t EVENT_INFO = 0x01;
		//0x02 = ZoneTile, 0x03 = TileType = unimportant
		const static byte_t ECONOMY_INFO = 0x04;

		const static byte_t ECONOMY_ITEMTYPE_MIN = 0x01; //Masks
		const static byte_t ECONOMY_ITEMTYPE_MAX = 0x0B; //Jewelry
#ifndef __ROSE_USE_VFS__
		ZON(const char* filePath, const word_t mapId);
#else
		ZON(const VFSData* file, const word_t mapId);
#endif

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
		
		__inline dword_t getCenterIFOX() const { return this->MapCenter.centerIFOX; }
		__inline dword_t getCenterIFOY() const { return this->MapCenter.centerIFOY; }
		void setZoneInfo(word_t minX, word_t maxX, word_t minY, word_t maxY);
		position_t calculateCenter(const word_t sectorId);
};

#endif //