#include "ZON.h"
#include "D:\Programmieren\QuickInfos\Trackable.hpp"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "..\WorldServer.h"

bool ZON::loadInfos() {
	if(this->filePath.length() < 4) //".zon"
		return false;
	CMyFile file(this->filePath.c_str(), "rb");
	if(!file.exists())
		return false;
	DWORD infoCount = file.read<DWORD>();

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
	this->MapCenter.position = Position(newCenter.x, newCenter.y);
	return true;
}

bool ZON::loadZoneInfos(CMyFile& file) {
	file.skip(0x04);

	//unimportant?
	DWORD mapWidth = file.read<DWORD>();
	DWORD mapHeight = file.read<DWORD>();

	DWORD patchGridCount = file.read<DWORD>();
	short gridSize = static_cast<short>(file.read<float>());
	float patchSize = static_cast<float>(patchGridCount * gridSize);

	/*float totalSize = 1024 * patchSize;
	DWORD zoneSize = mainServer->getZoneSTB()->getZoneSize(this->mapId) * 2;
	DWORD iX = static_cast<DWORD>(totalSize / zoneSize) + 1; 
	DWORD iY = iX;

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

bool ZON::loadEventInfos(CMyFile& file) {
	DWORD entryCount = file.read<DWORD>();
	for (unsigned int i = 0; i < entryCount; i++) {
		EventInfo info;
		info.x = file.read<float>() + 520000.0f;
		info.y = file.read<float>() + 520000.0f;
		info.z = file.read<float>();
		char buf[0x100] = { 0x00 };
		file.readString(buf);

		info.name = std::string(buf);

		this->eventInfos.push_back(info);
	}
	return true;
}

bool ZON::loadEconomyInfos(CMyFile& file) {
	char buf[0x100] = {0x00};
	file.readString(buf);

	DWORD isDungeon = file.read<DWORD>();
	
	//MUSIC?
	file.readString(buf);

	//MODEL?
	file.readString(buf);
	DWORD townCounter = file.read<DWORD>();
	DWORD popCounter = file.read<DWORD>();
	DWORD devCounter = file.read<DWORD>();
	for(BYTE i=ZON::ECONOMY_ITEMTYPE_MIN;i<ZON::ECONOMY_ITEMTYPE_MAX;i++) {
		DWORD consumRate = file.read<DWORD>();
		consumRate = consumRate;
	}
	return true;
}

void ZON::setZoneInfo(WORD minX, WORD minY, WORD maxX, WORD maxY) {
	this->MapInfo.totalMapSize = mainServer->getZoneSTB()->getZoneSize(this->mapId);
	
	this->MapInfo.sectorBeginX = minX;
	this->MapInfo.sectorBeginY = minY;
	this->MapInfo.sectorCountX = (maxX - minX) + 1;
	this->MapInfo.sectorCountY = (maxY - minY) + 1;

	float fieldSize = this->MapInfo.totalMapSize / static_cast<float>(this->MapInfo.sectorCountX * this->MapInfo.sectorCountY);

	this->MapInfo.offsetPerX = fieldSize / static_cast<float>(this->MapInfo.sectorCountX);
	this->MapInfo.offsetPerY = fieldSize / static_cast<float>(this->MapInfo.sectorCountY);
}

Position ZON::calculateCenter(const WORD sectorId) {
	WORD sectorX = sectorId % this->MapInfo.sectorCountX;
	WORD sectorY = sectorId / this->MapInfo.sectorCountX;

	float resultX = ((this->MapCenter.centerIFOX - this->MapInfo.sectorBeginX) - sectorX) * this->MapInfo.offsetPerX;
	resultX = this->MapCenter.position.x - resultX;

	float resultY = ((this->MapCenter.centerIFOY - this->MapInfo.sectorBeginY) - sectorY) * this->MapInfo.offsetPerY;
	resultY = this->MapCenter.position.y - resultY;

	return Position(resultX, resultY);
}