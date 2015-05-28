#include "ZON.h"
#include "D:\Programmieren\QuickInfos\Trackable.hpp"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "..\WorldServer.h"

#ifndef __ROSE_USE_VFS__
ZON::ZON(const char* filePath, const word_t mapId) {
	this->filePath = filePath;
	this->mapId = mapId;
	this->loadInfos();
}
#else
ZON::ZON(const VFSData* file, const word_t mapId) {
	this->mapId = mapId;
	if (file)  {
		this->filePath = file->filePath;
		this->loadInfos(file);
	}
}
#endif

#ifndef __ROSE_USE_VFS__
bool ZON::loadInfos() {
	if(this->filePath.length() < 4) //".zon"
		return false;
	CMyFileReader<char> file(this->filePath.c_str(), true);
#else
bool ZON::loadInfos(const VFSData* zon) {
	if (!zon || zon->data.size() == 0) //".zon"
		return false;
	CMyBufferedFileReader<char> file(zon->data, zon->data.size());
#endif
	if(!file.exists())
		return false;
	dword_t infoCount = file.read<DWORD>();

	struct _blocks {
		std::vector<DWORD> type;
		std::vector<DWORD> offset;
	} blocks;
	for(unsigned i=0;i<infoCount;i++) {
		blocks.type.push_back(file.read<DWORD>());
		blocks.offset.push_back(file.read<DWORD>());
	}
	for(unsigned int i=0;i<infoCount;i++) {
		file.setPosition(blocks.offset.at(i));
		switch(blocks.type.at(i)) {
			case ZON::ZONE_INFO:
				this->loadZoneInfos(file);
			break;
			case ZON::EVENT_INFO:
				this->loadEventInfos(file);
			break;
			case ZON::ECONOMY_INFO:
				this->loadEconomyInfos(file);
			break;
		}
	}
	EventInfo newCenter = this->findEventInfo(std::string("start"));
	this->MapCenter.position = position_t(newCenter.x, newCenter.y);
	return true;
}

template<class _FileReader> bool ZON::loadZoneInfos(_FileReader& file) {
	file.skip(0x04);

	//unimportant?
	dword_t mapWidth = file.read<DWORD>();
	dword_t mapHeight = file.read<DWORD>();

	dword_t patchGridCount = file.read<DWORD>();
	short gridSize = static_cast<short>(file.read<float>());
	float patchSize = static_cast<float>(patchGridCount * gridSize);

	/*float totalSize = 1024 * patchSize;
	dword_t zoneSize = mainServer->getZoneSTB()->getZoneSize(this->mapId) * 2;
	dword_t iX = static_cast<DWORD>(totalSize / zoneSize) + 1; 
	dword_t iY = iX;

	CMyFile zoneLog("D:\\Games\\ZoneLog.log", "a+");
	for(unsigned int i=0;i<iY;i++) {
		for(unsigned int j=0;j<iX;j++) {
			zoneLog.putStringWithVarOnly("[Map: %i | %i;%i]: Center Pos [%i;%i]\n", this->mapId, i, j, i * zoneSize + (zoneSize/2), j * zoneSize + (zoneSize/2));
		}
	}
	*/
	this->MapCenter.centerIFOX = static_cast<WORD>(file.read<DWORD>());
	this->MapCenter.centerIFOY = static_cast<WORD>(file.read<DWORD>());
	return true;
}

template<class _FileReader> bool ZON::loadEventInfos(_FileReader& file) {
	dword_t entryCount = file.read<DWORD>();
	for (unsigned int i = 0; i < entryCount; i++) {
		EventInfo info;
		info.x = file.read<float>() + 520000.0f;
		info.z = file.read<float>() + 520000.0f;
		info.y = file.read<float>() + 520000.0f;
		info.id = static_cast<BYTE>(i);
		char buf[0x100] = { 0x00 };
		file.readLengthThenString(buf);

		info.name = std::string(buf);

		this->eventInfos.push_back(info);
	}
	return true;
}

template<class _FileReader> bool ZON::loadEconomyInfos(_FileReader& file) {
	char buf[0x100] = {0x00};
	file.readLengthThenString(buf);

	dword_t isDungeon = file.read<DWORD>();
	
	//MUSIC?
	file.readLengthThenString(buf);

	//MODEL?
	file.readLengthThenString(buf);
	dword_t townCounter = file.read<DWORD>();
	dword_t popCounter = file.read<DWORD>();
	dword_t devCounter = file.read<DWORD>();
	for(byte_t i=ZON::ECONOMY_ITEMTYPE_MIN;i<ZON::ECONOMY_ITEMTYPE_MAX;i++) {
		dword_t consumRate = file.read<DWORD>();
		consumRate = consumRate;
	}
	return true;
}

void ZON::setZoneInfo(word_t minX, word_t minY, word_t maxX, word_t maxY) {
	this->MapInfo.sectorBeginX = minX;
	this->MapInfo.sectorBeginY = minY;
	this->MapInfo.sectorCountX = (maxX - minX) + 1;
	this->MapInfo.sectorCountY = (maxY - minY) + 1;

	float fieldSize = this->MapInfo.totalMapSize / static_cast<float>(this->MapInfo.sectorCountX * this->MapInfo.sectorCountY);

	this->MapInfo.offsetPerX = fieldSize / static_cast<float>(this->MapInfo.sectorCountX);
	this->MapInfo.offsetPerY = fieldSize / static_cast<float>(this->MapInfo.sectorCountY);
}

position_t ZON::calculateCenter(const word_t sectorId) {
	word_t sectorX = sectorId % this->MapInfo.sectorCountX;
	word_t sectorY = sectorId / this->MapInfo.sectorCountX;

	float resultX = ((this->MapCenter.centerIFOX - this->MapInfo.sectorBeginX) - sectorX) * this->MapInfo.offsetPerX;
	resultX = this->MapCenter.position.x - resultX;

	float resultY = ((this->MapCenter.centerIFOY - this->MapInfo.sectorBeginY) - sectorY) * this->MapInfo.offsetPerY;
	resultY = this->MapCenter.position.y - resultY;

	return position_t(resultX, resultY);
}