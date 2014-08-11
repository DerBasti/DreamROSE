#include "WorldServer.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"
#include "FileTypes\ZMO.h"
#include "FileTypes\VFS.h"

int main() {
	/*
	::ConfigA conf; conf.init("D:\\Games\\iROSE Online Server\\config.conf");
	VFS vfs(conf.getValueString("GameFolder"));

	Trackable<char> data;

	vfs.readFile("3DDATA\\STB\\TYPE_MOTION.STB", data);
	STBFile_Template<STBEntry_INT> typeMotionSTB(data, data.size());
	STBEntry_INT attackMotion = typeMotionSTB.getRow(0x08);

	std::vector<DWORD> attackMotionEntries;
	for (unsigned int i = 0; i < attackMotion.getColumnCount(); i++) {
		attackMotionEntries.push_back(attackMotion.getColumn<DWORD>(i));
	}
	vfs.readFile("3DDATA\\STB\\FILE_MOTION.STB", data);
	STBFile fileMotionSTB(data, data.size());
	
	std::vector<DWORD> timePerAttack;
	for (unsigned int j = 0; j < attackMotionEntries.size(); j++) {
		vfs.readFile(fileMotionSTB.getRow(attackMotionEntries.at(j)).getColumn(0x00).c_str(), data);
		ZMO zmo(data, data.size());

		timePerAttack.push_back(zmo.getFrameCount() * 1000 / zmo.getFPS());
	}
	*/
	//STBFile npc(".\\3DDATA\\STB\\LIST_NPC.STB");
	
	MYSQL mysql;
	WorldServer server(29200, &mysql);
	server.loadEncryption();
	server.start();
	

	/*
	std::cout << "Input allowed now!\n";
	std::string input = "";
	Player* player = new Player(0, &server);
	while(true) {
		std::getline(std::cin, input);
		const DWORD mapId = atoi(input.substr(0, input.find(" ")).c_str());
		
		input = input.substr(input.find(" ")+1);
		const float x = (float)atoi(input.substr(0, input.find(" ")).c_str()) * 100.0f;

		input = input.substr(input.find(" ")+1);
		const float y = (float)atoi(input.c_str()) * 100.0f;
		
		player->setPositionCurrent( Position(x, y) );
		player->setPositionDest( player->getPositionCurrent() );
		server.changeToMap(player, mapId);

		Map* currentMap = server.getMap(mapId);
		CMyFile file("D:\\Games\\iROSE Online Server\\sectorTest.log", "a+");
		if(currentMap) {
			MapSector* p = player->getSector();
			LinkedList<Entity*>::Node* eNode = p->getFirstEntity();
			while(eNode) {
				Entity* entity = eNode->getValue();
				eNode = eNode->getNextNode();
				if(!entity || entity == player)
					continue;
				
				file.putStringWithVarOnly("Entity[%s [%i], %i]: %f, %f\n", entity->getName().c_str(), dynamic_cast<NPC*>(entity)->getTypeId(), entity->getClientId(), entity->getCurrentX(), entity->getCurrentY());			
			}
			/*
			MapSector* s = currentMap->getSector( Position(x, y) );
			std::cout << "Nearest Sector[" << s->getId() << "]: " << s->getCenter().x << ", " << s->getCenter().y << "\n";
			file.putStringWithVarOnly("Nearest Sector[%i]: %f, %f\n", s->getId(), s->getCenter().x, s->getCenter().y);
			LinkedList<Entity*>::Node* eNode = s->getFirstEntity();
			while(eNode) {
				Entity* entity = eNode->getValue();
				eNode = eNode->getNextNode();
				if(!entity)
					continue;
				
				file.putStringWithVarOnly("Entity[%s [%i], %i]: %f, %f\n", entity->getName().c_str(), dynamic_cast<NPC*>(entity)->getTypeId(), entity->getClientId(), entity->getCurrentX(), entity->getCurrentY());			
			}
			file.putString("\n\nSURROUNDING SECTORS!\n");
			for(unsigned int i=0;i<MapSector::SURROUNDING_MAX;i++) {
				MapSector *e = currentMap->getSurroundingSector(s, i);
				std::cout << "Surrounding Sector[" << i << "=" << e->getId() << "]: " << e->getCenter().x << ", " << e->getCenter().y << "\n";
				file.putStringWithVarOnly("Surrounding Sector[%i]: %f, %f\n", e->getId(), e->getCenter().x, e->getCenter().y);
				eNode = e->getFirstEntity();
				while(eNode) {
					Entity* entity = eNode->getValue();
					eNode = eNode->getNextNode();
					if(!entity)
						continue;
					file.putStringWithVarOnly("Entity[%s [%i], %i]: %f, %f\n", entity->getName().c_str(), dynamic_cast<NPC*>(entity)->getTypeId(), entity->getClientId(), entity->getCurrentX(), entity->getCurrentY());			
				}
				file.putString("\nNEW SURROUNDING SECTORS!\n");
			}
		}
	}
	*/

	/*
	std::string filePath("D:\\Games\\iROSE Online Server\\3DDATA\\MAPS\\JUNON\\JDT01\\");
	std::vector<std::string> files;
	QuickInfo::getFilesFromDirectory(filePath, std::string(".ifo"), files, false);
	for (unsigned int i = 0; i < files.size(); i++) {
		IFO ifo(files.at(i).c_str());

		CMyFile file((files.at(i) + std::string(".txt")).c_str(), "a+");
		file.putString("[NPCs]\n");
		for (unsigned int j = 0; j < ifo.getNPCAmount(); j++) {
			IFONPC& npc = ifo.getNPC(j);
			file.putStringWithVarOnly("TypeId: %i, Position: [%f, %f]\n", npc.getObjectId(), npc.getPosition().x, npc.getPosition().y);
		}
		file.putString("\n[Telegates]\n");
		for (unsigned int j = 0; j < ifo.getTelegateAmount(); j++) {
			IFOTelegate& gate = ifo.getTelegate(j);
			file.putStringWithVarOnly("Position: [%f, %f], WarpName: %s\n", gate.getPosition().x, gate.getPosition().y, gate.getStringData().c_str());
		}
		file.putString("\n[Spawns]\n");
		for (unsigned int j = 0; j < ifo.getSpawnAmount(); j++) {
			IFOSpawn& spawn = ifo.getSpawn(j);
			file.putStringWithVarOnly("SpawnId: %i, Center: [%f, %f], Radius: %i\n", j, spawn.getPosition().x, spawn.getPosition().y, spawn.getAllowedSpawnDistance());
			char buf[0x100] = { 0x00 };
			sprintf(buf, "[BasicMobs: %i]: ", spawn.getBasicMobSpawnCount());
			for (unsigned int k = 0; k < spawn.getBasicMobSpawnCount(); k++) {
				sprintf(&buf[strlen(buf)], "(%i, %i) | ", spawn.getBasicMobSpawn(k).getMobId(), spawn.getBasicMobSpawn(k).getAmount());
			}
			file.putString(buf);
			sprintf(buf, "\n[TacticMobs: %i]: ", spawn.getTacticalMobSpawnCount());
			for (unsigned int k = 0; k < spawn.getTacticalMobSpawnCount(); k++) {
				sprintf(&buf[strlen(buf)], "(%i, %i) | ", spawn.getTacticalMobSpawn(k).getMobId(), spawn.getTacticalMobSpawn(k).getAmount());
			}
			file.putString(buf);
			file.putString("\n\n\n");
		}
	}
	*/
	return 0x00;
}