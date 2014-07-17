#include "IFO.h"
#include "..\WorldServer.h"

bool IFO::loadInfos() {
	FILE *fh = fopen(this->filePath, "rb");

	DWORD blockCount = 0x00; __READ__(&blockCount, 4);

	DWORD blockType = 0x00;
	DWORD blockOffset = 0x00;
	DWORD oldBlockOffset = 0x00;

	char buf[0x100] = { 0x00 };
	for (unsigned int i = 0; i < blockCount; i++) {
		__READ__(&blockType, 4);
		__READ__(&blockOffset, 4);

		oldBlockOffset = ftell(fh);

		//108 -> 194 bei 16 Einträgen
		fseek(fh, blockOffset, SEEK_SET);
		if (blockType != 0x00 && blockType != IFOType::NPC_ENTRY && blockType != IFOType::SPAWN_ENTRY && blockType != IFOType::TELEGATE_ENTRY) {
			fseek(fh, oldBlockOffset, SEEK_SET);
			continue;
		}
		DWORD entryCount = 0x00; __READ__(&entryCount, 4);
		BYTE newLen = 0x00;
		for (unsigned int i = 0; i < entryCount; i++) {
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
#define PI 3.1415
#endif
			//Bogenmaß auf Winkel
			float dir = rad * 180.0f / PI;
			if(dir < 0.0f)
				dir += 360.0f;

			IFONPC npc;
			IFOSpawn spawn;
			IFOTelegate telegate;
			BYTE bTmp = 0x00; WORD wTmp = 0x00; DWORD dTmp = 0x00;
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

					for (unsigned int i = 0; i<spawn.basicMobAmount; i++) {
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
							i--;
						}
					}

					__READ__(&spawn.tacticalMobAmount, 4); spawn.tacticalSpawn.reserve(spawn.tacticalMobAmount);
					for (unsigned int i = 0; i<spawn.tacticalMobAmount; i++) {
						IFOSpawnEntry newEntry;
						__STRING_READ__(bTmp, buf);
						newEntry.mobName = std::string(buf);

						__READ__(&newEntry.mobId, 4);
						__READ__(&newEntry.amount, 4);
						
						if(newEntry.amount>0 && mainServer->isValidNPCType(newEntry.mobId)) {
							spawn.tacticalSpawn.push_back(newEntry);
						} else {
							spawn.tacticalMobAmount--;
							i--;
						}
					}
					__READ__(&spawn.respawnInterval, 4);
					__READ__(&spawn.limit, 4);
					__READ__(&spawn.allowedSpawnDistance, 4);
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