#include "IFO.h"
#include "..\WorldServer.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

#ifdef __ROSE_USE_VFS__

IFO::IFO(const VFSData* file) {
	if (file) {
		this->filePath = file->filePath;
		CMyBufferedFileReader<char> reader(file->data, file->data.size());
		if (!this->loadInfos(reader)) {
			GlobalLogger::warning("Couldn't load IFO %s\n", this->filePath.c_str());
		}
	}
}

template<class _FileType> bool IFO::loadInfos(_FileType& file) {
	if (!file.exists())
		return false;
	dword_t blockCount = file.read<DWORD>();
	dword_t blockType = 0x00;
	dword_t blockOffset = 0x00;
	dword_t oldBlockOffset = 0x00;
	char buf[0x100] = { 0x00 };
	for (dword_t i = 0; i < blockCount; i++) {
		blockType = file.read<dword_t>();
		blockOffset = file.read<dword_t>();

		oldBlockOffset = file.getPosition();
		file.setPosition(blockOffset);
		if (blockType != 0x00 && blockType != IFOType::NPC_ENTRY && blockType != IFOType::SPAWN_ENTRY && blockType != IFOType::TELEGATE_ENTRY) {
			file.setPosition(oldBlockOffset);
			continue;
		}
		dword_t entryCount = file.read<dword_t>();
		for (dword_t j = 0; j < entryCount; j++) {
			_basicIFOEntry basic;
			file.readLengthThenString<byte_t>(buf); 
			basic.strData = std::string(buf);
			basic.unknown1 = file.read<word_t>();
			basic.eventId = file.read<word_t>();
			basic.objType = file.read<dword_t>();
			basic.objId = file.read<dword_t>();
			basic.mapPosX = file.read<dword_t>();
			basic.mapPosY = file.read<dword_t>();

			basic.quatW = file.read<float>();
			basic.quatX = file.read<float>();
			basic.quatY = file.read<float>();
			basic.quatZ = file.read<float>();

			basic.posX = file.read<float>();
			basic.posY = file.read<float>();
			basic.posZ = file.read<float>();

			basic.scaleX = file.read<float>();
			basic.scaleY = file.read<float>();
			basic.scaleZ = file.read<float>();
			
			float qw2 = pow(basic.quatW, 2);
			float qx2 = pow(basic.quatX, 2);
			float qy2 = pow(basic.quatY, 2);
			float qz2 = pow(basic.quatZ, 2);
			float rad = atan2(basic.quatZ * basic.quatY,qz2-qy2);
#ifndef PI
#define PI 3.1415f
#endif
			//Bogenmaﬂ auf Winkel
			float dir = rad * 180.0f / PI;
			if(dir < 0.0f)
				dir += 360.0f;

			IFONPC npc;
			IFOSpawn spawn;
			IFOTelegate telegate;
			byte_t bTmp = 0x00; word_t wTmp = 0x00; dword_t dTmp = 0x00;
			switch (blockType) {
				case IFOType::NPC_ENTRY:
					npc.setBasicInfos(basic);
					dTmp = file.read<dword_t>();
					file.readLengthThenString(buf);

					npc.setOtherInfo(dTmp, dir, buf);

					this->npcs.push_back(npc);
				break;
				case IFOType::SPAWN_ENTRY:
					spawn.setBasicInfos(basic);
					file.readLengthThenString(buf); 
					spawn.spawnName = std::string(buf);
					spawn.basicMobAmount = file.read<dword_t>(); 
					spawn.basicSpawn.reserve(spawn.basicMobAmount);

					for (unsigned int k = 0; k<spawn.basicMobAmount; k++) {
						IFOSpawnEntry newEntry;
						file.readLengthThenString(buf);
						newEntry.mobName = std::string(buf);

						newEntry.mobId = file.read<dword_t>();
						newEntry.amount = file.read<dword_t>();

						if(newEntry.amount>0 && mainServer->isValidNPCType(newEntry.mobId)) {
							spawn.basicSpawn.push_back(newEntry);
						} else {
							spawn.basicMobAmount--;
							k--;
						}
					}

					spawn.tacticalMobAmount = file.read<dword_t>(); 
					spawn.tacticalSpawn.reserve(spawn.tacticalMobAmount);

					for (unsigned int k = 0; k<spawn.tacticalMobAmount; k++) {
						IFOSpawnEntry newEntry;
						file.readLengthThenString(buf);
						newEntry.mobName = std::string(buf);

						newEntry.mobId = file.read<dword_t>();
						newEntry.amount = file.read<dword_t>();

						if(newEntry.amount>0 && mainServer->isValidNPCType(newEntry.mobId)) {
							spawn.tacticalSpawn.push_back(newEntry);
						} else {
							spawn.tacticalMobAmount--;
							k--;
						}
					}
					spawn.respawnInterval = file.read<dword_t>();
					spawn.limit = file.read<dword_t>();

					dTmp = file.read<dword_t>();
					spawn.allowedSpawnDistance = static_cast<float>(dTmp) * 100.0f;
					spawn.tacPoints = file.read<dword_t>();

					dTmp = 0x00; //Temporary spawn amount counter
					for(unsigned int a=0;a<spawn.basicMobAmount;a++) {
						dTmp += spawn.basicSpawn.at(a).amount;
					}
					for(unsigned int a=0;a<spawn.tacticalMobAmount;a++) {
						dTmp += spawn.tacticalSpawn.at(a).amount;
					}
					if(dTmp > 0x00)
						this->spawns.push_back(spawn);
				break;
				case IFOType::TELEGATE_ENTRY:
					telegate.setBasicInfos(basic);
					this->telegates.push_back(telegate);
				break;
				case IFOType::EVENT_ENTRY:
					file.readLengthThenString(buf);
					file.readLengthThenString(buf);
					if (basic.eventId) {
						//TODO:
					}
				break;
			}
		}
		file.setPosition(oldBlockOffset);	
	}
	return true;
}
#else
IFO::IFO(const char* filePath) {
	this->filePath = std::string(filePath);
	this->loadInfos();
}

bool IFO::loadInfos() {
	FILE *fh = fopen(this->filePath, "rb");
	if(!fh)
		return false;

	dword_t blockCount = 0x00; __READ__(&blockCount, 4);

	dword_t blockType = 0x00;
	dword_t blockOffset = 0x00;
	dword_t oldBlockOffset = 0x00;

	char buf[0x100] = { 0x00 };
	for (unsigned int i = 0; i < blockCount; i++) {
		__READ__(&blockType, 4);
		__READ__(&blockOffset, 4);

		oldBlockOffset = ftell(fh);

		//108 -> 194 bei 16 Eintr‰gen
		fseek(fh, blockOffset, SEEK_SET);
		if (blockType != 0x00 && blockType != IFOType::NPC_ENTRY && blockType != IFOType::SPAWN_ENTRY && blockType != IFOType::TELEGATE_ENTRY) {
			fseek(fh, oldBlockOffset, SEEK_SET);
			continue;
		}
		dword_t entryCount = 0x00; __READ__(&entryCount, 4);
		byte_t newLen = 0x00;
		for (unsigned int j = 0; j < entryCount; j++) {
			_basicIFOEntry basic;
			__STRING_READ__(newLen, buf); basic.strData = std::string(buf);
			__READ__(&basic.unknown1, 2);
			__READ__(&basic.eventId, 2);
			__READ__(&basic.objType, 4);
			__READ__(&basic.objId, 4);
			__READ__(&basic.mapPosX, 4);
			__READ__(&basic.mapPosY, 4);

			__READ__(&basic.quatW, 4);
			__READ__(&basic.quatX, 4);
			__READ__(&basic.quatY, 4);
			__READ__(&basic.quatZ, 4);
			
			__READ__(&basic.posX, 4);
			__READ__(&basic.posY, 4);
			__READ__(&basic.posZ, 4);

			__READ__(&basic.scaleX, 4);
			__READ__(&basic.scaleY, 4);
			__READ__(&basic.scaleZ, 4);

			
			float qw2 = pow(basic.quatW,2);
			float qx2 = pow(basic.quatX,2);
			float qy2 = pow(basic.quatY,2);
			float qz2 = pow(basic.quatZ,2);
			float rad = atan2(basic.quatZ * basic.quatY,qz2-qy2);
#ifndef PI
#define PI 3.1415f
#endif
			//Bogenmaﬂ auf Winkel
			float dir = rad * 180.0f / PI;
			if(dir < 0.0f)
				dir += 360.0f;

			IFONPC npc;
			IFOSpawn spawn;
			IFOTelegate telegate;
			byte_t bTmp = 0x00; word_t wTmp = 0x00; dword_t dTmp = 0x00;
			switch (blockType) {
				case IFOType::NPC_ENTRY:
					npc.setBasicInfos(basic);
					__READ__(&dTmp, 4); //UNKNOWN
					__STRING_READ__(bTmp, buf);

					npc.setOtherInfo(dTmp, dir, buf);

					this->npcs.push_back(npc);
				break;
				case IFOType::SPAWN_ENTRY:
					spawn.setBasicInfos(basic);
					__STRING_READ__(bTmp, buf); spawn.spawnName = std::string(buf);
					__READ__(&spawn.basicMobAmount, 4); spawn.basicSpawn.reserve(spawn.basicMobAmount);

					for (unsigned int k = 0; k<spawn.basicMobAmount; k++) {
						IFOSpawnEntry newEntry;
						__STRING_READ__(bTmp, buf);
						newEntry.mobName = std::string(buf);

						__READ__(&newEntry.mobId, 4);
						__READ__(&newEntry.amount, 4);
						
						if(newEntry.amount>0 && mainServer->isValidNPCType(newEntry.mobId)) {
							spawn.basicSpawn.push_back(newEntry);
						}
						else {
							spawn.basicMobAmount--;
							k--;
						}
					}

					__READ__(&spawn.tacticalMobAmount, 4); spawn.tacticalSpawn.reserve(spawn.tacticalMobAmount);
					for (unsigned int k = 0; k<spawn.tacticalMobAmount; k++) {
						IFOSpawnEntry newEntry;
						__STRING_READ__(bTmp, buf);
						newEntry.mobName = std::string(buf);

						__READ__(&newEntry.mobId, 4);
						__READ__(&newEntry.amount, 4);
						
						if(newEntry.amount>0 && mainServer->isValidNPCType(newEntry.mobId)) {
							spawn.tacticalSpawn.push_back(newEntry);
						} else {
							spawn.tacticalMobAmount--;
							k--;
						}
					}
					__READ__(&spawn.respawnInterval, 4);
					__READ__(&spawn.limit, 4);

					__READ__(&dTmp, 4);
					spawn.allowedSpawnDistance = static_cast<float>(dTmp) * 100.0f;
					__READ__(&spawn.tacPoints, 4);

					dTmp = 0x00; //Temporary spawn amount counter
					for(unsigned int a=0;wTmp==0 && a<spawn.basicMobAmount;a++) {
						dTmp += spawn.basicSpawn.at(a).amount;
					}
					for(unsigned int a=0;wTmp==0 && a<spawn.tacticalMobAmount;a++) {
						dTmp += spawn.tacticalSpawn.at(a).amount;
					}
					if(dTmp > 0x00)
						this->spawns.push_back(spawn);
					dTmp = 0x00;
				break;
				case IFOType::TELEGATE_ENTRY:
					telegate.setBasicInfos(basic);

					this->telegates.push_back(telegate);
				break;
				case IFOType::EVENT_ENTRY:
					__STRING_READ__(bTmp, buf);
					__STRING_READ__(bTmp, buf);
					if (basic.eventId) {
						//TODO:
					}
				break;
			}
		}
		fseek(fh, oldBlockOffset, SEEK_SET);	
	}
	fclose(fh);
	return true;
}
#endif

IFO::~IFO() {
	this->npcs.clear();
	this->spawns.clear();
	this->telegates.clear();
}