#include "WorldServer.h"
#include "Entity\Monster.h"
#include "FileTypes\ZON.h"
#include "FileTypes\ZMO.h"
#include "FileTypes\VFS.h"
#include "FileTypes\QSD.h"

#pragma comment(lib, "DbgHelp.lib")

int main() {
	MYSQL mySQL;
	WorldServer server(29200, &mySQL);
	server.loadEncryption();
	
	if(config->getValueBool("DumpQuests")) {
		std::cout << "Dumping Quests...\r";
		server.dumpQuest((::workingPath + "\\QUEST_DUMP\\").c_str(), config->getValueBool("UseQuestNames") );
		std::cout << "Dumping Quests finished!\n";
	}
	if (config->getValueBool("DumpAI")) {
		std::cout << "Dumping AI...\r";
		server.dumpAISeparated((::workingPath + "\\AI\\"));
		std::cout << "Dumping AI finished!\n";
	}
	if (config->getValueBool("DumpTelegates")) {
		std::cout << "Dumping Telegates...\r";
		server.dumpTelegates((::workingPath + "\\Telegates.log").c_str());
		std::cout << "Dumping Telegates finished!\n";
	}
	server.start();
	return 0x00;
}